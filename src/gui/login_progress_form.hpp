#pragma once

#include <nana/gui.hpp>
#include <nana/gui/place.hpp>
#include <nana/gui/widgets/progress.hpp>
#include <nana/gui/widgets/label.hpp>

using namespace nana;

class login_progress_form : public form
{
public:
    login_progress_form()
    {
        caption(L"RA3 Client Login");

        this->div("<><weight=80% vertical <><weight=70% vertical <vertical gap=10 _label_status_text>  <weight=25 gap=10 _progress>><>><>");

        // Status text
        _label_status_text.create(*this);
        _label_status_text.caption("IM A STATUS BEACH");
        this->operator[]("_label_status_text") << _label_status_text;

        // ProgressBar
        _progress.create(*this);
        this->operator[]("_progress") << _progress;

        this->collocate();
    }

    void change_description_text(std::string description)
    {
        _label_status_text.caption(description);
    }

    void change_percent(int percent)
    {
        _progress.value(percent);
    }

private:

    place _place;

    progress _progress;
    label _label_status_text;
};
