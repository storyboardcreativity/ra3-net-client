#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>

#include "login_progress_form.hpp"

#include "../fastdelegate/event.h"

using namespace nana;

class login_form : public form
{
public:
    login_form()
    {
        caption(L"RA3 Client Login");

        this->div("<><weight=80% vertical <><weight=70% vertical <vertical gap=10 textboxs arrange=[25,25,25]>  <weight=25 gap=10 buttons>><>><>");

        // "Login" textbox
        _textbox_login.create(*this);
        _textbox_login.tip_string("Login:");
        this->operator[]("textboxs") << _textbox_login;

        // "Password" textbox
        _textbox_password.create(*this);
        _textbox_password.tip_string("Password:");
        this->operator[]("textboxs") << _textbox_password;

        // "User ID" textbox
        _textbox_userid.create(*this);
        _textbox_userid.tip_string("User ID:");
        this->operator[]("textboxs") << _textbox_userid;

        // "Login" button
        _button_login.create(*this);
        _button_login.caption("Login");
        _button_login.events().click([this](const nana::arg_click &ei) { __button_handler__login(ei); });
        this->operator[]("buttons") << _button_login;

        // "Cancel" button
        _button_cancel.create(*this);
        _button_cancel.caption("Cancel");
        _button_cancel.events().click([this](const nana::arg_click &ei) { __button_handler__cancel(ei); });
        this->operator[]("buttons") << _button_cancel;

        this->collocate();
    }

    // ========================== Events ==========================

    DECLARE_EVENT(void) event_login_button_click;

private:
    void __button_handler__login(const nana::arg_click &ei)
    {
        TRIGGER_EVENT(event_login_button_click);
    }

    void __button_handler__cancel(const nana::arg_click &ei)
    {
        // Close form
        this->close();
    }

    place _place;
    textbox _textbox_login;
    textbox _textbox_password;
    textbox _textbox_userid;
    button _button_login;
    button _button_cancel;
};
