#pragma once

#include <mutex>

#include "../fastdelegate/event.h"
#include "peerchat.hpp"

class connection_state_visitor
{
public:
    void change_percent(int percent)
    {
        _mutex.lock();
        TRIGGER_EVENT(event_percent_changed, percent);
        _mutex.unlock();
    }

    void change_stage_description(std::string description)
    {
        _mutex.lock();
        TRIGGER_EVENT(event_stage_description_changed, description);
        _mutex.unlock();
    }

    DECLARE_EVENT(void, int) event_percent_changed;
    DECLARE_EVENT(void, std::string) event_stage_description_changed;

    DECLARE_EVENT(void, IPeerchatConnection*) event_connection_established;

private:
    std::mutex _mutex;
};

void process_connection(connection_state_visitor* visitor, std::string login, std::string password, std::string id);
