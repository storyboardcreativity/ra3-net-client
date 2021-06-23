#pragma once

#include <string>
#include <mutex>

class ra3_client_info
{
public:
    void preauth__ticket__set(std::string ticket)
    {
        _mutex.lock();
        _preauth_ticket = ticket;
        _mutex.unlock();
    }
    std::string preauth__ticket__get()
    {
        _mutex.lock();
        auto ticket = _preauth_ticket;
        _mutex.unlock();
        return ticket;
    }

    void preauth__challenge__set(std::string challenge)
    {
        _mutex.lock();
        _nuloginpersona_profile_id = challenge;
        _mutex.unlock();
    }
    std::string preauth__challenge__get()
    {
        _mutex.lock();
        auto id = _nuloginpersona_profile_id;
        _mutex.unlock();
        return id;
    }

    void gpcm__initial_challenge__set(std::string challenge)
    {
        _mutex.lock();
        _preauth_challenge = challenge;
        _mutex.unlock();
    }
    std::string gpcm__initial_challenge__get()
    {
        _mutex.lock();
        auto challenge = _preauth_challenge;
        _mutex.unlock();
        return challenge;
    }

    void gpcm__our_challenge__set(std::string challenge)
    {
        _mutex.lock();
        _gpcm_our_challenge = challenge;
        _mutex.unlock();
    }
    std::string gpcm__our_challenge__get()
    {
        _mutex.lock();
        auto challenge = _gpcm_our_challenge;
        _mutex.unlock();
        return challenge;
    }

    void nuloginpersona__profile_id__set(std::string profile_id)
    {
        _mutex.lock();
        _nuloginpersona_profile_id = profile_id;
        _mutex.unlock();
    }
    std::string nuloginpersona__profile_id__get()
    {
        _mutex.lock();
        auto id = _nuloginpersona_profile_id;
        _mutex.unlock();
        return id;
    }

private:
    std::string _preauth_ticket;
    std::string _preauth_challenge;
    std::string _gpcm_initial_challenge;
    std::string _gpcm_our_challenge;
    std::string _nuloginpersona_profile_id;

    std::mutex _mutex;
};
