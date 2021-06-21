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
#include "printing_macros.h"
#include "ra3_constants.h"
#include "printing_tools.h"
#include "peerchat_encryption.h"
#include "ra3_engine_tools/ra3_engine_tools.h"
#include "client_info.h"

int g_socket_fd__peerchat;
sockaddr_in g_peerchat_addr;

gs_peerchat_ctx g_peerchat_context__tx;
gs_peerchat_ctx g_peerchat_context__rx;
bool g_encrypted = false;

std::string g_our_ip;

int init_peerchat_socket()
{
    // Create a socket
    if ((g_socket_fd__peerchat = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1)
        return -1;

    // zero out the structure
    memset((char *)&g_peerchat_addr, 0, sizeof(g_peerchat_addr));

    g_peerchat_addr.sin_family = AF_INET;
    g_peerchat_addr.sin_port = htons(RA3_STRING_SERVER_PEERCHAT_PORT);
    g_peerchat_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_PEERCHAT);
    if (g_peerchat_addr.sin_addr.s_addr == -1)
    {
        PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_PEERCHAT);
        auto pHostent = gethostbyname(RA3_STRING_SERVER_PEERCHAT);
        if (!pHostent)
        {
            PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_PEERCHAT);
            return 0;
        }
        PROCESS_MESSAGE__OK("Alternative way worked!");
        g_peerchat_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
    }

    int32_t optval = 1;
    setsockopt(g_socket_fd__peerchat, 0xFFFF, SO_RCVBUF, &optval, sizeof(optval));

    sockaddr_in local_addr;
    memset((char *)&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;

    // For some reason 0.0.0.0:0 is used in original RA3
    local_addr.sin_addr.s_addr = htonl(0);
    local_addr.sin_port = 0;

    if (bind(g_socket_fd__peerchat, (sockaddr *)&local_addr, sizeof(local_addr)) == -1 ||
        connect(g_socket_fd__peerchat, (sockaddr *)&g_peerchat_addr, sizeof(g_peerchat_addr)) == -1)
    {
        PROCESS_MESSAGE__ERROR("Could not bind or connect!");
        close(g_socket_fd__peerchat);
        return 0;
    }

    PROCESS_MESSAGE__OK("Connected!");

    return 1;
}

void send_buffer(uint8_t* buff, uint32_t size)
{
    PROCESS_MESSAGE__DEBUG("Sending data:");
    PROCESS_BUFMESSAGE__DEBUG(buff, size);

    if (g_encrypted)
    {
        gs_peerchat(&g_peerchat_context__tx, buff, size);
        PROCESS_MESSAGE__DEBUG("Encryption is enabled - data after encryption:");
        PROCESS_BUFMESSAGE__DEBUG(buff, size);
    }

    int code = send(g_socket_fd__peerchat, buff, size, 0);
    if (code == -1)
        PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno);
}

uint32_t recv_buffer(uint8_t* buff, uint32_t size)
{
    auto len = recv(g_socket_fd__peerchat, buff, size, 0);

    if (len == -1)
        PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno)
    else
    {
        if (g_encrypted)
        {
            PROCESS_MESSAGE__DEBUG("Encryption is enabled - data before decryption:");
            PROCESS_BUFMESSAGE__DEBUG(buff, len);
            gs_peerchat(&g_peerchat_context__rx, buff, len);
        }
        PROCESS_MESSAGE__DEBUG("Received data:");
        PROCESS_BUFMESSAGE__DEBUG(buff, len);
    }

    return len;
}

std::vector<std::string> split_string(std::string str)
{
    std::stringstream ss(str);
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    return vstrings;
}

void send_CRYPT()
{
    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    auto string = string_format("CRYPT des %d %s\r\n", 1, RA3_STRING_GAMENAME);
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);

    send_buffer(buff, length);
    length = recv_buffer(buff, size);

    // Split received string using space symbol
    auto parts = split_string(std::string((char*)buff));
    if (parts.size() != 5 || parts[0] != ":s" || parts[1] != "705" || parts[2] != "*")
    {
        PROCESS_MESSAGE__ERROR("Wrong response format!");
        return;
    }

    PROCESS_MESSAGE__OK("Successfuly received keys!");
    PROCESS_MESSAGE__OK("TX Key: %s", parts[3].c_str());
    PROCESS_MESSAGE__OK("RX Key: %s", parts[4].c_str());

    PROCESS_MESSAGE__INFO("Initializing encryption context...");
    gs_peerchat_init(&g_peerchat_context__tx, (uint8_t*)parts[3].c_str(), (uint8_t*)RA3_STRING_PEERCHAT_SECRET_KEY);
    gs_peerchat_init(&g_peerchat_context__rx, (uint8_t*)parts[4].c_str(), (uint8_t*)RA3_STRING_PEERCHAT_SECRET_KEY);
    g_encrypted = true;
    PROCESS_MESSAGE__OK("Done!");
}

void send_USRIP()
{
    if (!g_encrypted)
        return;

    PROCESS_MESSAGE__INFO("Getting our IP from server...");

    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    auto string = string_format("USRIP\r\n");
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);

    send_buffer(buff, length);
    length = recv_buffer(buff, size);

    if (length != -1)
    {
        auto packet_text = std::string((char*)buff);

        // We save IP string (we cut off packet header and last two symbols: '\r' and '\n')
        g_our_ip = packet_text.substr(12, packet_text.size() - 14);

        PROCESS_MESSAGE__INFO("Got our IP: %s", g_our_ip.c_str());
    }
}

void send_USER()
{
    if (!g_encrypted)
        return;

    PROCESS_MESSAGE__INFO("Sending user data (our IP, GSProfileID and CD-KEY)...");

    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    // Encode our IP address to send it back
    auto enc_ip_addr = encode_inet_addr_to_string(inet_addr(g_our_ip.c_str()), 0);

    auto string = string_format("USER %s|%s 127.0.0.1 %s :%s\r\n"
                                "NICK %s\r\n",
                                // Here are parameters:
                                string_format("X%sX", enc_ip_addr.c_str()).c_str(),
                                nuloginpersona__profile_id__get().c_str(),
                                RA3_STRING_SERVER_PEERCHAT_ORIGINAL,
                                RA3_ACCOUNT_CDKEY_ENCODED,
                                RA3_ACCOUNT_ID);

    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);

    send_buffer(buff, length);
    length = recv_buffer(buff, size);
    buff[length] = '\0';

    PROCESS_MESSAGE__OK("Received:\n%s", buff);
}

void send_QUIT()
{
    if (!g_encrypted)
        return;

    PROCESS_MESSAGE__INFO("Sending quit...");

    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    auto string = string_format("QUIT :Later!\r\n");
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);

    send_buffer(buff, length);
    length = recv_buffer(buff, size);
}

void send_CDKEY()
{
    if (!g_encrypted)
        return;

    PROCESS_MESSAGE__INFO("Sending CD-KEY...");

    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    // Send CD-KEY
    auto string = string_format("CDKEY %s\r\n", RA3_ACCOUNT_CDKEY);
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);
    send_buffer(buff, length);

    // Receive result
    length = recv_buffer(buff, size);
    buff[length] = '\0';
    auto res = std::string((char*)buff);
    if (res.find("Authenticated") != res.npos)
    {
        if (res.find("PING") != res.npos)
        {
            PROCESS_MESSAGE__INFO("Got PING.");

            // Send PONG
            auto string = string_format("PONG :s\r\n");
            int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
            memcpy(buff, string.c_str(), length);
            send_buffer(buff, length);

            PROCESS_MESSAGE__INFO("Sent PONG.");
        }

        PROCESS_MESSAGE__OK("Authenticated successfuly!");
    }
}

void send_JOIN()
{
    if (!g_encrypted)
        return;

    PROCESS_MESSAGE__INFO("Joining...");

    const int size = 2048;
    uint8_t buff[size];
    memset(buff, 0x00, size);

    // Send JOIN request
    auto string = string_format("JOIN %s\r\n", "#GPG!2166");
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!
    memcpy(buff, string.c_str(), length);
    send_buffer(buff, length);

    // Receive result
    length = recv_buffer(buff, size);
    buff[length] = '\0';
    auto res = std::string((char*)buff);
    PROCESS_MESSAGE__INFO("Received lobby info:\n%s", res.c_str());
}

void process_peerchat_connection()
{
    init_peerchat_socket();

    // Get encryption keys from server
    send_CRYPT();

    // Get USeR IP
    send_USRIP();

    // Send user info and get MOTD (Message of the Day) from server
    send_USER();

    // Send CD-KEY and finally authenticate
    send_CDKEY();

    send_JOIN();

    // Finish :)
    send_QUIT();

    PROCESS_MESSAGE__DEBUG("Done!");
}