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
        
        _conn_state_visitor.event_connection_established +=
            MakeDelegate(this, &login_process_presenter::on_connection_established);
    }

    ~login_process_presenter()
    {
        _conn_state_visitor.event_connection_established -=
            MakeDelegate(this, &login_process_presenter::on_connection_established);

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
        _login_form.show();
        exec();
    }

private:
    void on_login_button_click()
    {
        _login_form.enabled(false);
        _login_process_form.show();
        _conn_thread = std::thread(process_connection, &_conn_state_visitor, "", "", "");
    }

    void on_chat_lobbies_control_lobby_select(std::string lobby_name)
    {
        auto player_list = _peerchat_connection->send_JOIN("#GPG!2166");
        _ra3online_form.chat_lobbies_control().set_player_list(player_list);
    }

    void on_connection_established(IPeerchatConnection* connection)
    {
        _peerchat_connection = connection;

        _login_form.close();
        _login_process_form.close();
        _ra3online_form.show();
    }

    login_form _login_form;
    login_progress_form _login_process_form;
    
    connection_state_visitor _conn_state_visitor;
    std::thread _conn_thread;

    ra3online_form _ra3online_form;

    IPeerchatConnection* _peerchat_connection{nullptr};
};