#include "receiver.h"
#include <sysc\kernel\sc_module.h>

receiver::receiver(sc_module_name mn, int addr) : sc_module(mn), address(addr), write_buf(40), read_buf(40)
{
    ready_to_write = false;
    SC_METHOD(init);
    SC_THREAD(sender);
    dont_initialize();
    sensitive << eop;
}

void receiver::init()
{
    fct_port->fct(address);
}

void receiver::write(sc_uint<8> data)
{
//    cout << "res " << data << " at " << sc_time_stamp() << "\n";
    write_buf.write(data);
    if (data == 255)
       eop.notify(SC_ZERO_TIME);
}

void receiver::write_byte(sc_uint<8> data)
{
//    cout << this->basename() << " received " << data << " at " << sc_time_stamp() << "\n";
    if (data == 255)
    {
        cout << this-> basename() << " received package: ";
        while (read_buf.nb_read(data))
            cout << data << " ";
        cout << " at " << sc_time_stamp() << "\n";
    }
    else
        read_buf.write(data);
    fct_port->fct(address);
}

void receiver::fct()
{
     ready_to_write = true;
     fct_event.notify(SC_ZERO_TIME);
//     cout << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void receiver::sender()
{
    while (true)
    {
        tmp_byte = 0;
        do
        {
            if (!ready_to_write)
                wait(fct_event);
//            cout << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
            if (address == 0)
                wait(50, SC_NS);
            fct_port->write_byte(address, tmp_byte);
            ready_to_write = false;
        } while (write_buf.nb_read(tmp_byte));
        wait();
    }
}
