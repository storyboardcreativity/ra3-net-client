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
#include "DirtySDKEAWebKit/protossl.h"
#include "DirtySDKEAWebKit/dirtynet.h"

void init_fesl_secure_connection()
{
    // Initialize API callbacks
    init_socket_api_callbacks();

    // Initialize EA socket
    SocketCreate(0);

    // Create ProtoSSL instance
    auto sslref = ProtoSSLCreate();

    // Reset ProtoSSL instance
    ProtoSSLReset(sslref);

    // Prepare
    ProtoSSLConnect(sslref, 0, RA3_STRING_SERVER_FESL, 0, RA3_STRING_SERVER_FESL_PORT);

    // Update (to connect)
    ProtoSSLUpdate(sslref);

    PROCESS_MESSAGE__DEBUG("ProtoSSLConnect done!");
}