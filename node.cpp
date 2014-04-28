#include "node.h"

node::node(sc_module_name mn, int addr, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), address(addr), delay(delay), write_buf(40), read_buf(40)
{
    ready_to_write = false;

    SC_METHOD(init);
    
    SC_THREAD(sender);
    dont_initialize();
    sensitive << eop;
    
    SC_METHOD(fct_delayed);
    dont_initialize();
    sensitive << fct_delayed_event;

    SC_METHOD(time_code_delayed);
    dont_initialize();
    sensitive << time_code_event;
}

void node::init()
{
    fct_port->fct(address, delay);
}

void node::write(sc_uint<8> data)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    write_buf.write(data);
    if (data == 255)
    {
       eop.notify();
       cout << this->basename() << " have new packet " << sc_time_stamp() << '\n';
    }
}

//void node::writeTick(double tickValue) {
//}

void node::write_byte(sc_uint<8> data)
{
    cout << this->basename() << " received " << data << " at " << sc_time_stamp() << "\n";
    if (data == 255)
    {
        cout << this-> basename() << " received package: ";
        while (read_buf.nb_read(data))
            cout << data << " ";
        cout << " at " << sc_time_stamp() << "\n";
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
     cout << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void node::time_code(sc_uint<14> t)
{
    if (cur_time == t - 1)
        time_code_event.notify(delay);
    else
        cur_time = t;
}

void node::time_code_delayed()
{
    cur_time++;
    cout << this->basename() << " time " << cur_time << " at " << sc_time_stamp() << "\n";
    fct_port->time_code(cur_time);
}

void node::sender()
{
    while (true)
    {
//        wait() check if it is allowed time-slot
        while (write_buf.nb_read(tmp_byte))
        {
            if (!ready_to_write)
                wait(fct_event);
            cout << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
            wait(delay);
            fct_port->write_byte(address, tmp_byte);
            ready_to_write = false;
        }
        wait();
    }
}
