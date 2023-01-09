#pragma once

#include <string>
#include <vector>

#include "../client_info.hpp"

class IPeerchatConnection
{
public:
    virtual ~IPeerchatConnection() {}

    virtual void send_CRYPT() = 0;
    virtual void send_USRIP() = 0;
    virtual void send_USER(std::string id) = 0;
    virtual void send_CDKEY() = 0;
    virtual std::vector<std::string> send_JOIN(std::string channel_name) = 0;
    virtual void send_QUIT() = 0;
};

IPeerchatConnection *process_peerchat_connection(ra3_client_info &client_info, std::string id);