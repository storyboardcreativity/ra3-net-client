#pragma once

#include <string>

#include "client_info.hpp"

bool init_fesl_secure_connection(ra3_client_info& client_info, std::string login, std::string password, std::string id);
