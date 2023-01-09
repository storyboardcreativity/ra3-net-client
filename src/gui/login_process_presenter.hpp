#pragma once

#include <thread>
#include <map>
#include <algorithm>

#include "login_progress_form.hpp"
#include "login_form.hpp"
#include "ra3online_form.hpp"
#include "../connection/connection.hpp"
#include "../auth_info.hpp"

class login_process_presenter
{
public:
    login_process_presenter()
    {
        _prepare_lobbies();

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

		_conn_state_visitor.event_connection_failed +=
			MakeDelegate(this, &login_process_presenter::on_connection_failed);
    }

    ~login_process_presenter()
    {
		_conn_state_visitor.event_connection_failed -=
			MakeDelegate(this, &login_process_presenter::on_connection_failed);

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
        load_auth_info(_auth_info);
        _login_form.checkbox_set(_auth_info.is_saving_enabled_get());
        if (_auth_info.is_saving_enabled_get())
        {
            _login_form.login_set(_auth_info.login_get());
            _login_form.password_set(_auth_info.password_get());
            _login_form.pseudonym_set(_auth_info.pseudonym_get());
        }

        _login_form.show();
        exec();
    }

private:
    void on_login_button_click()
    {
        _auth_info.is_saving_enabled_set(_login_form.checkbox_get());
        _auth_info.login_set(_login_form.login_get());
        _auth_info.password_set(_login_form.password_get());
        _auth_info.pseudonym_set(_login_form.pseudonym_get());
        save_auth_info(_auth_info);

        //_login_form.enabled(false);
        _login_process_form.show();
        _conn_thread = std::thread(process_connection, &_conn_state_visitor,
            _login_form.login_get(),
            _login_form.password_get(),
            _login_form.pseudonym_get());
    }

    void on_chat_lobbies_control_lobby_select(std::string lobby_name)
    {
        auto player_list = _peerchat_connection->send_JOIN(_lobbies[lobby_name]);
        _ra3online_form.chat_lobbies_control().set_player_list(player_list);
    }

    void on_connection_established(IPeerchatConnection* connection)
    {
        _peerchat_connection = connection;

        _login_form.close();
        _login_process_form.close();
        _ra3online_form.show();
    }

	void on_connection_failed(std::string reason)
	{
		_login_process_form.close();
	}

    void _prepare_lobbies()
    {
        _lobbies["BattleCast"] = "#GPG!2175";
        _lobbies["Дополнительные карты 01"] = "#GPG!2194";
        _lobbies["Дополнительные карты 02"] = "#GPG!2195";
        _lobbies["ИСПАНСКИЙ 01"] = "#GPG!2357";
        _lobbies["Кланы 01"] = "#GPG!2184";
        _lobbies["Кланы 02"] = "#GPG!2186";
        _lobbies["Корейский 01"] = "#GPG!2171";
        _lobbies["Любители 01"] = "#GPG!2166";
        _lobbies["Любители 02"] = "#GPG!2167";
        // ...

        std::vector<std::string> keys;
        std::transform(_lobbies.begin(), _lobbies.end(), std::back_inserter(keys),
            [](const std::map<std::string, std::string>::value_type &pair)
            {
                return pair.first;
            });

        _ra3online_form.chat_lobbies_control().set_lobby_list(keys);
    }

    std::map<std::string, std::string> _lobbies;

    login_form _login_form;
    login_progress_form _login_process_form;
    
    connection_state_visitor _conn_state_visitor;
    std::thread _conn_thread;

    ra3online_form _ra3online_form;

    IPeerchatConnection* _peerchat_connection{nullptr};

    auth_info _auth_info;
};