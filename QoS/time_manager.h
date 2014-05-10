#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H 

#include "systemc.h"
#include "node.h"

typedef double tick_value_type;

class time_manager : public sc_module
{
private:
    tick_value_type m_tickValue;
    node* tm_node;

public:
    SC_HAS_PROCESS(time_manager);
    time_manager(sc_module_name mn, node* tm_node);

private:
    void tick();
};

#endif
