#pragma once

#include <string>
#include <vector>

#include "../client_info.hpp"

class IMasterServerConnection
{
public:
    virtual ~IMasterServerConnection() {}

    virtual void send_games_request(std::string channel_name) = 0;
};

IMasterServerConnection *process_ms_connection(ra3_client_info &client_info);
