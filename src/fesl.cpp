// std
#include <stdio.h>
#include <sstream>
#include <string>
#include <string.h>
#include <chrono>
#include <thread>

// OS-specific
#ifdef _MSC_VER

#elif __linux__

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

#else

#error Unknown OS!

#endif

// own headers
#include "printing_macros.h"
#include "ra3_constants.h"
#include "client_info.hpp"

//
#include "DirtySDKEAWebKit/socketapicallbacks.h"
#include "DirtySDKEAWebKit/protossl.h"
#include "DirtySDKEAWebKit/dirtynet.h"

void fesl_send(ProtoSSLRefT *ref, uint32_t type, uint32_t magic, const char *str)
{
    // === HEADER ===
    // [4 bytes (Big Endian)] - header (packet type)
    // [4 bytes (Big Endian)] - ??? (unknown magic)
    // [4 bytes (Big Endian)] - full packet size, including this header
    // ==== BODY ====
    // ... - char string with null terminator ('\0')
    // ==============

    uint32_t size = 4 + 4 + 4 + strlen(str) + 1;
    auto data = new uint8_t[size];
    (*(uint32_t *)&data[0]) = htonl(type);
    (*(uint32_t *)&data[4]) = htonl(magic);
    (*(uint32_t *)&data[8]) = htonl(size);
    memcpy(&data[12], str, size - 12);

    PROCESS_MESSAGE__DEBUG("Sending data:");
    PROCESS_BUFMESSAGE__DEBUG(data, size);

    ProtoSSLSend(ref, (const char *)data, size);

    delete[] data;
}

std::string fesl_recv(ProtoSSLRefT *ref)
{
    const int max_size = 2048;
    uint8_t data[max_size];

    int32_t sz = ProtoSSLRecv(ref, (char *)data, max_size);
    
    PROCESS_MESSAGE__DEBUG("Received data:");
    PROCESS_BUFMESSAGE__DEBUG(data, sz);

    return std::string((char*)&data[12]);
}

void send_client_hello(ProtoSSLRefT *ref)
{
    PROCESS_MESSAGE__INFO("Sending \"HELLO\" to server...");

    // Send "HELLO" request
    auto hello_string = "TXN=Hello\n"
                        "clientString=cncra3-pc\n"
                        "sku=15299\n"
                        "locale=ru\n"
                        "clientPlatform=PC\n"
                        "clientVersion=1.0\n"
                        "SDKVersion=4.3.4.0.0\n"
                        "protocolVersion=2.0\n"
                        "fragmentSize=8096\n"
                        "clientType=\n"; // for some reason "clientType" field is empty
    fesl_send(ref, 'fsys', 0xC0000001, hello_string);

    // Wait for response (standard list with parameters)
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server responded to our \"HELLO\" message with this text:\n%s", response.c_str());

    // Wait for memcheck response
    response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us memcheck data:\n%s", response.c_str());

    // Imitate memcheck stage
    auto memcheck_string = "TXN=MemCheck\n"
                           "result=\n"; // for some reason "result" field is empty
    fesl_send(ref, 'fsys', 0x00000080, memcheck_string);
}

void send_login_info(ProtoSSLRefT *ref, std::string email, std::string password)
{
    PROCESS_MESSAGE__INFO("Logging in...");

    // Send login request
    auto login_string = string_format("TXN=NuLogin\n"
                                      "returnEncryptedInfo=1\n"
                                      "nuid=%s\n"                // << here is our username
                                      "password=%s\n"         // << here is out password
                                      "macAddr=$cccccccccccc\n",  // for some reason this field is filled with 'c');
                                      email.c_str(), password.c_str());
    fesl_send(ref, 'acct', 0x020000C0, login_string.c_str());

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us login response:\n%s", response.c_str());
}

void get_personas(ProtoSSLRefT *ref)
{
    PROCESS_MESSAGE__INFO("Getting personas list...");

    // Send request
    auto personas_request_string = "TXN=NuGetPersonas\n"
                                   "namespace=\n"; // for some reason "namespace" field is empty
    fesl_send(ref, 'acct', 0x030000C0, personas_request_string);

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us personas list:\n%s", response.c_str());
}

void add_persona(ProtoSSLRefT *ref, std::string account_id)
{
    PROCESS_MESSAGE__INFO("Adding persona to account...");

    // Send request
    auto add_persona_request_string = string_format("TXN=NuAddPersona\n"
                                                    "name=%s\n", account_id.c_str());
    fesl_send(ref, 'acct', 0x040000C0, add_persona_request_string.c_str());

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us response:\n%s", response.c_str());
}

void login_persona(ra3_client_info& client_info, ProtoSSLRefT *ref, std::string account_id)
{
    PROCESS_MESSAGE__INFO("Processing login to persona...");

    // Send request
    auto login_persona_request_string = string_format("TXN=NuLoginPersona\n"
                                                      "name=%s\n", account_id.c_str());
    fesl_send(ref, 'acct', 0x060000C0, login_persona_request_string.c_str());

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us response:\n%s", response.c_str());

    // Extract profileId from response
    auto profileId = response.substr(response.find("profileId=") + 10);
    PROCESS_MESSAGE__OK("Got profileId: %s", profileId.c_str());
    client_info.nuloginpersona__profile_id__set(profileId);
}

void get_telemetry_token(ProtoSSLRefT *ref)
{
    PROCESS_MESSAGE__INFO("Requesting telemetry token...");

    // Send request
    auto telemetry_request_string = "TXN=GetTelemetryToken\n";
    fesl_send(ref, 'acct', 0x070000C0, telemetry_request_string);

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us response:\n%s", response.c_str());
}

void gamespy_pre_auth(ra3_client_info& client_info, ProtoSSLRefT *ref)
{
    PROCESS_MESSAGE__INFO("Sending GameSpy pre-auth request...");

    // Send request
    auto request_string = "TXN=GameSpyPreAuth\n";
    fesl_send(ref, 'acct', 0x080000C0, request_string);

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us response:\n%s", response.c_str());

    // Extract ticket from response
    auto ticket = response.substr(response.find("ticket=") + 7);
    PROCESS_MESSAGE__OK("Got pre-auth ticket: %s", ticket.c_str());
    client_info.preauth__ticket__set(ticket);

    // Extract challenge from response
    auto offsetA = response.find("challenge=") + 10;
    auto offsetB = response.find("\nticket=");
    auto challenge = response.substr(offsetA, offsetB - offsetA);
    PROCESS_MESSAGE__OK("Got pre-auth challenge: %s", challenge.c_str());
    client_info.preauth__challenge__set(challenge);
}

void process_ping(ProtoSSLRefT *ref)
{
    PROCESS_MESSAGE__INFO("Answering ping...");

    // Wait for response from server
    auto response = fesl_recv(ref);
    PROCESS_MESSAGE__OK("Server sent us:\n%s", response.c_str());

    if (response == "TXN=Ping")
    {
        // Send request
        auto request_string = "TXN=Ping\n";
        fesl_send(ref, 'fsys', 0x090000C0, request_string);
    }
}

void init_fesl_secure_connection(ra3_client_info& client_info, std::string login, std::string password, std::string id)
{
    // Initialize API callbacks
    init_socket_api_callbacks();

    // Initialize EA socket
    SocketCreate(0);

    // Create ProtoSSL instance
    auto sslref = ProtoSSLCreate();

    // Reset ProtoSSL instance
    ProtoSSLReset(sslref);

    // Allow any valid certificate (don't check if cert is issued to using host)
    ProtoSSLControl(sslref, 'ncrt', 1, 0, nullptr);

    // Prepare connection
    ProtoSSLConnect(sslref, 1, RA3_STRING_SERVER_FESL_ORIGINAL, 0, RA3_STRING_SERVER_FESL_PORT);

    // Update (we connect here and make secure handshake)
    ProtoSSLUpdate(sslref);

    // Process client hello to RA3 server
    send_client_hello(sslref);

    // Try to login
    send_login_info(sslref, login, password);

    // Get names list of our account
    get_personas(sslref);

    // Add our persona to account
    add_persona(sslref, id);

    // Login our persona
    login_persona(client_info, sslref, id);

    // Get telemetry token
    get_telemetry_token(sslref);

    // Request GameSpy pre-auth
    gamespy_pre_auth(client_info, sslref);

    //
    process_ping(sslref);

    PROCESS_MESSAGE__DEBUG("Done!");
}