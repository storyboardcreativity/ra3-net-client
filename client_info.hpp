#pragma once

#include <string>

class ra3_client_info
{
public:
    void preauth__ticket__set(std::string ticket)
    {
        _preauth_ticket = ticket;
    }
    std::string preauth__ticket__get()
    {
        return _preauth_ticket;
    }

    void preauth__challenge__set(std::string challenge)
    {
        _nuloginpersona_profile_id = challenge;
    }
    std::string preauth__challenge__get()
    {
        return _nuloginpersona_profile_id;
    }

    void gpcm__initial_challenge__set(std::string challenge)
    {
        _preauth_challenge = challenge;
    }
    std::string gpcm__initial_challenge__get()
    {
        return _preauth_challenge;
    }

    void gpcm__our_challenge__set(std::string challenge)
    {
        _gpcm_our_challenge = challenge;
    }
    std::string gpcm__our_challenge__get()
    {
        return _gpcm_our_challenge;
    }

    void nuloginpersona__profile_id__set(std::string profile_id)
    {
        _nuloginpersona_profile_id = profile_id;
    }
    std::string nuloginpersona__profile_id__get()
    {
        return _nuloginpersona_profile_id;
    }

private:
    std::string _preauth_ticket;
    std::string _preauth_challenge;
    std::string _gpcm_initial_challenge;
    std::string _gpcm_our_challenge;
    std::string _nuloginpersona_profile_id;
};
