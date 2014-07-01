#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H 

#include "systemc.h"
#include "node.h"
#include "defs.h"

class time_manager : public sc_module
{
private:
    int m_time_code_value;
    sc_time m_tick_period;
    node* m_time_master_node;

public:
    SC_HAS_PROCESS(time_manager);
    time_manager(sc_module_name mn, node* m_time_master_node);

private:
    void tick();
};
#endif
