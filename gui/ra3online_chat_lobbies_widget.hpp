#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/textbox.hpp>
#include <nana/gui/widgets/combox.hpp>

#include "../fastdelegate/event.h"

using namespace nana;

class ra3online_chat_lobbies_widget : public panel<false>
{
public:
    ra3online_chat_lobbies_widget(window wd) : panel<false>(wd)
    {
        _place.div("<vertical <weight=25 _combobox_lobbies><_listbox_player_list>><vertical <_textbox_chat><weight=25 <horizontal <weight=80% _textbox_message><_button_send_message>>>>");

        _listbox_player_list.append_header("Player List");

        _button_send_message.caption("Send");

        _combobox_lobbies.push_back("lobby1");
        _combobox_lobbies.push_back("lobby2");
        _combobox_lobbies.push_back("lobby3");
        _combobox_lobbies.option(0);
        _combobox_lobbies.events().selected([this](const nana::arg_combox& acmb)
        {
            TRIGGER_EVENT(event_lobby_changed, acmb.widget.caption());
        });

        _textbox_message.tip_string("Message:");

        _place["_combobox_lobbies"] << _combobox_lobbies;
        _place["_listbox_player_list"] << _listbox_player_list;
        _place["_textbox_chat"] << _textbox_chat;
        _place["_textbox_message"] << _textbox_message;
        _place["_button_send_message"] << _button_send_message;

        _place.collocate();
    }

    void set_player_list(std::vector<std::string>& players)
    {
        _listbox_player_list.clear();
        for (auto it = players.begin(); it != players.end(); ++it)
            _listbox_player_list.at(0).append(*it);
    }

    // ========================== Events ==========================

    DECLARE_EVENT(void, std::string lobby_name) event_lobby_changed;

private:
    place _place{*this};

    listbox _listbox_player_list{*this};
    textbox _textbox_chat{*this};
    textbox _textbox_message{*this};
    combox _combobox_lobbies{*this};
    button _button_send_message{*this};
};
