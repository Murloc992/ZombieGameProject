#pragma once
#include "GUIEvent.h"

class gui_event_listener
{
public:
    virtual ~gui_event_listener() {}
    virtual bool on_event(const gui_event & e)=0;
};
