#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/checkbox.hpp>

#include "login_progress_form.hpp"

#include "../fastdelegate/event.h"

using namespace nana;

class login_form : public form
{
public:
    login_form()
    {
        caption(L"RA3 Client Login");
        size(nana::size{320, 240});

        this->div("<><weight=80% vertical <><weight=70% vertical <vertical gap=10 textboxs arrange=[25,25,25]> <weight=30 gap=10 _checkbox_save> <weight=25 gap=10 buttons>><>><>");

        // Checkbox
        this->operator[]("_checkbox_save") << _checkbox_save;
        _checkbox_save.caption("Remember me");

        // "Login" textbox
        _textbox_login.tip_string("Login:");
        this->operator[]("textboxs") << _textbox_login;

        // "Password" textbox
        _textbox_password.tip_string("Password:");
        this->operator[]("textboxs") << _textbox_password;

        // "User ID" textbox
        _textbox_userid.tip_string("User ID:");
        this->operator[]("textboxs") << _textbox_userid;

        // "Login" button
        _button_login.caption("Login");
        _button_login.events().click([this](const nana::arg_click &ei) { __button_handler__login(ei); });
        this->operator[]("buttons") << _button_login;

        // "Cancel" button
        _button_cancel.caption("Cancel");
        _button_cancel.events().click([this](const nana::arg_click &ei) { __button_handler__cancel(ei); });
        this->operator[]("buttons") << _button_cancel;

        this->collocate();
    }

    std::string login_get()
    {
        return _textbox_login.caption();
    }
    void login_set(std::string login)
    {
        _textbox_login.caption(login);
    }

    std::string password_get()
    {
        return _textbox_password.caption();
    }
    void password_set(std::string password)
    {
        _textbox_password.caption(password);
    }

    std::string pseudonym_get()
    {
        return _textbox_userid.caption();
    }
    void pseudonym_set(std::string pseudonym)
    {
        _textbox_userid.caption(pseudonym);
    }

    void checkbox_set(bool flag)
    {
        _checkbox_save.check(flag);
    }
    bool checkbox_get()
    {
        return _checkbox_save.checked();
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

    place _place{*this};
    textbox _textbox_login{*this};
    textbox _textbox_password{*this};
    textbox _textbox_userid{*this};
    button _button_login{*this};
    button _button_cancel{*this};
    checkbox _checkbox_save{*this};
};
