#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/panel.hpp>
#include <nana/gui/widgets/label.hpp>

#include "../fastdelegate/event.h"

using namespace nana;

class ra3online_join_game_widget : public panel<false>
{
public:
    ra3online_join_game_widget(window wd) : panel<false>(wd)
    {
        _place.div("<><weight=80% vertical <><weight=70% vertical <vertical gap=10 __label_not_implemented_yet>><>><>");

        __label_not_implemented_yet.format(true);
        __label_not_implemented_yet.caption("<center bold color=0xff0000>Not implemented yet.");
        __label_not_implemented_yet.bgcolor(color_rgb(0x111111));
        _place["__label_not_implemented_yet"] << __label_not_implemented_yet;

        _place.collocate();
    }

    // ========================== Events ==========================

private:
    place _place{*this};

    label __label_not_implemented_yet{*this};
};
