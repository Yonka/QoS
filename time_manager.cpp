#include "time_manager.h"

time_manager::time_manager(sc_module_name mn, node* m_time_master_node) : sc_module(mn), m_time_master_node(m_time_master_node) 
{
    SC_THREAD(tick);
    m_tick_period = sc_time(TICK, SC_NS);
    m_time_code_value = 0;
}

void time_manager::tick()
{
    int m_scheduling = m_time_master_node->node_QoS_port->get_scheduling();
    if (m_scheduling == WITHOUT_SCHEDULING)
        return;
    m_tick_period = (m_scheduling == ALGORYTHM_2) ? m_time_master_node->node_QoS_port->get_te() : m_time_master_node->node_QoS_port->get_tc();
    while (true)
    {
        m_time_master_node->newTimeCode(m_time_code_value);
        m_time_code_value = (m_time_code_value + 1) % 64;
            wait(m_tick_period);
    }
}
