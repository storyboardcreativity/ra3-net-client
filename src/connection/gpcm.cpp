/*

GPCM - GameSpy Connection Manager

*/

// std
#include <fcntl.h>
#include <memory.h>
#include <signal.h>

// OS-specific
#ifdef _MSC_VER

#include "unistd_windows.h"
#include <WinSock2.h>

#define poll WSAPoll

#elif __linux__

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#else

#error Unknown OS!

#endif

// own headers
#include "../printing_macros.h"
#include "../ra3_constants.h"
#include "../printing_tools.h"
#include "../client_info.hpp"
#include "../ra3_engine_tools/ra3_engine_tools.h"

class gpcm_connection
{
public:
    gpcm_connection(ra3_client_info& client_info)
        : _client_info(client_info) {}

    bool init()
    {
        // Create a socket
        if ((_socket_fd__gpcm = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) == -1)
            return false;

        // Make it non-blocking
#ifdef _MSC_VER
		u_long mode = 1;  // 1 to enable non-blocking socket
		ioctlsocket(_socket_fd__gpcm, FIONBIO, &mode);
#elif __linux__
		int flags = fcntl(_socket_fd__gpcm, F_GETFL, 0);
		fcntl(_socket_fd__gpcm, F_SETFL, flags | O_NONBLOCK);
#else
#error Unknown OS!
#endif

        // Init address structure
        memset((char *)&_gpcm_addr, 0, sizeof(_gpcm_addr));
        _gpcm_addr.sin_family = AF_INET;
        _gpcm_addr.sin_port = htons(RA3_STRING_SERVER_GPCM_PORT);
        _gpcm_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_GPCM);
        if (_gpcm_addr.sin_addr.s_addr == -1)
        {
            PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_GPCM);
            auto pHostent = gethostbyname(RA3_STRING_SERVER_GPCM);
            if (!pHostent)
            {
                PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_GPCM);
                return false;
            }
            PROCESS_MESSAGE__OK("Alternative way worked!");
            _gpcm_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
        }

        // Connect to server
        if (connect(_socket_fd__gpcm, (sockaddr *)&_gpcm_addr, sizeof(_gpcm_addr)) == -1)
        {
            if (errno == EINPROGRESS)
            {
                PROCESS_MESSAGE__WARNING("connect(...) operation is now in progress. Waiting for connection...");

                pollfd pfd;
                memset(&pfd, 0x00, sizeof(pfd));
                pfd.fd = _socket_fd__gpcm;
                pfd.events = POLLIN;
                poll(&pfd, 1, -1);
            }
            else
            {
                PROCESS_MESSAGE__ERROR("Could not connect! Error msg: %s (%d)", strerror(errno), errno);

#ifdef _MSC_VER
				closesocket(_socket_fd__gpcm);
#elif __linux__
				close(_socket_fd__gpcm);
#else
#error Unknown OS!
#endif

                return false;
            }
        }

        PROCESS_MESSAGE__OK("Connected!");

        return true;
    }

    void send_ka_final()
    {
        auto string = string_format("\\ka\\\\final\\");
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!

        // Send request
        send_gpcm_buffer((uint8_t *)string.c_str(), length);

        // Receive response
        uint8_t buffer[2048];
        length = recv_gpcm_buffer(buffer, sizeof(buffer));
        buffer[length] = '\0';

        auto packet = std::string((char *)buffer);
        auto offsetA = packet.find("challenge\\") + 10;
        auto offsetB = packet.find("\\id");
        auto challenge = packet.substr(offsetA, offsetB - offsetA);
        _client_info.gpcm__initial_challenge__set(challenge);
        PROCESS_MESSAGE__INFO("Initial challenge => %s", challenge.c_str());

        PROCESS_MESSAGE__OK("Done!");
    }

    void send_login_challenge()
    {
        // FIXME: We use constant challenge because we don't have generating algorithm
        _client_info.gpcm__our_challenge__set("wN1sOMYBAX3pci0t1cbsxa7W8PvVbwnr");

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
                                    _client_info.gpcm__our_challenge__get().c_str(),
                                    _client_info.preauth__ticket__get().c_str(),
                                    get_gpcm_challenge_response(_client_info).c_str(),
                                    RA3_STRING_GAMENAME);
        int length = strlen(string.c_str()); // Important: WITHOUT null terminator!

        // Send request
        send_gpcm_buffer((uint8_t *)string.c_str(), length);

        pollfd pfd;
        memset(&pfd, 0x00, sizeof(pfd));
        pfd.fd = _socket_fd__gpcm;
        pfd.events = POLLIN;
        poll(&pfd, 1, -1);

        // Receive response
        uint8_t buffer[2048];
        length = recv_gpcm_buffer(buffer, sizeof(buffer));
        buffer[length] = '\0';

        PROCESS_MESSAGE__OK("Done!");
    }

private:
    int _socket_fd__gpcm;
    sockaddr_in _gpcm_addr;
    ra3_client_info& _client_info;

    void send_gpcm_buffer(uint8_t *buff, uint32_t size)
    {
        PROCESS_MESSAGE__DEBUG("Sending data:");
        PROCESS_BUFMESSAGE__DEBUG(buff, size);

        int code = send(_socket_fd__gpcm, (char*)buff, size, 0);
        if (code == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno);
    }

    uint32_t recv_gpcm_buffer(uint8_t *buff, uint32_t size)
    {
        auto len = recv(_socket_fd__gpcm, (char*)buff, size, 0);

        if (len == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno)
        else
        {
            PROCESS_MESSAGE__DEBUG("Received data:");
            PROCESS_BUFMESSAGE__DEBUG(buff, len);
        }

        return len;
    }
};

void process_gpcm_connection(ra3_client_info& client_info)
{
    gpcm_connection connection(client_info);

    connection.init();

    connection.send_ka_final();

    connection.send_login_challenge();

    PROCESS_MESSAGE__DEBUG("Done!");
}