#include "time_manager.h"
#include "defs.h"

time_manager::time_manager(sc_module_name mn, node* tm_node) : sc_module(mn), tm_node(tm_node) 
{
    SC_THREAD(tick);
    m_t_tc = sc_time(100, SC_NS);
    t_tick_value = 0;
}

void time_manager::tick()
{
    while (true)
    {
        tm_node->new_time_code(m_tick_value);
        m_tickValue++;
        wait(m_t_tc, SC_NS);
    }
}
