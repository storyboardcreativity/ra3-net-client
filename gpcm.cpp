/*

GPCM - GameSpy Connection Manager

*/

// std
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <netdb.h>
#include <signal.h>
#include <poll.h>

// own headers
#include "printing_macros.h"
#include "ra3_constants.h"
#include "printing_tools.h"
#include "client_info.h"
#include "ra3_engine_tools/ra3_engine_tools.h"

int g_socket_fd__gpcm;
sockaddr_in g_gpcm_addr;

int init_gpcm_socket()
{
    // Create a socket
    if ((g_socket_fd__gpcm = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1)
        return -1;
    
    // Make it non-blocking
    int flags = fcntl(g_socket_fd__gpcm, F_GETFL, 0);
    fcntl(g_socket_fd__gpcm, F_SETFL, flags | O_NONBLOCK);

    // Init address structure
    memset((char *)&g_gpcm_addr, 0, sizeof(g_gpcm_addr));
    g_gpcm_addr.sin_family = AF_INET;
    g_gpcm_addr.sin_port = htons(RA3_STRING_SERVER_GPCM_PORT);
    g_gpcm_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_GPCM);
    if (g_gpcm_addr.sin_addr.s_addr == -1)
    {
        PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_GPCM);
        auto pHostent = gethostbyname(RA3_STRING_SERVER_GPCM);
        if (!pHostent)
        {
            PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_GPCM);
            return 0;
        }
        PROCESS_MESSAGE__OK("Alternative way worked!");
        g_gpcm_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
    }

    // Connect to server
    if (connect(g_socket_fd__gpcm, (sockaddr *)&g_gpcm_addr, sizeof(g_gpcm_addr)) == -1)
    {
        if (errno == EINPROGRESS)
        {
            PROCESS_MESSAGE__WARNING("connect(...) operation is now in progress. Waiting for connection...");
            
            pollfd pfd;
            memset(&pfd, 0x00, sizeof(pfd));
            pfd.fd = g_socket_fd__gpcm;
            pfd.events = POLLIN;
            poll(&pfd, 1, -1);
        }
        else
        {
            PROCESS_MESSAGE__ERROR("Could not connect! Error msg: %s (%d)", strerror(errno), errno);
            close(g_socket_fd__gpcm);
            return 0;
        }
    }

    PROCESS_MESSAGE__OK("Connected!");

    return 1;
}

void send_gpcm_buffer(uint8_t* buff, uint32_t size)
{
    PROCESS_MESSAGE__DEBUG("Sending data:");
    PROCESS_BUFMESSAGE__DEBUG(buff, size);

    int code = send(g_socket_fd__gpcm, buff, size, 0);
    if (code == -1)
        PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno);
}

uint32_t recv_gpcm_buffer(uint8_t* buff, uint32_t size)
{
    auto len = recv(g_socket_fd__gpcm, buff, size, 0);

    if (len == -1)
        PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno)
    else
    {
        PROCESS_MESSAGE__DEBUG("Received data:");
        PROCESS_BUFMESSAGE__DEBUG(buff, len);
    }

    return len;
}

void send_ka_final()
{
    auto string = string_format("\\ka\\\\final\\");
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!

    // Send request
    send_gpcm_buffer((uint8_t*)string.c_str(), length);

    // Receive response
    uint8_t buffer[2048];
    length = recv_gpcm_buffer(buffer, sizeof(buffer));
    buffer[length] = '\0';

    auto packet = std::string((char*)buffer);
    auto offsetA = packet.find("challenge\\") + 10;
    auto offsetB = packet.find("\\id");
    auto challenge = packet.substr(offsetA, offsetB - offsetA);
    gpcm__initial_challenge__set(challenge);
    PROCESS_MESSAGE__INFO("Initial challenge => %s", challenge.c_str());

    PROCESS_MESSAGE__OK("Done!");
}

void send_login_challenge()
{
    // FIXME: We use constant challenge because we don't have generating algorithm
    gpcm__our_challenge__set("wN1sOMYBAX3pci0t1cbsxa7W8PvVbwnr");

    auto string = string_format("\\"
                                "login\\\\"
                                "challenge\\%s\\"
                                "authtoken\\%s\\"
                                "partnerid\\0\\"
                                "response\\%s\\"
                                "firewall\\1\\"
                                "port\\0\\"
                                "productid\\11419\\"
                                "gamename\\%s\\"
                                "namespaceid\\1\\"
                                "sdkrevision\\11\\"
                                "quiet\\0\\"
                                "id\\1\\"
                                "final\\",
                                // Parameters:
                                gpcm__our_challenge__get().c_str(),
                                preauth__ticket__get().c_str(),
                                get_gpcm_challenge_response().c_str(),
                                RA3_STRING_GAMENAME);
    int length = strlen(string.c_str());    // Important: WITHOUT null terminator!

    // Send request
    send_gpcm_buffer((uint8_t*)string.c_str(), length);

    pollfd pfd;
    memset(&pfd, 0x00, sizeof(pfd));
    pfd.fd = g_socket_fd__gpcm;
    pfd.events = POLLIN;
    poll(&pfd, 1, -1);

    // Receive response
    uint8_t buffer[2048];
    length = recv_gpcm_buffer(buffer, sizeof(buffer));
    buffer[length] = '\0';

    PROCESS_MESSAGE__OK("Done!");
}

void process_gpcm_connection()
{
    init_gpcm_socket();

    send_ka_final();

    send_login_challenge();

    PROCESS_MESSAGE__DEBUG("Done!");
}
