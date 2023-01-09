#include <string>

// curlpp
#define CURL_STATICLIB
#define CURLPP_STATICLIB
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Exception.hpp>

// own headers
#include "../printing_macros.h"
#include "../ra3_constants.h"
#include "../gamespy.h"
#include "../fesl.h"
#include "peerchat.hpp"
#include "gpcm.h"
#include "master_server.h"

#include "connection.hpp"

void process_http_connection(const char *server, const char *server_original, const char *path, const char *file_name)
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

void process_connection(connection_state_visitor *visitor, std::string login, std::string password, std::string id)
{
    ra3_client_info client_info;

    if (visitor)
    {
        visitor->change_percent(0);
        visitor->change_stage_description("Starting connection process...");
    }

    if (visitor)
    {
        visitor->change_percent(10);
        visitor->change_stage_description("Processing download step...");
    }

    process_download_step();

    if (visitor)
    {
        visitor->change_percent(20);
        visitor->change_stage_description("Processing GameSpy availability ping...");
    }

    process_gamespy_avail_ping();

    if (visitor)
    {
        visitor->change_percent(40);
        visitor->change_stage_description("Processing FESL secure connection...");
    }

	if (!init_fesl_secure_connection(client_info, login, password, id))
	{
		visitor->change_percent(0);
		visitor->change_stage_description("Failed!");
		TRIGGER_EVENT(visitor->event_connection_failed, "Could not initialize FESL secure connection!");
		return;
	}

    if (visitor)
    {
        visitor->change_percent(60);
        visitor->change_stage_description("Processing GPCM connection...");
    }

    process_gpcm_connection(client_info);

    if (visitor)
    {
        visitor->change_percent(80);
        visitor->change_stage_description("Processing peerchat connection...");
    }

    auto peerchat_connection = process_peerchat_connection(client_info, id);

    if (visitor)
    {
        visitor->change_percent(80);
        visitor->change_stage_description("Processing master-server connection...");
    }

    auto ms_connection = process_ms_connection(client_info);

    if (visitor)
    {
        visitor->change_percent(100);
        visitor->change_stage_description("Done!");
        TRIGGER_EVENT(visitor->event_connection_established, peerchat_connection);
    }
}