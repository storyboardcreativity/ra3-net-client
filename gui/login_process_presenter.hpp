#pragma once

#include <thread>

#include "login_progress_form.hpp"
#include "login_form.hpp"
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
    }

    ~login_process_presenter()
    {
        _login_form.event_login_button_click -= MakeDelegate(this, &login_process_presenter::on_login_button_click);

        _conn_state_visitor.event_stage_description_changed -=
            MakeDelegate(&_login_process_form, &login_progress_form::change_description_text);
        _conn_state_visitor.event_percent_changed -=
            MakeDelegate(&_login_process_form, &login_progress_form::change_percent);
    }

    void on_login_button_click()
    {
        _login_process_form.show();
        _conn_thread = std::thread(process_connection, &_conn_state_visitor, "", "", "");
    }

    void run()
    {
        _login_form.show();
        exec();
    }

private:
    login_form _login_form;
    login_progress_form _login_process_form;
    
    connection_state_visitor _conn_state_visitor;
    std::thread _conn_thread;
};