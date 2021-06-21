#include "printing_macros.h"

// gui
#include "gui/login_process_presenter.hpp"

// ===

int main()
{
    PROCESS_MESSAGE__OK("Red Alert 3 network client!");

    login_process_presenter presenter;
    presenter.run();

    return 0;
}
