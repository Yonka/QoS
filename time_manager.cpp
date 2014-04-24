#include "time_manager.h"
#include <sysc\kernel\sc_module.h>

time_manager::time_manager(sc_module_name mn) : sc_module(mn)
{
    SC_THREAD(tick);

}

void time_manager::tick()
{
    while (true)
    {
        wait(1, SC_MS);
        
    }
}
