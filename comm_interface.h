#ifndef _COMM_INTERFACE_H_
#define _COMM_INTERFACE_H_
#endif
#include "events/EventQueue.h"
#include "BLEProcess.h"
#include "blesda.h"
//#include "ftcd_comm_base.h"


class Comm_interface{
    public:
        Comm_interface(){
        };
        bool init();
        bool start();
        int wait_for_message(uint8_t **message_out, uint32_t *message_size_out);
        bool send_response(uint8_t* response, size_t message_size);
        bool finish();
    private:
        bool isconnected();
        events::EventQueue _event_queue;
        BLEProcess* _bleprocess;
        BLESDA* _blesda;
        bool _isconnected = false;
};