#include "time_manager.h"

time_manager::time_manager(sc_module_name mn, node* tm_node) : sc_module(mn), tm_node(tm_node) 
{
    SC_THREAD(tick);

}

void time_manager::tick()
{
    while (true)
    {
        tm_node->new_time_code();
        wait(1000, SC_NS);
    }
}
