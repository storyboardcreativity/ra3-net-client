using System;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Threading;

public static class GameSpy
{
    private static Socket? _socket_fd = null;

    private static readonly byte[] _gamespy_ping_out_buff = new byte[64];
    private static int _gamespy_ping_out_buff_len = 0;

    private static IPEndPoint? _gamespy_addr = null;

    private static void Debug(string message)
    {
        Console.WriteLine("[DEBUG] " + message);
    }

    private static void Warning(string message)
    {
        Console.WriteLine("[WARNING] " + message);
    }

    private static void Error(string message)
    {
        Console.WriteLine("[ERROR] " + message);
    }

    private static void Ok(string message)
    {
        Console.WriteLine("[OK] " + message);
    }

    public static void FillGameSpyPingOutBuff()
    {
        // Clean buffer
        Array.Clear(_gamespy_ping_out_buff);

        // Calculate size to use
        byte[] gameName = Encoding.ASCII.GetBytes(
            RA3Constants.RA3_STRING_GAMENAME);

        _gamespy_ping_out_buff_len = 1 + 4 + gameName.Length + 1;

        // Set version
        _gamespy_ping_out_buff[0] = 9;

        // Set game name
        Array.Copy(
            gameName,
            0,
            _gamespy_ping_out_buff,
            5,
            gameName.Length);
    }

    //
    // Can be found in original ra3_1.12.game by string:
    // "%s.available.gamespy.com"
    //
    public static int InitGameSpyAddr()
    {
        try
        {
            // Create a UDP socket
            _socket_fd = new Socket(
                AddressFamily.InterNetwork,
                SocketType.Dgram,
                ProtocolType.Udp);
        }
        catch (SocketException)
        {
            return -1;
        }

        string server = RA3Constants.RA3_STRING_SERVER_GAMESPY_AVAIL;

        IPAddress? address = null;

        // Equivalent to inet_addr()
        if (IPAddress.TryParse(server, out IPAddress? parsedAddress))
        {
            address = parsedAddress;
        }

        // Alternative DNS resolution
        if (address == null)
        {
            Warning(
                $"Could not resolve server {server}! " +
                "Attempting to use alternative way...");

            try
            {
                foreach (IPAddress ip in Dns.GetHostAddresses(server))
                {
                    if (ip.AddressFamily == AddressFamily.InterNetwork)
                    {
                        address = ip;
                        break;
                    }
                }
            }
            catch (SocketException)
            {
                // DNS resolution failed
            }

            if (address == null)
            {
                Error($"Finally could not resolve server {server}!");
                return 0;
            }

            Ok("Alternative way worked!");
        }

        _gamespy_addr = new IPEndPoint(address, 27900);

        return 1;
    }

    public static void GameSpySendUDPAvail()
    {
        if (_socket_fd == null || _gamespy_addr == null)
            return;

        Debug(
            $"GameSpy UDP sending: " +
            $"{_gamespy_ping_out_buff[0]:X2} " +
            $"{_gamespy_ping_out_buff[1]:X2} " +
            $"{_gamespy_ping_out_buff[2]:X2} " +
            $"{_gamespy_ping_out_buff[3]:X2}...");

        try
        {
            _socket_fd.SendTo(
                _gamespy_ping_out_buff,
                0,
                _gamespy_ping_out_buff_len,
                SocketFlags.None,
                _gamespy_addr);
        }
        catch (SocketException ex)
        {
            Error($"UDP send failed: {ex.Message}");
        }
    }

    public static int GameSpyCheckBuffer(
        byte[] buff,
        int size,
        out uint answer)
    {
        answer = 0;

        if (size < 7)
            return 1;

        int val0 = buff[0] - 0xFE;

        Debug(
            $"GameSpy UDP ping answer magic: " +
            $"{buff[0]:X2} {buff[1]:X2} {buff[2]:X2}");

        // Check magic: FE FD 09
        if (buff[0] != 0xFE ||
            buff[1] != 0xFD ||
            buff[2] != 0x09)
        {
            return val0 <= 0 ? -1 : 1;
        }

        // Construct result field (big-endian)
        answer =
            ((uint)buff[3] << 24) |
            ((uint)buff[4] << 16) |
            ((uint)buff[5] << 8) |
            buff[6];

        Debug($"GameSpy UDP ping answer: 0x{answer:X}");

        return 0;
    }

    public static int GameSpyRecvUDPAvail()
    {
        if (_socket_fd == null)
            return 0;

        byte[] buf = new byte[64];

        EndPoint remoteEndPoint = new IPEndPoint(
            IPAddress.Any,
            0);

        int len;

        try
        {
            len = _socket_fd.ReceiveFrom(
                buf,
                0,
                buf.Length,
                SocketFlags.None,
                ref remoteEndPoint);
        }
        catch (SocketException)
        {
            return 0;
        }

        uint answer;

        if (GameSpyCheckBuffer(buf, len, out answer) != 0)
        {
            Error("Wrong UDP answer from avail gamespy server!");
        }

        return 1;
    }

    public static void ProcessGameSpyAvailabilityPing()
    {
        // Prepare buffer
        FillGameSpyPingOutBuff();

        // Prepare socket
        int result = InitGameSpyAddr();

        if (result != 1)
            return;

        // Send UDP ping
        GameSpySendUDPAvail();

        // Attempt to receive UDP ping
        while (GameSpyRecvUDPAvail() == 0)
        {
            Thread.Sleep(10);
        }
    }

    public static void Close()
    {
        _socket_fd?.Dispose();
        _socket_fd = null;
        _gamespy_addr = null;
    }
}
