#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H 

#include "systemc.h"

typedef double tick_value_type;

class time_manager : public sc_module
{
public:
    sc_port<recei
private:
    tick_value_type m_tickValue;

public:
    SC_HAS_PROCESS(time_manager);
    time_manager(sc_module_name mn, receiver & r);
    time_manager(sc_module_name mn, params..);


private:
    void tick();
};

#endif