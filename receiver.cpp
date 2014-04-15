#include "receiver.h"

receiver::receiver(sc_module_name mn, int addr) : sc_module(mn), address(addr), write_buf(40), read_buf(8)
{
    ready_to_read = true;
    ready_to_write = false;
    SC_THREAD(sender);
    sensitive << eop;
}

void receiver::write(sc_uint<8> data)
{
    cout << "res " << data << "\n";
    if (data == 255)
       eop.notify(0, SC_MS);
    else
        write_buf.write(data);
}

void receiver::write_byte(int num, sc_uint<8> data)
{
    cout << this->basename() << " received " << data << " at " << sc_time_stamp() << "\n";
    read_buf.write(data);
    fct_port->fct(address);
}

void receiver::fct(int num)
{
     ready_to_write = true;
     fct_event.notify(0, SC_MS);
     cout << this->basename() << " received fct at " << sc_time_stamp() << "\n";
}

void receiver::sender()
{
    fct_port->fct(address);
    while (true)
    {
        tmp_byte = (address + 1) % 2;
        do
        {
            if (!ready_to_write)
            {
                wait(fct_event);
            }
            cout << this->basename() << " send " << tmp_byte << " at " << sc_time_stamp() << "\n";
            fct_port->write_byte(address, tmp_byte);
            ready_to_write = false;
        } while (write_buf.nb_read(tmp_byte));
        wait();
    }
}
