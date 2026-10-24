using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Sockets;
using System.Text;
using DirtySDKEAWebKit;

public sealed class PeerchatDataContext
{
    public List<string> PlayerList { get; set; } = new();
    public string TxKey { get; set; } = string.Empty;
    public string RxKey { get; set; } = string.Empty;
    public string OurIp { get; set; } = string.Empty;
}

public sealed class PeerchatConnection : IDisposable
{
    public string SecretKey { get; set; } = string.Empty;

    private Socket? _socket;

    private GsPeerChat? _txCrypto;
    private GsPeerChat? _rxCrypto;

    private bool _encrypted;
    private string _ourIp = string.Empty;
    private bool _disposed;

    private const int BufferSize = 8192;

    private void Log(string message) => Tools.NetPrintf(message);

    private static int CmdToNum(string commandCodeFragment)
    {
        return int.TryParse(commandCodeFragment, out int result)
            ? result
            : -1;
    }

    // ---------------------------------------------------------
    // Initialization
    // ---------------------------------------------------------

    public bool Init(string server, int port)
    {
        try
        {
            IPAddress address;

            if (!IPAddress.TryParse(server, out address!))
            {
                var addresses = Dns.GetHostAddresses(server);

                address = addresses.FirstOrDefault(
                    a => a.AddressFamily == AddressFamily.InterNetwork)
                    ?? throw new SocketException(
                        (int)SocketError.HostNotFound);
            }

            _socket = new Socket(
                AddressFamily.InterNetwork,
                SocketType.Stream,
                ProtocolType.Tcp);

            // Аналог SO_RCVBUF
            _socket.ReceiveBufferSize = BufferSize;

            // Аналог bind(0.0.0.0:0).
            _socket.Bind(new IPEndPoint(IPAddress.Any, 0));

            _socket.Connect(new IPEndPoint(address, port));

            Log("Connected!");
            return true;
        }
        catch (Exception ex)
        {
            Log($"Could not connect: {ex.Message}");
            _socket?.Dispose();
            _socket = null;
            return false;
        }
    }

    // ---------------------------------------------------------
    // Response processing
    // ---------------------------------------------------------

    private PeerchatDataContext ProcessResponse(string response)
    {
        var context = new PeerchatDataContext();

        var commands = response.Split(
            new[] { "\r\n" },
            StringSplitOptions.RemoveEmptyEntries);

        foreach (string command in commands)
        {
            var parts = command.Split(
                new[] { ' ' },
                StringSplitOptions.RemoveEmptyEntries);

            if (parts.Length < 2)
                continue;

            switch (CmdToNum(parts[1]))
            {
                case 302: // Our IP address
                {
                    // Сохранена совместимость с исходным offset.
                    if (command.Length >= 12)
                    {
                        context.OurIp = command.Substring(12).Trim();
                        _ourIp = context.OurIp;

                        Log($"Got our IP: {_ourIp}");
                    }

                    break;
                }

                case 353: // Player names list body
                {
                    int separator = command.IndexOf(" :",
                        StringComparison.Ordinal);

                    if (separator >= 0)
                    {
                        string players = command.Substring(separator + 2);

                        context.PlayerList = players.Split(
                            new[] { ' ' },
                            StringSplitOptions.RemoveEmptyEntries
                        ).ToList();
                    }

                    break;
                }

                case 705: // Crypto keys
                {
                    if (parts.Length != 5 || parts[2] != "*")
                    {
                        Log("Wrong response format!");
                        break;
                    }

                    context.TxKey = parts[3];
                    context.RxKey = parts[4];

                    Log("Successfully received keys!");
                    Log($"TX Key: {context.TxKey}");
                    Log($"RX Key: {context.RxKey}");

                    break;
                }

                case 706: // Auth OK / PING
                {
                    if (command.Contains(
                        "Authenticated",
                        StringComparison.Ordinal))
                    {
                        if (command.Contains(
                            "PING",
                            StringComparison.Ordinal))
                        {
                            Log("Got PING.");
                            SendPong();
                        }

                        Log("Authenticated successfully!");
                    }

                    break;
                }
            }
        }

        return context;
    }

    // ---------------------------------------------------------
    // Encryption initialization
    // ---------------------------------------------------------

    public bool SendCrypt(string gameName, string secretKey)
    {
        if (_socket == null)
            return false;

        try
        {
            SendRaw(
                $"CRYPT des 1 {gameName}\r\n",
                encrypt: false);

            string response = ReceiveText(encrypt: false);
            var context = ProcessResponse(response);

            if (string.IsNullOrWhiteSpace(context.TxKey) ||
                string.IsNullOrWhiteSpace(context.RxKey))
            {
                Log("Server did not return valid encryption keys.");
                return false;
            }

            Log("Initializing encryption context...");

            byte[] txKeyBytes = Encoding.ASCII.GetBytes(context.TxKey);
            byte[] rxKeyBytes = Encoding.ASCII.GetBytes(context.RxKey);
            byte[] secretKeyBytes = Encoding.ASCII.GetBytes(secretKey);

            _txCrypto.Init(txKeyBytes, secretKeyBytes);
            _rxCrypto.Init(rxKeyBytes, secretKeyBytes);

            _encrypted = true;

            Log("Encryption initialized.");
            return true;
        }
        catch (Exception ex)
        {
            Log($"CRYPT failed: {ex.Message}");
            return false;
        }
    }

    // ---------------------------------------------------------
    // USRIP
    // ---------------------------------------------------------

    public void SendUsrIp()
    {
        if (!_encrypted)
            return;

        Log("Getting our IP from server...");

        SendCommand("USRIP\r\n");

        var context = ProcessResponse(ReceiveText());

        if (!string.IsNullOrWhiteSpace(context.OurIp))
            _ourIp = context.OurIp;
    }

    // ---------------------------------------------------------
    // USER / NICK
    // ---------------------------------------------------------

    public bool SendUser(
        string profileId,
        string pseudonym,
        string originalServer,
        string encodedCdKey)
    {
        if (!_encrypted)
            return false;

        if (string.IsNullOrWhiteSpace(_ourIp))
        {
            Log("Our IP is not known. Call SendUsrIp first.");
            return false;
        }

        Log("Sending user data (IP, profile ID and CD-KEY)...");

        if (!IPAddress.TryParse(_ourIp, out var ip))
        {
            Log($"Invalid IP address: {_ourIp}");
            return false;
        }

        string encodedIp = EncodeInetAddressToString(ip);

        string message =
            $"USER X{encodedIp}X|{profileId} 127.0.0.1 " +
            $"{originalServer} :" +
            $"{encodedCdKey}\r\n" +
            $"NICK {pseudonym}\r\n";

        SendCommand(message);

        ProcessResponse(ReceiveText());

        return true;
    }

    // Замените тело этой функции реализацией исходного
    // encode_inet_addr_to_string(addr, 0) из C++.
    private static string EncodeInetAddressToString(IPAddress ip)
    {
        throw new NotImplementedException("Port encode_inet_addr_to_string from the C++ project.");
    }

    // ---------------------------------------------------------
    // CDKEY
    // ---------------------------------------------------------

    public void SendCdKey(string cdKey)
    {
        if (!_encrypted)
            return;

        Log("Sending CD-KEY...");

        SendCommand($"CDKEY {cdKey}\r\n");

        ProcessResponse(ReceiveText());
    }

    // ---------------------------------------------------------
    // PONG
    // ---------------------------------------------------------

    public void SendPong()
    {
        SendCommand("PONG :s\r\n");
        Log("Sent PONG.");
    }

    // ---------------------------------------------------------
    // JOIN
    // ---------------------------------------------------------

    public List<string> SendJoin(string channelName)
    {
        if (!_encrypted)
            return new List<string>();

        Log("Joining...");

        SendCommand($"JOIN {channelName}\r\n");

        var context = ProcessResponse(ReceiveText());

        return context.PlayerList;
    }

    // ---------------------------------------------------------
    // QUIT
    // ---------------------------------------------------------

    public void SendQuit()
    {
        if (!_encrypted)
            return;

        Log("Sending quit...");

        SendCommand("QUIT :Later!\r\n");

        ProcessResponse(ReceiveText());
    }

    // ---------------------------------------------------------
    // Socket helpers
    // ---------------------------------------------------------

    private void SendCommand(string command)
    {
        SendRaw(command, encrypt: _encrypted);
    }

    private void SendRaw(string command, bool encrypt)
    {
        if (_socket == null)
            throw new InvalidOperationException("Socket is not connected.");

        byte[] buffer = Encoding.ASCII.GetBytes(command);

        Log($"Sending data ({buffer.Length} bytes).");

        if (encrypt)
        {
            if (_txCrypto == null)
                throw new InvalidOperationException("TX crypto is not initialized.");

            _txCrypto.Process(buffer, buffer.Length);
        }

        int offset = 0;

        while (offset < buffer.Length)
        {
            int sent = _socket.Send(
                buffer,
                offset,
                buffer.Length - offset,
                SocketFlags.None);

            if (sent <= 0)
                throw new SocketException((int)SocketError.ConnectionReset);

            offset += sent;
        }
    }

    private string ReceiveText(bool encrypt = true)
    {
        if (_socket == null)
            throw new InvalidOperationException("Socket is not connected.");

        byte[] buffer = new byte[BufferSize];

        int length = _socket.Receive(
            buffer,
            0,
            buffer.Length,
            SocketFlags.None);

        if (length == 0)
            throw new SocketException((int)SocketError.ConnectionReset);

        Log($"Received data ({length} bytes).");

        if (encrypt && _encrypted)
        {
            if (_rxCrypto == null)
                throw new InvalidOperationException("RX crypto is not initialized.");

            _rxCrypto.Process(buffer, length);
        }

        return Encoding.ASCII.GetString(buffer, 0, length);
    }

    // ---------------------------------------------------------
    // Disposal
    // ---------------------------------------------------------

    public void Dispose()
    {
        if (_disposed)
            return;

        _disposed = true;

        try
        {
            _socket?.Shutdown(SocketShutdown.Both);
        }
        catch (SocketException)
        {
            // Соединение могло быть уже закрыто.
        }
        catch (ObjectDisposedException)
        {
        }

        _socket?.Dispose();
        _socket = null;

        _encrypted = false;
    }
}
