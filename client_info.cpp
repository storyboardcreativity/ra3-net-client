#include "client_info.h"

std::string g_preauth_ticket;

void preauth__ticket__set(std::string ticket)
{
    g_preauth_ticket = ticket;
}
std::string preauth__ticket__get()
{
    return g_preauth_ticket;
}

std::string g_preauth_challenge;

void preauth__challenge__set(std::string challenge)
{
    g_preauth_challenge = challenge;
}
std::string preauth__challenge__get()
{
    return g_preauth_challenge;
}

std::string g_gpcm_initial_challenge;

void gpcm__initial_challenge__set(std::string challenge)
{
    g_gpcm_initial_challenge = challenge;
}
std::string gpcm__initial_challenge__get()
{
    return g_gpcm_initial_challenge;
}

std::string g_gpcm_our_challenge;

void gpcm__our_challenge__set(std::string challenge)
{
    g_gpcm_our_challenge = challenge;
}
std::string gpcm__our_challenge__get()
{
    return g_gpcm_our_challenge;
}

std::string g_nuloginpersona_profile_id;

void nuloginpersona__profile_id__set(std::string profile_id)
{
    g_nuloginpersona_profile_id = profile_id;
}
std::string nuloginpersona__profile_id__get()
{
    return g_nuloginpersona_profile_id;
}
