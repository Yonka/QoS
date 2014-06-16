#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H 

#include "systemc.h"
#include "node.h"
#include "defs.h"

class time_manager : public sc_module
{
private:
    int m_tickValue;
    sc_time m_t_tc;
    node* tm_node;

    void tick();

public:
    SC_HAS_PROCESS(time_manager);
    time_manager(sc_module_name mn, node* tm_node);
};
#endif
