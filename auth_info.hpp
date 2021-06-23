#pragma once

#include <string>
#include <mutex>

class auth_info
{
public:
    std::string login_get()
    {
        _mutex.lock();
        auto login = _login;
        _mutex.unlock();
        return login;
    }
    void login_set(std::string login)
    {
        _mutex.lock();
        _login = login;
        _mutex.unlock();
    }

    std::string password_get()
    {
        _mutex.lock();
        auto password = _password;
        _mutex.unlock();
        return password;
    }
    void password_set(std::string password)
    {
        _mutex.lock();
        _password = password;
        _mutex.unlock();
    }

    std::string pseudonym_get()
    {
        _mutex.lock();
        auto pseudonym = _pseudonym;
        _mutex.unlock();
        return pseudonym;
    }
    void pseudonym_set(std::string pseudonym)
    {
        _mutex.lock();
        _pseudonym = pseudonym;
        _mutex.unlock();
    }

    bool is_saving_enabled_get()
    {
        _mutex.lock();
        auto is_saving_enabled = _is_saving_enabled;
        _mutex.unlock();
        return is_saving_enabled;
    }
    void is_saving_enabled_set(bool is_saving_enabled)
    {
        _mutex.lock();
        _is_saving_enabled = is_saving_enabled;
        _mutex.unlock();
    }

private:
    std::mutex _mutex;

    std::string _login;
    std::string _password;
    std::string _pseudonym;
    bool _is_saving_enabled;
};

void save_auth_info(auth_info& info);
void load_auth_info(auth_info& info);