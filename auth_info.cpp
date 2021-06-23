#include <cstdio>
#include <fstream>
#include <unistd.h>

#include "auth_info.hpp"

#define SAVED_AUTH_INFO_FILE_NAME "saved_auth_info.txt"

void save_auth_info(auth_info& info)
{
    if (!info.is_saving_enabled_get())
    {
        std::remove(SAVED_AUTH_INFO_FILE_NAME);
        return;
    }

    std::ofstream out(SAVED_AUTH_INFO_FILE_NAME);
    out << info.login_get() << "\n"
        << info.password_get() << "\n"
        << info.pseudonym_get();
    out.close();
}

void load_auth_info(auth_info& info)
{
    std::ifstream in(SAVED_AUTH_INFO_FILE_NAME);
    if (!in.is_open())
    {
        info.is_saving_enabled_set(false);
        return;
    }

    std::string login, password, pseudonym;
    in >> login;
    in >> password;
    in >> pseudonym;
    
    info.login_set(login);
    info.password_set(password);
    info.pseudonym_set(pseudonym);
    info.is_saving_enabled_set(true);
}