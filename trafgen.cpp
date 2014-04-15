#include "trafgen.h"
#include "cstdlib"
#include "ctime"

trafgen::trafgen(sc_module_name mn, int param = 1) : sc_module(mn), runs(param)
{
    srand((unsigned int) time(0));
    SC_THREAD(gen_event);
    sensitive << send;
}

void trafgen::gen_event()
{
    for (int r = 0; r < runs; r++)
    {
        for (int i = 0; i < 19; i++)
        {
            x = (sc_uint<8>)rand() % 255;
            send_byte();
            wait();
        }
        x = 255;
        send_byte();
        wait();
    }
}

void trafgen::send_byte()
{
//    cout << "send " << x << '\n';
    out_port->write(x);
    send.notify(5, SC_NS);
}
