using System;
using System.Text;
using System.Diagnostics;
using System.Buffers.Binary;
using DirtySDKEAWebKit;
using DirtySDKEAWebKit.CSharp;
using System.Text.RegularExpressions;
using System.Linq;
using System.Collections.Generic;
using System.Collections.Concurrent;
using System.Threading.Tasks;
using System.Threading;

namespace SimpleConnProj;

class FeslClient
{
    private enum MessageType
    {
        MessageHello,
        MessageMemCheck,
        MessageNuLogin,
        MessageNuGetPersonas,
        MessageNuAddPersona,
        MessageNuLoginPersona,
        MessageTelemetryToken,
        MessagePing,
        MessageGameSpyPreAuth,
        MessageUnknown
    }
    private readonly ConcurrentDictionary<MessageType, Dictionary<string, string>> _responses = new();
    private readonly Thread _readerThread;
    private readonly object _sendLock = new();

    private enum ReaderThreadState
    {
        AwaitingForConnection,
        ProcessingResponses,
        Shutdowning
    }
    private volatile ReaderThreadState _readerThreadState = ReaderThreadState.AwaitingForConnection;

    public FeslClient()
    {
        // Create ProtoSSL instance
        _ssl = ProtoSSL.ProtoSSLCreate();

        // Reset the ProtoSSL instance
        ProtoSSL.ProtoSSLReset(_ssl!);

        // Allow any valid certificate (don't check if cert is issued to using host)
        ProtoSSL.ProtoSSLControl(_ssl!, Tools.FourCC("ncrt"), 1);

        _readerThread = new Thread(_receivingThread)
        {
            IsBackground = true
        };
        _readerThread.Start();
    }

    public void DeInitialize()
    {
        _readerThreadState = ReaderThreadState.Shutdowning;
        _timeoutMs = 0; // force the timeout (FIXME: dirty hack! Change later!)
        _readerThread.Join();
    }

    public void Initialize()
    {
        // Prepare connection
        int result = ProtoSSL.ProtoSSLConnect(_ssl!, 1, _serverAddress, 0, _serverPort);

        // Advance the connection/handshake state machine
        ProtoSSL.ProtoSSLUpdate(_ssl!);

        _readerThreadState = ReaderThreadState.ProcessingResponses;

        string hello =
            "TXN=Hello\n" +
            "clientString=cncra3-pc\n" +
            "sku=15299\n" +
            "locale=ru\n" +
            "clientPlatform=PC\n" +
            "clientVersion=1.0\n" +
            "SDKVersion=4.3.4.0.0\n" +
            "protocolVersion=2.0\n" +
            "fragmentSize=8096\n" +
            "clientType=\n";

        _sendFeslPacket("fsys", 0xC0000001, hello);
        
        var fieldsHello = _awaitForTheResponseWithTimeout(MessageType.MessageHello, _timeoutMs);
        if (fieldsHello == null)
            return;
        
        var fieldsMemCheck = _awaitForTheResponseWithTimeout(MessageType.MessageMemCheck, _timeoutMs);
        if (fieldsMemCheck == null)
            return;

        string memCheck =
            "TXN=MemCheck\n" +
            "result=\n";
        _sendFeslPacket("fsys", 0x00000080, memCheck);
    }

    public bool Login(string login, string password)
    {
        Tools.NetPrintf("Logging in...");

        var loginQueryString =
            "TXN=NuLogin\n" +
            "returnEncryptedInfo=1\n" +
            $"nuid={login}\n" +             // << here is our username
            $"password={password}\n" +      // << here is out password
            "macAddr=$cccccccccccc\n";      // for some reason this field is filled with 'c');

        _sendFeslPacket("acct", 0x020000C0, loginQueryString);
        
        var fieldsLogin = _awaitForTheResponseWithTimeout(MessageType.MessageNuLogin, _timeoutMs);
        if (fieldsLogin == null)
            return false;

        if (fieldsLogin.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Login error code: {fieldsLogin["errorCode"]}");
            if (fieldsLogin.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fieldsLogin["localizedMessage"]}");
            return false;
        }

        return true;
    }

    public string[] Personas()
    {
        Tools.NetPrintf("Querying personas list...");

        var personasQueryString =
            "TXN=NuGetPersonas\n" +
            "namespace=\n";                 // for some reason "namespace" field is empty
        _sendFeslPacket("acct", 0x030000C0, personasQueryString);

        var fields = _awaitForTheResponseWithTimeout(MessageType.MessageNuGetPersonas, _timeoutMs);
        if (fields == null)
            return [];
            
        if (fields.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Error code: {fields["errorCode"]}");
            if (fields.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fields["localizedMessage"]}");
            return [];
        }

        if (!fields.ContainsKey("personas.[]"))
        {
            Tools.NetPrintf($"No personas list in the packet!");
            return [];
        }

        if (fields["personas.[]"] == "0")
            return [];

        return [];
    }

    public void AddPersona(string personaName)
    {
        Tools.NetPrintf("Adding persona...");

        var addPersonaQueryString =
            "TXN=NuAddPersona\n" +
            $"name={personaName}\n";
        _sendFeslPacket("acct", 0x040000C0, addPersonaQueryString);

        var fields = _awaitForTheResponseWithTimeout(MessageType.MessageNuAddPersona, _timeoutMs);
        if (fields == null)
            return;

        if (fields.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Error code: {fields["errorCode"]}");
            if (fields.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fields["localizedMessage"]}");
            return;
        }
    }

    public bool LoginPersona(string personaName, out string profileId)
    {
        profileId = "";

        Tools.NetPrintf("Logging in as a persona...");

        var loginPersonaQueryString =
            "TXN=NuLoginPersona\n" +
            $"name={personaName}\n";
        _sendFeslPacket("acct", 0x060000C0, loginPersonaQueryString);

        var fields = _awaitForTheResponseWithTimeout(MessageType.MessageNuLoginPersona, _timeoutMs);
        if (fields == null)
            return false;

        if (fields.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Error code: {fields["errorCode"]}");
            if (fields.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fields["localizedMessage"]}");
            return false;
        }

        if (!fields.ContainsKey("profileId"))
        {
            Tools.NetPrintf("Could not find profileId in the response!");
            return false;
        }

        profileId = fields["profileId"];
        return true;
    }

    public void GetTelemetryToken()
    {
        Tools.NetPrintf("Requesting telemetry token...");

        var telemetryTokenQueryString =
            "TXN=GetTelemetryToken\n";
        _sendFeslPacket("acct", 0x070000C0, telemetryTokenQueryString);

        var fields = _awaitForTheResponseWithTimeout(MessageType.MessageTelemetryToken, _timeoutMs);
        if (fields == null)
            return;
        
        if (fields.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Error code: {fields["errorCode"]}");
            if (fields.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fields["localizedMessage"]}");
            return;
        }
    }

    public bool GameSpyPreAuth(out string ticket, out string challenge)
    {
        ticket = "";
        challenge = "";

        Tools.NetPrintf("Sending GameSpy pre-auth request...");

        var gameSpyPreAuthQueryString = "TXN=GameSpyPreAuth\n";
        _sendFeslPacket("acct", 0x080000C0, gameSpyPreAuthQueryString);

        var fields = _awaitForTheResponseWithTimeout(MessageType.MessageGameSpyPreAuth, _timeoutMs);
        if (fields == null)
            return false;
        
        if (fields.ContainsKey("errorCode"))
        {
            Tools.NetPrintf($"Error code: {fields["errorCode"]}");
            if (fields.ContainsKey("localizedMessage"))
                Tools.NetPrintf($"Message: {fields["localizedMessage"]}");
            return false;
        }

        if (fields.ContainsKey("ticket"))
            Tools.NetPrintf($"PreAuth ticket: {fields["ticket"]}");
        else
            return false;
        ticket = fields["ticket"];

        if (fields.ContainsKey("challenge"))
            Tools.NetPrintf($"PreAuth ticket: {fields["challenge"]}");
        else
            return false;
        challenge = fields["challenge"];

        return true;
    }

#region Internals

    public void _processPingResponse()
    {
        Tools.NetPrintf("Answering ping...");
        var pingQueryString = "TXN=Ping\n";
        _sendFeslPacket("fsys", 0x090000C0, pingQueryString);
    }

    // Returns null on timeout
    private Dictionary<string, string>? _awaitForTheResponseWithTimeout(MessageType msgType, int timeoutMs)
    {
        var stopwatch = Stopwatch.StartNew();
        while (stopwatch.ElapsedMilliseconds < timeoutMs)
        {
            if (_responses.TryGetValue(msgType, out var response))
                return response;

            Thread.Sleep(10);
        }

        Tools.NetPrintf("Awaiting response timeout!");
        return null;
    }

    private Dictionary<string, string> _getKeyValuePairsFromFeslPayload(string payload)
    {
        return Regex.Matches(payload, @"([a-zA-Z][a-zA-Z0-9.\[\]]*)=([^\r\n\0]*)")
            .Cast<Match>()
            .ToDictionary(
                m => m.Groups[1].Value,
                m => m.Groups[2].Value
            );
    }

    private void _sendFeslPacket(string type, uint magic, string message)
    {
        byte[] body = Encoding.UTF8.GetBytes(message + '\0');
        byte[] packet = new byte[12 + body.Length];

        // Four ASCII bytes identifying the packet type
        Encoding.ASCII.GetBytes(type).CopyTo(packet, 0);

        BinaryPrimitives.WriteUInt32BigEndian(packet.AsSpan(4, 4), magic);
        BinaryPrimitives.WriteUInt32BigEndian(packet.AsSpan(8, 4), (uint)packet.Length);
        Buffer.BlockCopy(body, 0, packet, 12, body.Length);

        int sent = -1;
        lock (_sendLock)
        {
            sent = ProtoSSL.ProtoSSLSend(_ssl!, packet, packet.Length);
        }
        if (sent < 0)
        {
            Tools.NetPrintf($"ProtoSSLSend failed with error {sent}.");
            throw new InvalidOperationException($"ProtoSSLSend failed with error {sent}.");
        }
    }

    private int _receiveFeslPacket(out string response)
    {
        var stopwatch = Stopwatch.StartNew();
        byte[] header = new byte[12];
        int received = 0;
        response = "";

        // Read the complete 12-byte FESL header.
        while (received < header.Length)
        {
            if (_ensureNotTimedOut(stopwatch) && received == 0)
                return 0;

            ProtoSSL.ProtoSSLUpdate(_ssl);

            byte[] chunk = new byte[header.Length - received];
            int count = ProtoSSL.ProtoSSLRecv(_ssl, chunk, chunk.Length);

            if (count > 0)
            {
                Buffer.BlockCopy(chunk, 0, header, received, count);
                received += count;
            }
            else if (count < -1)
            {
                return count;
            }
            else
            {
                System.Threading.Thread.Sleep(10);
            }
        }

        uint packetSize = BinaryPrimitives.ReadUInt32BigEndian(header.AsSpan(8, 4));

        //Assert.InRange(packetSize, 12u, 1024u * 1024u);

        int bodySize = checked((int)packetSize - 12);
        byte[] body = new byte[bodySize];
        received = 0;

        while (received < body.Length)
        {
            _ensureNotTimedOut(stopwatch);

            ProtoSSL.ProtoSSLUpdate(_ssl);

            byte[] chunk = new byte[body.Length - received];
            int count = ProtoSSL.ProtoSSLRecv(_ssl, chunk, chunk.Length);

            if (count > 0)
            {
                Buffer.BlockCopy(chunk, 0, body, received, count);
                received += count;
            }
            else if (count < -1)
            {
                return count;
            }
            else
            {
                System.Threading.Thread.Sleep(10);
            }
        }

        int textLength = Array.IndexOf(body, (byte)0);
        if (textLength < 0)
            textLength = body.Length;

        response = Encoding.UTF8.GetString(body, 0, textLength);
        return textLength;
    }

    private bool _ensureNotTimedOut(Stopwatch stopwatch)
    {
        if (stopwatch.ElapsedMilliseconds >= _timeoutMs)
        {
            Tools.NetPrintf($"Timed out after {_timeoutMs} ms waiting for server data.");
            return true;
        }
        return false;
    }

    private void _receivingThread()
    {
        while (_readerThreadState != ReaderThreadState.Shutdowning)
        {
            if (_readerThreadState == ReaderThreadState.AwaitingForConnection)
            {
                Thread.Sleep(10);
                continue;
            }

            if (_readerThreadState != ReaderThreadState.ProcessingResponses)
                throw new NotImplementedException();

            int code = _receiveFeslPacket(out string response);
            if (code > 0)
            {
                // If we are here - we've got a message!
                Tools.NetPrintf($"Server response: {response}");
                var payload = _getKeyValuePairsFromFeslPayload(response);
                var responseType = MessageType.MessageUnknown;
                if (payload.ContainsKey("TXN"))
                {
                    switch (payload["TXN"])
                    {
                    case "Hello":
                        responseType = MessageType.MessageHello;
                        break;

                    case "MemCheck":
                        responseType = MessageType.MessageMemCheck;
                        break;

                    case "NuLogin":
                        responseType = MessageType.MessageNuLogin;
                        break;

                    case "NuGetPersonas":
                        responseType = MessageType.MessageNuGetPersonas;
                        break;

                    case "NuAddPersona":
                        responseType = MessageType.MessageNuAddPersona;
                        break;

                    case "NuLoginPersona":
                        responseType = MessageType.MessageNuLoginPersona;
                        break;

                    case "GetTelemetryToken":
                        responseType = MessageType.MessageTelemetryToken;
                        break;

                    case "GameSpyPreAuth":
                        responseType = MessageType.MessageGameSpyPreAuth;
                        break;

                    case "Ping":
                        responseType = MessageType.MessagePing;
                        _processPingResponse();
                        break;

                    default:
                        throw new NotImplementedException();
                    }
                }

                _responses[responseType] = payload;
            }
        }
    }

#endregion

    private volatile int _timeoutMs = 30_000;
    private const string _serverAddress = RA3Constants.RA3_STRING_SERVER_FESL_ORIGINAL;
    private const int _serverPort = RA3Constants.RA3_STRING_SERVER_FESL_PORT;

    ProtoSSLRefT? _ssl;
}
