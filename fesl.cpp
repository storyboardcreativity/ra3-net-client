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

//
#include "DirtySDKEAWebKit/socketapicallbacks.h"

void init_fesl_secure_connection()
{
    init_socket_api_callbacks();
}