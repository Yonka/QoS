#include "node.h"

node::node(sc_module_name mn, int addr, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), address(addr), delay(delay), write_buf(40), read_buf(40)
{
    ready_to_write = false;
    have_time_code_to_send = false;
    cur_time = 0;
    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << eop;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;

//    SC_METHOD(time_code_delayed);
//    dont_initialize();
//    sensitive << time_code_event;
}

void node::init()
{
    fct_port->fct(address, delay);
}

void node::write(sc_uint<8> data)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    write_buf.write(data);
    if (data == EOP_SYMBOL)
    {
       eop.notify();
       cout << this->basename() << address  << " have new packet " << sc_time_stamp() << '\n';
    }
}

void node::write_byte(symbol s)
{
    sc_uint<8> data;
    cout << this->basename() << address << " received " << s.data << " at " << sc_time_stamp() << "\n";
    if (s.data == EOP_SYMBOL)
    {
        cout << this-> basename() << address << " received package: ";
        while (read_buf.nb_read(data))
            cout << data << " ";
        cout << " at " << sc_time_stamp() << "\n";
    }
    else if (s.t == lchar)
    {
        time_code(s.data);
//TODO
    }
    else
        read_buf.write(data);
    fct_port->fct(address, delay);
}

void node::fct(sc_time holdup)
{
     fct_delayed_event.notify(FCT_SIZE * holdup);
}

void node::fct_delayed()
{
    ready_to_write = true;
    fct_event.notify();
     cout << this->basename() << address << " received fct at " << sc_time_stamp() << "\n";
}

void node::time_code(sc_uint<14> t)
{
    cur_time = t;
    cout << address << " received tc\n";
}

void node::new_time_code()
{
    cout << address << " send tc\n";
    cur_time++;
    have_time_code_to_send = true;
    eop.notify();
}
void node::sender()
{
    while (true)
    {
//        wait() check if it is allowed time-slot
        int receiver_addr = -1;
        while (write_buf.nb_read(tmp_byte) || have_time_code_to_send)
        {
            if (!ready_to_write)
                wait(fct_event);
            symbol s;
            if (have_time_code_to_send)
            {
                s = symbol(cur_time, BROADCAST_SYMBOL, lchar);
                cout << this->basename() << address << " send tc" << cur_time << " at " << sc_time_stamp() << "\n";
                have_time_code_to_send = false;
            }
            else
            {
                if (receiver_addr = -1) 
                    receiver_addr = tmp_byte;
                s = symbol(tmp_byte, receiver_addr, nchar);
                cout << this->basename() << address << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
            }
            wait(delay * s.t);
            fct_port->write_byte(address, s);
            ready_to_write = false;
        }
        receiver_addr = -1;
        wait();
    }
}
