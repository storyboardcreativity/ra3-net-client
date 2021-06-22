#pragma once

#include <thread>

#include "login_progress_form.hpp"
#include "login_form.hpp"
#include "ra3online_form.hpp"
#include "../connection/connection.hpp"

class login_process_presenter
{
public:
    login_process_presenter()
    {
        _login_form.event_login_button_click +=
            MakeDelegate(this, &login_process_presenter::on_login_button_click);

        _conn_state_visitor.event_percent_changed +=
            MakeDelegate(&_login_process_form, &login_progress_form::change_percent);
        _conn_state_visitor.event_stage_description_changed +=
            MakeDelegate(&_login_process_form, &login_progress_form::change_description_text);

        _ra3online_form.chat_lobbies_control().event_lobby_changed +=
            MakeDelegate(this, &login_process_presenter::on_chat_lobbies_control_lobby_select);
    }

    ~login_process_presenter()
    {
        _ra3online_form.chat_lobbies_control().event_lobby_changed -=
            MakeDelegate(this, &login_process_presenter::on_chat_lobbies_control_lobby_select);

        _conn_state_visitor.event_stage_description_changed -=
            MakeDelegate(&_login_process_form, &login_progress_form::change_description_text);
        _conn_state_visitor.event_percent_changed -=
            MakeDelegate(&_login_process_form, &login_progress_form::change_percent);
        
        _login_form.event_login_button_click -=
            MakeDelegate(this, &login_process_presenter::on_login_button_click);
    }

    void run()
    {
        //_login_form.show();
        _ra3online_form.show();

        exec();
    }

private:
    void on_login_button_click()
    {
        _login_process_form.show();
        _conn_thread = std::thread(process_connection, &_conn_state_visitor, "", "", "");
    }

    void on_chat_lobbies_control_lobby_select(std::string lobby_name)
    {
        std::vector<std::string> aaa;
        aaa.push_back(lobby_name);
        aaa.push_back("sadfasdf");
        aaa.push_back("asdfasdfasdfasd");
        aaa.push_back("asdfasd");
        aaa.push_back("asdfasdfwerrt423rt234");
        aaa.push_back("asdf323421241");

        _ra3online_form.chat_lobbies_control().set_player_list(aaa);
    }




    login_form _login_form;
    login_progress_form _login_process_form;
    
    connection_state_visitor _conn_state_visitor;
    std::thread _conn_thread;

    ra3online_form _ra3online_form;
};