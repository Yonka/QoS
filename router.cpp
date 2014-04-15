#include "router.h"

router::router(sc_module_name mn): sc_module(mn)
{
    address.resize(num_of_ports);
    fill_n(address.begin(), num_of_ports, -1);
    busy.resize(num_of_ports);
    fill_n(busy.begin(), num_of_ports, false);
    ready.resize(num_of_ports);
    fill_n(ready.begin(), num_of_ports, false);
    SC_THREAD(init);
}

void router::fct(int num)
{
    ready[num] = true;
    fct_event[num].notify(0, SC_MS);
    cout << this->basename() << " received fct from " << num << " at "<< sc_time_stamp() << "\n";
}

void router::write_byte(int num, sc_uint<8> data)
{
    cout << this->basename() << " received " << data << " on port " << num << " at " << sc_time_stamp() << "\n";
//    if (data == 255)
//        eop.notify(0, SC_MS);
//    else
//    buf[num].write(data);
    if (address[num] == -1)
    {
        address[num] = data;
        if (busy[data])
            wait(freed[data]);
        busy[data] = true;
    }
    
    fct_port[num]->fct(-1);
    
//    buf[address[num]].write(buf[num].read());
    redirect(address[num], data);
    if (data == 255)
    {
        busy[address[num]] = false;
        freed[address[num]].notify(0, SC_MS);
        address[num] = -1;
    }
}

void router::redirect(int num, sc_uint<8> data)
{
    if (!ready[num])
        next_trigger(fct_event[num]);
    fct_port[num]->write_byte(-1, data);
}

void router::init()
{
    for (int i = 0; i < num_of_ports; i++)
    {
        fct_port[i]->fct(0);
    }
}