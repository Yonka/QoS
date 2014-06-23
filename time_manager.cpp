#include "time_manager.h"

time_manager::time_manager(sc_module_name mn, node* tm_node) : sc_module(mn), tm_node(tm_node) 
{
    SC_THREAD(tick);
    m_t_tc = sc_time(TICK, SC_NS);
    m_tickValue = 0;
}

void time_manager::tick()
{
    while (scheduling != 0)
    {
        if (m_tickValue != 1 && m_tickValue != 2)
            tm_node->new_time_code(m_tickValue);
        m_tickValue++;
        if (scheduling == 2 )
            wait(tm_node->m_QoS.get_te());
        else wait(tm_node->m_QoS.get_tc());
    }
}
