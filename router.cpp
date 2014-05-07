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

//    SC_METHOD(time_code_delayed);
//    dont_initialize();
//    sensitive << time_code_event;
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
    dest_for_tc.resize(num_of_ports);

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

//void router::time_code(sc_uint<14> t)
//{
//    if (cur_time == t - 1)
//    {
//        time_code_event.notify(delay);
//    }
//    else
//        cur_time = t;
//}

//void router::time_code_delayed()
//{
//    cur_time++;
//    cout << this->basename() << " time " << cur_time << " at " << sc_time_stamp() << "\n";
//    for (int i = 0; i < num_of_ports; i++)
//    {
//        fct_port[i]->time_code(cur_time);
//    }
//}

void router::write_byte(int num, symbol s)
{
    cout << this->basename() << " received " << s.data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (address_destination[num] == -1 && (s.addr != BROADCAST_SYMBOL))
    {
        int addr = routing_table[s.addr];
        address_destination[num] = addr;
//        cerr << data;
        fct_port[num]->fct(delay);
    }
    else 
    {
        if (s.addr == BROADCAST_SYMBOL)
        {
            if (ready_to_redirect[num])
            {
                tmp_buf[num] = buf[num]; 
            }
            fill_n(dest_for_tc.begin(), num_of_ports, true);
            dest_for_tc[num] = false;
        }
        sc_time t = sc_time_stamp() + delay; 
        buf[num] = make_pair(s, t);
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
        if (buf[i].second > sc_time_stamp())    //too early to send
        {
//            cerr << buf[i].second << " " << sc_time_stamp() << "\n";
            sc_time t = buf[i].second - sc_time_stamp();
            free_port.notify(t); 
            continue;
        }
        if (buf[i].first.addr == BROADCAST_SYMBOL)
        {
            bool freed = true;
            for (int j = 0; j < num_of_ports; j++)
            {
                if (dest_for_tc[j] && ready_to_send[j])
                {
                    dest_for_tc[j] = false;
                    ready_to_send[j] = false;
                    fct_port[j]->write_byte(buf[i].first);
                    fct_port[i]->fct(delay);
                    freed = false;
                }
            }
            if (freed)
            {
                buf[i] = tmp_buf[i];
            }
            continue;
        }
        if (!inner_connect(i))
            continue;
        ready_to_redirect[i] = false;
        ready_to_send[address_destination[i]] = false;
        fct_port[address_destination[i]]->write_byte(buf[i].first);
        fct_port[i]->fct(delay);
        if (buf[i].first.data == EOP_SYMBOL)
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
