#include "routing_switch.h"


routing_switch::routing_switch(sc_module_name mn, int id,int ports, sc_time delay/* = sc_time(0, SC_NS)*/, vector<int> table): sc_module(mn), id(id), ports(ports), delay(delay), routing_table(table)
{
    init();
    SC_METHOD(init_fct);
    
    SC_METHOD(redirect);
    dont_initialize();
    sensitive << new_data << free_port;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;
}

void routing_switch::init()
{
    std::srand (unsigned(std::time(0)));
    cur_time = -1;
    time_source = 0;
    address_destination.resize(ports);
    fill_n(address_destination.begin(), ports, -1);

    address_source.resize(ports);
    fill_n(address_source.begin(), ports, -1);

    processed.resize(ports);
    fill_n(processed.begin(), ports, 0);

    ready_to_send.resize(ports);
    fill_n(ready_to_send.begin(), ports, 0);

    ready_to_redirect.resize(ports);
    fill_n(ready_to_redirect.begin(), ports, false);

    dest_for_tc.resize(ports);
    dest_for_fct.resize(ports);
    in_proc.resize(ports);
    fill_n(in_proc.begin(), ports, make_pair(0, sc_time(0, SC_NS)));

    out_proc.resize(ports);
    fill_n(out_proc.begin(), ports, false);

    direct.resize(ports);

    for (int i = 0; i < ports; i++)
    {
        queue<symbol> t;
        data_buffer.push_back(t);
        time_buffer.push_back(symbol(0, 0, 0, nchar));
        fct_port.push_back(new sc_port<data_if>);
    }
}

void routing_switch::fct(int num)
{
    stat_n++;
    freed_ports.insert(pair<int, sc_time>(num, sc_time_stamp()));
    fct_delayed_event.notify(FCT_SIZE * delays[num]);
}

void routing_switch::fct_delayed()
{
    for (int i = 0; i < ports; i++)
    {
        map<int, sc_time>::iterator it;
        it = freed_ports.find(i);
        if (it != freed_ports.end())
        {
            if (it->second + FCT_SIZE * delays[i] > sc_time_stamp())
            {
                fct_delayed_event.notify(it->second + FCT_SIZE * delays[i] - sc_time_stamp());
            }
            else
            {
//                cerr << this->basename() << " received fct from " << i << " at "<< sc_time_stamp() << "\n";
                ready_to_send[i] = 8;
                freed_ports.erase(it);
                free_port.notify();
            }
        }
    }
}

void routing_switch::write_byte(int num, symbol s)
{
//    cerr << this->basename() << " received " << s.data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (s.t == lchar)
    {
        stat_k++;
        if (s.data == (cur_time + 1) % 64)
        {
            time_buffer[num] = s;
            new_data.notify(SC_ZERO_TIME);
            time_source = num;
            fill_n(dest_for_tc.begin(), ports, true);
            dest_for_tc[num] = false;   //we already have it
        }
        cur_time = s.data;
    }
    else
    {
        stat_m++;
        if (address_destination[num] == -1)
        {
            int addr = routing_table[s.addr];
            address_destination[num] = addr;
        }
        data_buffer[num].push(s);
        new_data.notify(SC_ZERO_TIME);
    }
}

void routing_switch::redirect()
{
    redirect_close();           //free from data
    if (time_buffer[time_source].t == lchar)
        redirect_time();        //time
    redirect_fct();
    redirect_data();            //data
}

void routing_switch::redirect_close()
{
    for (int i = 0; i < ports; i++)
    {
        if (out_proc[i] == 3  && in_proc[address_destination[i]].second > sc_time_stamp())    //too early to free ports
        {
            sc_time t = in_proc[address_destination[i]].second - sc_time_stamp();
            free_port.notify(t); 
            continue;
        }
        else if (out_proc[i] == 3)
        {
            in_proc[address_destination[i]].first = 0;
            out_proc[i] = 0;

            (*fct_port[address_destination[i]])->write_byte(direct[address_destination[i]], data_buffer[i].front());
            processed[i]++;
            if (processed[i] == 8)
            {
                redirecting_fct.insert(i);
                processed[i] = 0;
            }
            new_data.notify(SC_ZERO_TIME);
            if (data_buffer[i].front().data == EOP_SYMBOL)
            {
                address_source[address_destination[i]] = -1;
                address_destination[i] = -1;
            }
            data_buffer[i].pop();
        }
    }
}

void routing_switch::redirect_time()
{
    if ((out_proc[time_source] == 0 || out_proc[time_source] == 1))
    {
        out_proc[time_source] = 1;

        for (int j = 0; j < ports; j++)
        {
            if (!dest_for_tc[j])
                continue;
            if (in_proc[j].first == 0)
            {
                in_proc[j].first = 1;
                in_proc[j].second = sc_time_stamp() + delays[j] * time_buffer[time_source].t + delay;
                free_port.notify(delays[j] * time_buffer[time_source].t + delay);
                continue;
            }
            if (in_proc[j].first == 1 && in_proc[j].second > sc_time_stamp())
            {
                free_port.notify(in_proc[j].second - sc_time_stamp());
                continue;
            }
            if (in_proc[j].first == 1)
            {
                in_proc[j].first = 0;
                dest_for_tc[j] = false;
                (*fct_port[j])->write_byte(direct[j], time_buffer[time_source]);
            }              
        }
        bool freed = true;
        for (int j = 0; j < ports; j++)
            if (dest_for_tc[j])
                freed = false;
        if (freed)
        {
            time_buffer[time_source].t = nchar;
            out_proc[time_source] = 0;
            free_port.notify(SC_ZERO_TIME);
        }
    }
}

void routing_switch::redirect_fct()
{
    for (set<int>::iterator i = redirecting_fct.begin(); i != redirecting_fct.end(); ) 
    {
        if (in_proc[*i].first == 0)
        {
            in_proc[*i].first = 2;
            in_proc[*i].second = sc_time_stamp() + delays[*i] * FCT_SIZE;
            (*fct_port[*i])->fct(direct[*i]);
            i++;
            continue;
        }
        if (in_proc[*i].first == 2 && in_proc[*i].second > sc_time_stamp())
        {
            free_port.notify(in_proc[*i].second - sc_time_stamp());
            i++;
            continue;
        }
        if (in_proc[*i].first == 2)
        {
            in_proc[*i].first = 0;
            redirecting_fct.erase(i++);
        }              
        else
            ++i;
    }
}

void routing_switch::redirect_data()
{
    vector<int> shuffled;
    for (int i = 0; i < ports; i++) shuffled.push_back(i);
    std::random_shuffle(shuffled.begin(), shuffled.end());
    for (int j = 0; j < ports; j++)
    {
        int i = shuffled[j];
        if (!inner_connect(i))
            continue;
        in_proc[address_destination[i]].second = sc_time_stamp() + delays[address_destination[i]] * data_buffer[i].front().t + delay;
        new_data.notify(delays[address_destination[i]] * data_buffer[i].front().t + delay);
        ready_to_send[address_destination[i]]--;
        in_proc[address_destination[i]].first = 3;
        out_proc[i] = 3;
    }
}

bool routing_switch::inner_connect(int x)
{
    if (address_destination[x] == -1 || data_buffer[x].size() == 0)
        return false;
    if (address_source[address_destination[x]] == -1)
        address_source[address_destination[x]] = x;
    if (in_proc[address_destination[x]].first != 0 || out_proc[x] != 0)
        return false;
    if (address_source[address_destination[x]] != x || ready_to_send[address_destination[x]] == 0)
        return false;
    return true;
}

void routing_switch::init_fct()
{
    for (int i = 0; i < ports; i++)
    {
        redirecting_fct.insert(i);
        new_data.notify();  
    }
}
