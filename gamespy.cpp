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

// own headers
#include "printing_macros.h"
#include "ra3_constants.h"

int g_socket_fd = -1;
char g_gamespy_ping_out_buff[64];
int g_gamespy_ping_out_buff_len = 0;

struct sockaddr_in g_gamespy_addr;

void fill_gamespy_ping_out_buff()
{
    // Clean buffer
    memset(g_gamespy_ping_out_buff, 0x00, sizeof(g_gamespy_ping_out_buff));

    // Calculate size to use
    g_gamespy_ping_out_buff_len = 1 + 4 + strlen(RA3_STRING_GAMENAME) + 1;

    // Set ??version??
    g_gamespy_ping_out_buff[0] = 9;

    // Set game name
    memcpy(&g_gamespy_ping_out_buff[5], RA3_STRING_GAMENAME, strlen(RA3_STRING_GAMENAME));
}

//
// Can be found in original ra3_1.12.game by string: "%s.available.gamespy.com"
//
int init_gamespy_addr()
{
    // Create a UDP socket
    if ((g_socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        return -1;

    // zero out the structure
    memset((char *)&g_gamespy_addr, 0, sizeof(g_gamespy_addr));

    g_gamespy_addr.sin_family = AF_INET;
    g_gamespy_addr.sin_port = htons(27900);
    g_gamespy_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_GAMESPY_AVAIL);
    if (g_gamespy_addr.sin_addr.s_addr == -1)
    {
        PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_GAMESPY_AVAIL);
        auto pHostent = gethostbyname(RA3_STRING_SERVER_GAMESPY_AVAIL);
        if (!pHostent)
        {
            PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_GAMESPY_AVAIL);
            return 0;
        }
        PROCESS_MESSAGE__OK("Alternative way worked!");
        g_gamespy_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
    }

    return 1;
}

void gamespy_send_UDP_avail()
{
    PROCESS_MESSAGE__DEBUG("GameSpy UDP sending: %02X %02X %02X %02X...",
        g_gamespy_ping_out_buff[0],
        g_gamespy_ping_out_buff[1],
        g_gamespy_ping_out_buff[2],
        g_gamespy_ping_out_buff[3]);
    sendto(g_socket_fd, g_gamespy_ping_out_buff, g_gamespy_ping_out_buff_len, 0, (struct sockaddr *)&g_gamespy_addr, 16);
}

int gamespy_check_buffer(uint8_t* buff, uint32_t size, uint32_t& answer)
{
    if (size < 7)
        return 1;
    
    int val0 = buff[0] - 0xFE;
    int val1 = buff[1] - 0xFD;
    int val2 = buff[2] - 0x09;

    PROCESS_MESSAGE__DEBUG("GameSpy UDP ping answer magic: %02X %02X %02X", buff[0], buff[1], buff[2]);

    // Check magic - <FE FD 09>
    if (!(buff[0] == 0xFE && buff[1] == 0xFD && buff[2] == 0x09))
        return val0 <= 0 ? -1 : 1;

    // Construct result field
    answer = buff[6] | ((buff[5] | ((buff[4] | ((int8_t)buff[3] << 8)) << 8)) << 8);
    PROCESS_MESSAGE__DEBUG("GameSpy UDP ping answer: 0x%X", answer);

    return 0;
}

int gamespy_recv_UDP_avail()
{
    uint8_t buf[64];
    socklen_t fromlen = 16;

    struct sockaddr from;
    int len = recvfrom(g_socket_fd, buf, sizeof(buf), 0, &from, &fromlen);
    if (len == -1)
        return 0;

    uint32_t answer = 0;
    if (gamespy_check_buffer(buf, len, answer))
        PROCESS_MESSAGE__ERROR("Wrong UDP answer from avail gamespy server!");
    
    return 1;
}

void process_gamespy_avail_ping()
{
    // Prepare buffer
    fill_gamespy_ping_out_buff();

    // Prepare socket
    init_gamespy_addr();

    // Send UDP ping
    gamespy_send_UDP_avail();

    // Attempt to receive UDP ping
    for (int i = gamespy_recv_UDP_avail(); !i; i = gamespy_recv_UDP_avail())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
