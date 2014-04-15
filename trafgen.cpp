#include "trafgen.h"
#include "cstdlib"
#include "ctime"
#include <vector>

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
        vector<sc_uint<8> > packet;
        for (int i = 0; i < 19; i++)
        {
            //x = (sc_uint<8>)rand() % 255;
            packet.push_back((sc_uint<8>)rand() % 255);
            send_byte();
            wait();
        }
        x = 255;
        send_byte();
        send_packet();
        if (отправили_НЕ_последний_байт_пакета)
            send.notify(timeDelay);
        wait();
    }
}

void trafgen::send_byte()
{
//    cout << "send " << x << '\n';
    out_port->write(x);
    send.notify(5, SC_NS);
}
