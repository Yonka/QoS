#include "time_manager.h"

time_manager::time_manager(sc_module_name mn, node* m_time_master_node) : sc_module(mn), m_time_master_node(m_time_master_node) 
{
    SC_THREAD(tick);
    m_t_tc = sc_time(TICK, SC_NS);
    m_tick_value = 0;
}

void time_manager::tick()
{
    while (scheduling != 0)
    {
        if (m_tick_value != 1 && m_tick_value != 2)
            m_time_master_node->new_time_code(m_tick_value);
        m_tick_value++;
        if (scheduling == 2 )
            wait(m_time_master_node->node_QoS_port->get_te());
        else wait(m_time_master_node->node_QoS_port->get_tc());
    }
}
