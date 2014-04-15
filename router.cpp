#include "router.h"
#include <sysc\kernel\sc_module.h>

router::router(sc_module_name mn): sc_module(mn)
{
    address_destination.resize(num_of_ports);
    fill_n(address_destination.begin(), num_of_ports, -1);
    address_source.resize(num_of_ports);
    fill_n(address_source.begin(), num_of_ports, -1);
    ready_to_send.resize(num_of_ports);
    fill_n(ready_to_send.begin(), num_of_ports, false);
    ready_to_redirect.resize(num_of_ports);
    fill_n(ready_to_redirect.begin(), num_of_ports, false);
    SC_THREAD(init);
    SC_METHOD(redirect);
    dont_initialize();
    sensitive << new_data << free_port;
}

void router::fct(int num)
{
    ready_to_send[num] = true;
    free_port.notify(SC_ZERO_TIME);
//    cout << this->basename() << " received fct from " << num << " at "<< sc_time_stamp() << "\n";
}

void router::write_byte(int num, sc_uint<8> data)
{
//    cout << this->basename() << " received " << data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (address_destination[num] == -1)
    {
        address_destination[num] = data;
        fct_port[num]->fct();
    }
    else
    {
        buf[num].write(data);
        new_data.notify(SC_ZERO_TIME);
        ready_to_redirect[num] = true;
    }

//    buf[address[num]].write(buf[num].read());
//    redirect(address[num], data);
//    if (data == 255)
//    {
//        busy[address[num]] = false;
//        freed[address[num]].notify(0, SC_MS);
//        address[num] = -1;
//    }
}

void router::redirect()
{
//    cout <<"i've been here\n";
    for (int i = 0; i < num_of_ports; i++)
    {
        if (!inner_connect(i))
            continue;
        ready_to_redirect[i] = false;
        ready_to_send[address_destination[i]] = false;
        sc_uint<8> data = buf[i].read();
        fct_port[address_destination[i]]->write_byte(data);
        fct_port[i]->fct();
        if (data == 255)
        {
            address_source[address_destination[i]] = -1;
            address_destination[i] = -1;
        }
    }
}

bool router::inner_connect(int x)
{
    if (address_destination[x] == -1 || !ready_to_redirect[x])
        return false;
    if (address_source[address_destination[x]] == -1)
        address_source[address_destination[x]] = x;
    if (address_source[address_destination[x]] != x || !ready_to_send[address_destination[x]])
        return false;
    return true;
}

void router::init()
{
    for (int i = 0; i < num_of_ports; i++)
    {
        fct_port[i]->fct();
    }
}