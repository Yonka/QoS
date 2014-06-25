#include "trafgen.h"
#include "cstdlib"
#include "ctime"
#include <vector>

trafgen::trafgen(sc_module_name mn, int dest_id, int packets_num = 1, sc_time delay = sc_time(0, SC_NS)) : sc_module(mn), dest_id(dest_id), runs(packets_num), delay(delay)
{
    srand((unsigned int) time(0));

    SC_THREAD(gen_event);
    sensitive << gen_next_event;
    
    SC_METHOD(send_packet);
    dont_initialize();
    sensitive << send_event;
}

void trafgen::gen_event()
{
    for (int r = 0; r < runs; r++)
    {
        packet.push_back(EOP_SYMBOL);
        for (int i = 0; i < PACKET_SIZE; i++)
        {
            packet.push_back((sc_uint<8>)rand() % 254 + 1);
        }
        packet.push_back(dest_id);
        send_event.notify(SC_ZERO_TIME);
        wait();
    }
}

void trafgen::send_packet()
{
    trafgen_node_port->write_packet(&packet);
}

void trafgen::new_packet_request()
{
    gen_next_event.notify(SC_ZERO_TIME);
}
