#include "router.h"


router::router(sc_module_name mn, sc_time delay = sc_time(0, SC_NS)): sc_module(mn), delay(delay)
{
    init();
    SC_METHOD(init_fct);
    
    SC_METHOD(redirect);
    dont_initialize();
    sensitive << new_data << free_port;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;

    SC_METHOD(time_code_delayed);
    dont_initialize();
    sensitive << time_code_event;
}

void router::init()
{
    address_destination.resize(num_of_ports);
    fill_n(address_destination.begin(), num_of_ports, -1);
    address_source.resize(num_of_ports);
    fill_n(address_source.begin(), num_of_ports, -1);
    ready_to_send.resize(num_of_ports);
    fill_n(ready_to_send.begin(), num_of_ports, false);
    ready_to_redirect.resize(num_of_ports);
    fill_n(ready_to_redirect.begin(), num_of_ports, false);

    srand (time(NULL));

    int dir;
    for (int i = 0; i < 256; i++)
    {
        dir = rand() % num_of_ports;
        while (dir == i)
        {
            dir = rand() % num_of_ports;
        }
        routing_table.push_back(dir);
    }
}

void router::fct(int num, sc_time holdup)
{
    freed_ports.push_back(num);
    fct_delayed_event.notify(FCT_SIZE * holdup);
}

void router::fct_delayed()
{
    while(!freed_ports.empty())
    {
        cout << this->basename() << " received fct from " << freed_ports.back() << " at "<< sc_time_stamp() << "\n";
        ready_to_send[freed_ports.back()] = true;
        freed_ports.pop_back();
    }
    free_port.notify();
}

void router::time_code(sc_uint<14> t)
{
    if (cur_time == t - 1)
    {
        time_code_event.notify(delay);
    }
    else
        cur_time = t;
}

void router::time_code_delayed()
{
    cur_time++;
    cout << this->basename() << " time " << cur_time << " at " << sc_time_stamp() << "\n";
    for (int i = 0; i < num_of_ports; i++)
    {
        fct_port[i]->time_code(cur_time);
    }
}

void router::write_byte(int num, sc_uint<8> data)
{
    cout << this->basename() << " received " << data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (address_destination[num] == -1)
    {
        int addr = routing_table[data];
        address_destination[num] = addr;
 //       cerr << data;
        fct_port[num]->fct(delay);
    }
    else
    {
        sc_time t = sc_time_stamp() + delay; 
        buf[num] = make_pair(data, t);
        new_data.notify(delay);
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
        if (buf[i].second > sc_time_stamp())    //too early to send
        {
            cerr << buf[i].second << " " << sc_time_stamp() << "\n";
            sc_time t = buf[i].second - sc_time_stamp();
            free_port.notify(t); 
            continue;
        }
        ready_to_redirect[i] = false;
        ready_to_send[address_destination[i]] = false;
        sc_uint<8> data = buf[i].first;
        fct_port[address_destination[i]]->write_byte(data);
        fct_port[i]->fct(delay);
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

void router::init_fct()
{
    for (int i = 0; i < num_of_ports; i++)
    {
        fct_port[i]->fct(delay);
    }
}
