// std
//#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <signal.h>
#include <stdint.h>

// OS-specific
#ifdef _MSC_VER

#include "unistd_windows.h"

#elif __linux__

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

#include "enctypex_decoder.h"
#include "master_server.h"

class ms_connection : public IMasterServerConnection
{
public:
    ms_connection(ra3_client_info& client_info)
        : _client_info(client_info)
    {
        memset(_validate, 0x00, sizeof(_validate));
        _validate[0] = 0x77;
        _validate[1] = 0x61;
        _validate[2] = 0x72;
        _validate[3] = 0x5F;
        _validate[4] = 0x32;
        _validate[5] = 0x41;
        _validate[6] = 0x5E;
        _validate[7] = 0x5B;
    }

    bool init()
    {
        // Create a socket
        if ((_socket_fd__ms = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
            return false;

        // Init address structure
        memset((char *)&_ms_addr, 0, sizeof(_ms_addr));
        _ms_addr.sin_family = AF_INET;
        _ms_addr.sin_port = htons(RA3_STRING_SERVER_MASTER_PORT);
        _ms_addr.sin_addr.s_addr = inet_addr(RA3_STRING_SERVER_MASTER);
        if (_ms_addr.sin_addr.s_addr == -1)
        {
            PROCESS_MESSAGE__WARNING("Could not resolve server %s! Attempting to use alternative way...", RA3_STRING_SERVER_MASTER);
            auto pHostent = gethostbyname(RA3_STRING_SERVER_MASTER);
            if (!pHostent)
            {
                PROCESS_MESSAGE__ERROR("Finally could not resolve server %s!", RA3_STRING_SERVER_MASTER);
                return false;
            }
            PROCESS_MESSAGE__OK("Alternative way worked!");
            _ms_addr.sin_addr.s_addr = **(in_addr_t **)pHostent->h_addr_list;
        }

        // Connect to server
        if (connect(_socket_fd__ms, (sockaddr *)&_ms_addr, sizeof(_ms_addr)))
        {
            PROCESS_MESSAGE__ERROR("Could not connect! Error msg: %s (%d)", strerror(errno), errno);

#ifdef _MSC_VER
			closesocket(_socket_fd__ms);
#elif __linux__
			close(_socket_fd__ms);
#else
#error Unknown OS!
#endif

            return false;
        }

        PROCESS_MESSAGE__OK("Connected!");

        return true;
    }

    void send_request(std::string gamename, std::string query, std::string tags)
    {
        uint32_t size_needed = 13 + gamename.size() * 2 + 2 + 8 + query.size() + 1 + tags.size() + 1;
        auto data = new uint8_t[size_needed];

        // size
        *(uint16_t*)&data[0] = htons(size_needed);

        // magic (const)
        data[2] = 0x00;
        data[3] = 0x01;
        data[4] = 0x03;

        // unk magic
        *(uint32_t*)&data[5] = 0x00010000;

        // unk magic
        *(uint32_t*)&data[size_needed - 4] = 0x04000000;

        // gamename (twice)
        memcpy(&data[9], gamename.c_str(), gamename.size() + 1);
        memcpy(&data[9 + gamename.size() + 1], gamename.c_str(), gamename.size() + 1);

        // key
        memcpy(&data[9 + gamename.size() * 2 + 2], _validate, 8);

        // query string
        memcpy(&data[9 + gamename.size() * 2 + 2 + 8], query.c_str(), query.size() + 1);

        // tags string
        memcpy(&data[9 + gamename.size() * 2 + 2 + 8 + query.size() + 1], tags.c_str(), tags.size() + 1);

        send_ms_buffer(data, size_needed);

        delete[] data;
    }

    void send_games_request(std::string group_id) override
    {
        send_request(RA3_STRING_GAMENAME,
            string_format("(groupid=%s) AND (gamemode != \'closedplaying\')", group_id.c_str()),
            "\\hostname\\gamemode\\hostname\\mapname\\gamemode\\vCRC\\iCRC\\cCRC\\pw\\obs\\rules\\pings\\numRPlyr\\maxRPlyr\\numObs\\mID\\mod\\modV\\name_");
        
        uint8_t buffer[2048];
        recv_ms_buffer(buffer, 2048);
    }
private:
    int _socket_fd__ms;
    sockaddr_in _ms_addr;
    ra3_client_info& _client_info;

    uint8_t _validate[8];

    void send_ms_buffer(uint8_t *buff, uint32_t size)
    {
        PROCESS_MESSAGE__DEBUG("Sending data:");
        PROCESS_BUFMESSAGE__DEBUG(buff, size);

        int code = send(_socket_fd__ms, (char*)buff, size, 0);
        if (code == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno);
    }

    uint32_t recv_ms_buffer(uint8_t *buff, uint32_t size)
    {
        auto len = recv(_socket_fd__ms, (char*)buff, size, 0);

        if (len == -1)
            PROCESS_MESSAGE__ERROR("Err: %s (%d)", strerror(errno), errno)
        else
        {
            PROCESS_MESSAGE__DEBUG("Received data:");
            PROCESS_BUFMESSAGE__DEBUG(buff, len);

            auto sz = enctypex_wrapper((uint8_t*)RA3_STRING_PEERCHAT_SECRET_KEY, _validate, buff, len);

            PROCESS_MESSAGE__DEBUG("Decrypted data:");
            PROCESS_BUFMESSAGE__DEBUG(buff, sz);

            len = sz;
        }

        return len;
    }
};

IMasterServerConnection *process_ms_connection(ra3_client_info &client_info)
{
    auto connection = new ms_connection(client_info);

    connection->init();

    connection->send_games_request("2166");

    PROCESS_MESSAGE__DEBUG("Done!");

    return connection;
}