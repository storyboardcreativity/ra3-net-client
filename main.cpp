// std
#include <stdio.h>
#include <sstream>
#include <string>

// curlpp
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

// own headers
#include "printing_macros.h"
#include "ra3_constants.h"
#include "gamespy.h"
#include "fesl.h"
#include "peerchat.h"
#include "gpcm.h"

// ===

void process_http_connection(const char* server, const char* server_original, const char* path, const char* file_name)
{
    PROCESS_MESSAGE__DEBUG("GET -> http://%s/%s/%s", server, path, file_name);

    try
    {
        curlpp::Cleanup cleaner;
        curlpp::Easy request;

        std::list<std::string> header;
        if (server_original != nullptr)
            header.push_back(string_format("Host: %s", server_original));
        header.push_back("Content-Length: 0");
        header.push_back("Connection: Close");
        header.push_back("User-Agent: ProtoHttp 1.2/DS 6.3.5.0");
        header.push_back("Accept: */*");

        request.setOpt(new curlpp::options::HttpHeader(header));

        request.setOpt(new curlpp::options::Url(string_format("http://%s/%s/%s", server, path, file_name)));
        //request.setOpt(new curlpp::options::Verbose(true));

        request.perform();
    }
    catch (curlpp::LogicError &e)
    {
        std::cout << e.what() << std::endl;
    }
    catch (curlpp::RuntimeError &e)
    {
        std::cout << e.what() << std::endl;
    }
}

void process_download_step()
{
    PROCESS_MESSAGE__DEBUG("Processing download step [1]");

    process_http_connection(
        RA3_STRING_SERVER_DOWNLOADS,
        RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
        RA3_STRING_LIVESUITE_PATH,
        string_format("%s.patchinfo", RA3_STRING_PATCHINFO).c_str());
    
    process_http_connection(
        RA3_STRING_SERVER_DOWNLOADS,
        RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
        RA3_STRING_LIVESUITE_PATH,
        string_format("maps-%s.txt", RA3_STRING_MAPS_COUNT).c_str());

    process_http_connection(
        RA3_STRING_SERVER_DOWNLOADS,
        RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
        RA3_STRING_LIVESUITE_PATH,
        "config.txt");

    process_http_connection(
        RA3_STRING_SERVER_DOWNLOADS,
        RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
        RA3_STRING_LIVESUITE_PATH,
        string_format("MOTD-%s.txt", RA3_STRING_LOCALE).c_str());

    process_http_connection(
        RA3_STRING_SERVER_DOWNLOADS,
        RA3_STRING_SERVER_DOWNLOADS_ORIGINAL,
        RA3_STRING_LIVESUITE_PATH,
        "drivers.txt");
}

int main()
{
    PROCESS_MESSAGE__OK("Red Alert 3 network client!");

    process_download_step();

    process_gamespy_avail_ping();

    init_fesl_secure_connection();

    process_gpcm_connection();

    process_peerchat_connection();
}
