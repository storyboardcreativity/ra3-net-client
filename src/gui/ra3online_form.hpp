#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/tabbar.hpp>

#include "login_progress_form.hpp"
#include "ra3online_chat_lobbies_widget.hpp"
#include "ra3online_join_game_widget.hpp"

#include "../fastdelegate/event.h"

using namespace nana;

class ra3online_form : public form
{
public:
    ra3online_form()
    {
        caption(L"Red Alert 3 Online");

        this->div("vertical <weight=40 _tabbar> <tab_frame>");

        _tabbar.create(*this);
        this->operator[]("_tabbar") << _tabbar;

        this->operator[]("tab_frame").fasten(_chat_lobbies_control);
        this->operator[]("tab_frame").fasten(_join_game_control);

        _tabbar.append("Chat Lobbies", _chat_lobbies_control);
        //_tabbar.append("Play Game", _chat_lobbies);
        _tabbar.append("Join Game", _join_game_control);
        //_tabbar.append("Host Game", _chat_lobbies);

        this->collocate();
    }

    ra3online_chat_lobbies_widget& chat_lobbies_control()
    {
        return _chat_lobbies_control;
    }

    // ========================== Events ==========================

    DECLARE_EVENT(void) event_login_button_click;

private:

    tabbar<std::string> _tabbar;
    ra3online_chat_lobbies_widget _chat_lobbies_control{*this};
    ra3online_join_game_widget _join_game_control{*this};
};
