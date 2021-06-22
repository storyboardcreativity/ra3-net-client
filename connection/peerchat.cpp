// std
#include <stdio.h>
#include <sstream>
#include <string>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <chrono>
#include <thread>
#include <vector>
#include <iterator>

// own headers
#include "../printing_macros.h"
#include "../ra3_constants.h"
#include "../printing_tools.h"
#include "../peerchat_encryption.h"
#include "../ra3_engine_tools/ra3_engine_tools.h"

#include "peerchat.hpp"

class peerchat_connection : public IPeerchatConnection
{
public:
    peerchat_connection(ra3_client_info& client_info)
        : _client_info(client_info) {}

    bool init()
    {
        // Create a socket
        if ((_socket_fd__peerchat = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1)
            return false;

        // zero out the structure
        memset((char *)&_peerchat_addr, 0, sizeof(_peerchat_addr));

        _peerchat_addr.sin_family = AF_INET;
        _peerchat_addr.sin_port = htons(RA3_STRING_SERVER_PEERCHAT_PORT);
        _peerchat_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_PEERCHAT);
        if (_peerchat_addr.sin_addr.s_addr == -1)
        {
            PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_PEERCHAT);
            auto pHostent = gethostbyname(RA3_STRING_SERVER_PEERCHAT);
            if (!pHostent)
            {
                PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_PEERCHAT);
                return false;
            }
            PROCESS_MESSAGE__OK("Alternative way worked!");
            _peerchat_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
        }

        int32_t optval = 1;
        setsockopt(_socket_fd__peerchat, 0xFFFF, SO_RCVBUF, &optval, sizeof(optval));

        sockaddr_in local_addr;
        memset((char *)&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;

        // For some reason 0.0.0.0:0 is used in original RA3
        local_addr.sin_addr.s_addr = htonl(0);
        local_addr.sin_port = 0;

        if (bind(_socket_fd__peerchat, (sockaddr *)&local_addr, sizeof(local_addr)) == -1 ||
            connect(_socket_fd__peerchat, (sockaddr *)&_peerchat_addr, sizeof(_peerchat_addr)) == -1)
        {
            PROCESS_MESSAGE__ERROR("Could not bind or connect!");
            close(_socket_fd__peerchat);
            return false;
        }

        PROCESS_MESSAGE__OK("Connected!");

        return true;
    }

    void send_CRYPT()
    {
        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        auto string = string_format("CRYPT des %d %s\r\n", 1, RA3_STRING_GAMENAME);
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);

        send_buffer(buff, length);
        length = recv_buffer(buff, size);

        // Split received string using space symbol
        auto parts = split_string(std::string((char *)buff));
        if (parts.size() != 5 || parts[0] != ":s" || parts[1] != "705" || parts[2] != "*")
        {
            PROCESS_MESSAGE__ERROR("Wrong response format!");
            return;
        }

        PROCESS_MESSAGE__OK("Successfuly received keys!");
        PROCESS_MESSAGE__OK("TX Key: %s", parts[3].c_str());
        PROCESS_MESSAGE__OK("RX Key: %s", parts[4].c_str());

        PROCESS_MESSAGE__INFO("Initializing encryption context...");
        gs_peerchat_init(&_peerchat_context__tx, (uint8_t *)parts[3].c_str(), (uint8_t *)RA3_STRING_PEERCHAT_SECRET_KEY);
        gs_peerchat_init(&_peerchat_context__rx, (uint8_t *)parts[4].c_str(), (uint8_t *)RA3_STRING_PEERCHAT_SECRET_KEY);
        _encrypted = true;
        PROCESS_MESSAGE__OK("Done!");
    }

    void send_USRIP()
    {
        if (!_encrypted)
            return;

        PROCESS_MESSAGE__INFO("Getting our IP from server...");

        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        auto string = string_format("USRIP\r\n");
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);

        send_buffer(buff, length);
        length = recv_buffer(buff, size);

        if (length != -1)
        {
            auto packet_text = std::string((char *)buff);

            // We save IP string (we cut off packet header and last two symbols: '\r' and '\n')
            _our_ip = packet_text.substr(12, packet_text.size() - 14);

            PROCESS_MESSAGE__INFO("Got our IP: %s", _our_ip.c_str());
        }
    }

    void send_USER()
    {
        if (!_encrypted)
            return;

        PROCESS_MESSAGE__INFO("Sending user data (our IP, GSProfileID and CD-KEY)...");

        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        // Encode our IP address to send it back
        auto enc_ip_addr = encode_inet_addr_to_string(inet_addr(_our_ip.c_str()), 0);

        auto string = string_format("USER %s|%s 127.0.0.1 %s :%s\r\n"
                                    "NICK %s\r\n",
                                    // Here are parameters:
                                    string_format("X%sX", enc_ip_addr.c_str()).c_str(),
                                    _client_info.nuloginpersona__profile_id__get().c_str(),
                                    RA3_STRING_SERVER_PEERCHAT_ORIGINAL,
                                    RA3_ACCOUNT_CDKEY_ENCODED,
                                    RA3_ACCOUNT_ID);

        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);

        send_buffer(buff, length);
        length = recv_buffer(buff, size);
        buff[length] = '\0';

        PROCESS_MESSAGE__OK("Received:\n%s", buff);
    }

    void send_CDKEY()
    {
        if (!_encrypted)
            return;

        PROCESS_MESSAGE__INFO("Sending CD-KEY...");

        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        // Send CD-KEY
        auto string = string_format("CDKEY %s\r\n", RA3_ACCOUNT_CDKEY);
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);
        send_buffer(buff, length);

        // Receive result
        length = recv_buffer(buff, size);
        buff[length] = '\0';
        auto res = std::string((char *)buff);
        if (res.find("Authenticated") != res.npos)
        {
            if (res.find("PING") != res.npos)
            {
                PROCESS_MESSAGE__INFO("Got PING.");

                // Send PONG
                auto string = string_format("PONG :s\r\n");
                int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
                memcpy(buff, string.c_str(), length);
                send_buffer(buff, length);

                PROCESS_MESSAGE__INFO("Sent PONG.");
            }

            PROCESS_MESSAGE__OK("Authenticated successfuly!");
        }
    }

    std::vector<std::string> send_JOIN(std::string channel_name)
    {
        if (!_encrypted)
            return std::vector<std::string>();

        PROCESS_MESSAGE__INFO("Joining...");

        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        // Send JOIN request
        auto string = string_format("JOIN %s\r\n", channel_name.c_str());
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);
        send_buffer(buff, length);

        // Receive result
        length = recv_buffer(buff, size);
        buff[length] = '\0';
        auto res = std::string((char *)buff);
        PROCESS_MESSAGE__INFO("Received lobby info:\n%s", res.c_str());

        auto off_353 = res.find("\r\n:s 353");
        auto off_366 = res.find("\r\n:s 366");
        auto subres_players = res.substr(off_353, off_366 - off_353);
        subres_players = subres_players.substr(subres_players.find(channel_name) + channel_name.size() + 2);
        return split_string(subres_players);
    }

    void send_QUIT()
    {
        if (!_encrypted)
            return;

        PROCESS_MESSAGE__INFO("Sending quit...");

        const int size = 2048;
        uint8_t buff[size];
        memset(buff, 0x00, size);

        auto string = string_format("QUIT :Later!\r\n");
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!
        memcpy(buff, string.c_str(), length);

        send_buffer(buff, length);
        length = recv_buffer(buff, size);
    }

private:
    ra3_client_info& _client_info;

    int _socket_fd__peerchat;
    sockaddr_in _peerchat_addr;

    gs_peerchat_ctx _peerchat_context__tx;
    gs_peerchat_ctx _peerchat_context__rx;
    bool _encrypted = false;

    std::string _our_ip;

    void send_buffer(uint8_t *buff, uint32_t size)
    {
        PROCESS_MESSAGE__DEBUG("Sending data:");
        PROCESS_BUFMESSAGE__DEBUG(buff, size);

        if (_encrypted)
        {
            gs_peerchat(&_peerchat_context__tx, buff, size);
            PROCESS_MESSAGE__DEBUG("Encryption is enabled - data after encryption:");
            PROCESS_BUFMESSAGE__DEBUG(buff, size);
        }

        int code = send(_socket_fd__peerchat, buff, size, 0);
        if (code == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno);
    }

    uint32_t recv_buffer(uint8_t *buff, uint32_t size)
    {
        auto len = recv(_socket_fd__peerchat, buff, size, 0);

        if (len == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno)
        else
        {
            if (_encrypted)
            {
                PROCESS_MESSAGE__DEBUG("Encryption is enabled - data before decryption:");
                PROCESS_BUFMESSAGE__DEBUG(buff, len);
                gs_peerchat(&_peerchat_context__rx, buff, len);
            }
            PROCESS_MESSAGE__DEBUG("Received data:");
            PROCESS_BUFMESSAGE__DEBUG(buff, len);
        }

        return len;
    }
};

IPeerchatConnection* process_peerchat_connection(ra3_client_info& client_info)
{
    auto connection = new peerchat_connection(client_info);

    connection->init();

    // Get encryption keys from server
    connection->send_CRYPT();

    // Get USeR IP
    connection->send_USRIP();

    // Send user info and get MOTD (Message of the Day) from server
    connection->send_USER();

    // Send CD-KEY and finally authenticate
    connection->send_CDKEY();

    PROCESS_MESSAGE__DEBUG("Done!");

    return connection;
}