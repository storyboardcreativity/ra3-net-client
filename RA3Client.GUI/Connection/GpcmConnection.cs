using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading.Tasks;

// GPCM - GameSpy Connection Manager

public sealed class GpcmConnection : IDisposable
{
    private readonly string _preAuthTicket;
    private readonly string _preAuthChallenge;
    private string _initialChallenge = "";
    private string _ourChallenge = "";
    private TcpClient? _client;
    private NetworkStream? _stream;

    public GpcmConnection(string preAuthTicket, string preAuthChallenge)
    {
        _preAuthTicket = preAuthTicket;
        _preAuthChallenge = preAuthChallenge;
    }

    public async Task<bool> InitAsync()
    {
        try
        {
            // Resolve the server address.
            IPAddress address;

            if (!IPAddress.TryParse(
                    RA3Constants.RA3_STRING_SERVER_GPCM,
                    out address!))
            {
                Console.WriteLine(
                    $"Could not parse server address " +
                    $"{RA3Constants.RA3_STRING_SERVER_GPCM}. Resolving hostname...");

                var addresses = await Dns.GetHostAddressesAsync(
                    RA3Constants.RA3_STRING_SERVER_GPCM);

                address = Array.Find(
                    addresses,
                    a => a.AddressFamily == AddressFamily.InterNetwork)
                    ?? throw new SocketException(
                        (int)SocketError.HostNotFound);
            }

            _client = new TcpClient(AddressFamily.InterNetwork);

            // Connect asynchronously.
            await _client.ConnectAsync(
                address,
                RA3Constants.RA3_STRING_SERVER_GPCM_PORT);

            _stream = _client.GetStream();

            Console.WriteLine("Connected!");

            return true;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Could not connect: {ex.Message}");
            Dispose();
            return false;
        }
    }

    public async Task SendKaFinalAsync()
    {
        string request = @"\ka\\final\";

        // Send the request without a null terminator.
        await SendGpcmBufferAsync(request);

        // Receive the response.
        byte[] buffer = await ReceiveGpcmBufferAsync(2048);
        string packet = Encoding.ASCII.GetString(buffer);

        const string challengeMarker = @"challenge\";
        const string idMarker = @"\id";

        int challengeStart = packet.IndexOf(
            challengeMarker,
            StringComparison.Ordinal);

        int idStart = packet.IndexOf(
            idMarker,
            StringComparison.Ordinal);

        if (challengeStart < 0 || idStart < 0)
        {
            throw new InvalidDataException(
                $"Unexpected GPCM response: {packet}");
        }

        challengeStart += challengeMarker.Length;

        if (idStart < challengeStart)
        {
            throw new InvalidDataException(
                $"Invalid challenge in GPCM response: {packet}");
        }

        string challenge = packet.Substring(
            challengeStart,
            idStart - challengeStart);

        _initialChallenge = challenge;

        Console.WriteLine($"Initial challenge => {challenge}");
        Console.WriteLine("Done!");
    }

    public async Task SendLoginChallengeAsync()
    {
        // FIXME: The original implementation uses a constant challenge.
        _ourChallenge = "wN1sOMYBAX3pci0t1cbsxa7W8PvVbwnr";

        string request = string.Format(
            @"\login\\challenge\{0}\authtoken\{1}\partnerid\0\" +
            @"response\{2}\firewall\1\port\0\productid\11419\" +
            @"gamename\{3}\namespaceid\1\sdkrevision\11\" +
            @"quiet\0\id\1\final\",
            _ourChallenge,
            _preAuthTicket,
            RA3Crypto.GetGpcmChallengeResponse(_preAuthTicket, _preAuthChallenge, _ourChallenge, _initialChallenge),
            RA3Constants.RA3_STRING_GAMENAME);

        await SendGpcmBufferAsync(request);

        // Wait for the login response.
        byte[] buffer = await ReceiveGpcmBufferAsync(2048);
        string response = Encoding.ASCII.GetString(buffer);

        Console.WriteLine($"GPCM login response: {response}");
        Console.WriteLine("Done!");
    }

    private async Task SendGpcmBufferAsync(string data)
    {
        if (_stream == null)
        {
            throw new InvalidOperationException(
                "GPCM connection is not initialized.");
        }

        byte[] buffer = Encoding.ASCII.GetBytes(data);

        Console.WriteLine("Sending data:");
        Console.WriteLine(data);

        await _stream.WriteAsync(buffer, 0, buffer.Length);
    }

    private async Task<byte[]> ReceiveGpcmBufferAsync(int size)
    {
        if (_stream == null)
        {
            throw new InvalidOperationException(
                "GPCM connection is not initialized.");
        }

        byte[] buffer = new byte[size];

        int length = await _stream.ReadAsync(
            buffer, 0, buffer.Length);

        if (length == 0)
        {
            throw new IOException(
                "The GPCM server closed the connection.");
        }

        Array.Resize(ref buffer, length);

        Console.WriteLine("Received data:");
        Console.WriteLine(Encoding.ASCII.GetString(buffer));

        return buffer;
    }

    public void Dispose()
    {
        _stream?.Dispose();
        _client?.Dispose();

        _stream = null;
        _client = null;
    }
}

public static class GpcmProcessor
{
    public static async Task ProcessGpcmConnectionAsync(IProgress<(int percent, string message)>? progress, string preAuthTicket, string preAuthChallenge)
    {
        progress?.Report((1, "Starting GPCM session"));

        using var connection = new GpcmConnection(preAuthTicket, preAuthChallenge);

        progress?.Report((40, "Initializing GPCM connection"));

        if (!await connection.InitAsync())
        {
            return;
        }

        progress?.Report((60, "Requesting challenge"));

        await connection.SendKaFinalAsync();

        progress?.Report((80, "Logging in"));

        await connection.SendLoginChallengeAsync();

        progress?.Report((100, "Finished"));
        Console.WriteLine("Done!");
    }
}
