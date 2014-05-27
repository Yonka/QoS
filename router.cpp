#include "router.h"


router::router(sc_module_name mn, int id,int ports, sc_time delay/* = sc_time(0, SC_NS)*/, vector<int> table): sc_module(mn), id(id), ports(ports), delay(delay), routing_table(table)
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
    address_destination.resize(ports);
    fill_n(address_destination.begin(), ports, -1);

    address_source.resize(ports);
    fill_n(address_source.begin(), ports, -1);

    ready_to_send.resize(ports);
    fill_n(ready_to_send.begin(), ports, false);

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
        buf.push_back(symbol(0,0,nchar));
        tmp_buf.push_back(symbol(0,0,nchar));
        fct_port.push_back(new sc_port<conn_I>);
    }
//    srand (time(NULL));

//    int dir;
//    for (int i = 0; i < 256; i++)
//    {
//        dir = rand() % ports;
//        while (dir == i)
//        {
//            dir = rand() % ports;
//        }
//        routing_table.push_back(dir);
//    }
}

void router::fct(int num)
{
//TODO: delays for fct 
    freed_ports.insert(pair<int, sc_time>(num, sc_time_stamp()));
    fct_delayed_event.notify(FCT_SIZE * delays[num]);
}

void router::fct_delayed()
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
                cerr << this->basename() << " received fct from " << i << " at "<< sc_time_stamp() << "\n";
                ready_to_send[i] = true;
                freed_ports.erase(it);
                free_port.notify();
            }
        }
    }
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
    cerr << this->basename() << " received " << s.data << " on port " << num << " at " << sc_time_stamp() << "\n";
    if (s.t == lchar)
    {
        tmp_buf[num] = s;
        new_data.notify(SC_ZERO_TIME);
        fill_n(dest_for_tc.begin(), ports, true);
        dest_for_tc[num] = false;   //we already have it
    }
    else
    {
       if (address_destination[num] == -1)
        {
            int addr = routing_table[s.addr];
            address_destination[num] = addr;
        }
        ready_to_redirect[num] = true;
        buf[num] = s;
        new_data.notify(SC_ZERO_TIME);
    }
}

void router::redirect()
{
    redirect_ports();       //free from data
    redirect_time();        //time
    redirect_fct();
    redirect_connect();     //data
}

void router::redirect_ports()
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

            (*fct_port[address_destination[i]])->write_byte(direct[address_destination[i]], buf[i]);
//            fct_port[i]->fct();       //fct-sending moved to another function
            dest_for_fct[i] = true;
            new_data.notify(SC_ZERO_TIME);
            if (buf[i].data == EOP_SYMBOL)
            {
                address_source[address_destination[i]] = -1;
                address_destination[i] = -1;
            }
        }
    }
}

void router::redirect_time()
{
    for (int i = 0; i < ports; i++)
    {
        if (tmp_buf[i].t == lchar)
        {
            if (out_proc[i] == 2 || out_proc[i] == 3)
                continue;

            out_proc[i] = 1;

            for (int j = 0; j < ports; j++)
            {
                if (!dest_for_tc[j])
                    continue;
                if (/*ready_to_send[j]&& */in_proc[j].first == 0)
                {
                    in_proc[j].first = 1;
                    in_proc[j].second = sc_time_stamp() + delays[j] * tmp_buf[i].t + delay;
                    free_port.notify(delays[j] * tmp_buf[i].t + delay);
//                    ready_to_send[j] = false;         //we don't need fct
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
                    (*fct_port[j])->write_byte(direct[j], tmp_buf[i]);
                    //                    fct_port[i]->fct(delay);      //what???
                }              

            }
            bool freed = true;
            for (int j = 0; j < ports; j++)
                if (dest_for_tc[j])
                    freed = false;
            if (freed)
            {
                tmp_buf[i].t = nchar;
                out_proc[i] = 0;
//                fct_port[i]->fct();   //TODO: ask
            }
        }
    }
}

void router::redirect_fct()
{
    for (int i = 0; i < ports; i++)
    {
        if (dest_for_fct[i] && in_proc[i].first == 0)
        {
            in_proc[i].first = 2;
            in_proc[i].second = sc_time_stamp() + delays[i] * FCT_SIZE;
            dest_for_fct[i] = false;
            (*fct_port[i])->fct(direct[i]);
            continue;
        }
        if (in_proc[i].first == 2 && in_proc[i].second > sc_time_stamp())
        {
            free_port.notify(in_proc[i].second - sc_time_stamp());
            continue;
        }
        if (in_proc[i].first == 2)
        {
            in_proc[i].first = 0;
        }              
    }
}

void router::redirect_connect()
{
    for (int i = 0; i < ports; i++)
    {
        if (!inner_connect(i))
            continue;
        in_proc[address_destination[i]].second = sc_time_stamp() + delays[address_destination[i]] * buf[i].t + delay;
        new_data.notify(delays[address_destination[i]] * buf[i].t + delay);
        ready_to_redirect[i] = false;
        ready_to_send[address_destination[i]] = false;
        in_proc[address_destination[i]].first = 3;
        out_proc[i] = 3;
    }
}

bool router::inner_connect(int x)
{
    if (address_destination[x] == -1 || !ready_to_redirect[x])
        return false;
    if (address_source[address_destination[x]] == -1)
        address_source[address_destination[x]] = x;
    if (in_proc[address_destination[x]].first != 0 || out_proc[x] != 0)
        return false;
    if (address_source[address_destination[x]] != x || !ready_to_send[address_destination[x]])
        return false;
    return true;
}

void router::init_fct()
{
    for (int i = 0; i < ports; i++)
    {
//        fct_port[i]->fct();       //fct-sending moved to another function
        dest_for_fct[i] = true;
        new_data.notify();  //TODO: immediate?
    }
}
