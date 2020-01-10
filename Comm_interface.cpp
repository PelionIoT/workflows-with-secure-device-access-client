#include "comm_interface.h"

bool Comm_interface::init(){
    BLE &ble_interface = BLE::Instance();
    _bleprocess = new BLEProcess(_event_queue, ble_interface);
    _bleprocess->on_init(NULL);
    _blesda = new BLESDA(ble_interface);
	return true;
}
bool Comm_interface::start(){
    bool status = _bleprocess->start();
    _event_queue.dispatch_forever();
    return status;
}

int Comm_interface::wait_for_message(uint8_t **message_out, uint32_t *message_size_out){
     bool success = false;
	 if(_isconnected == false){
	 	return -1;
     }
    int status_code = 0;
    uint8_t *message = NULL;
    uint32_t message_size = 0;

    *message_out = NULL;
    *message_size_out = 0;
	printf("%s", message);

    *message_out = message;
    *message_size_out = message_size;
    return status_code;
}

bool Comm_interface::send_response(uint8_t *message_out, size_t message_size){
	_blesda->write(message_out, message_size);
}

bool Comm_interface::finish(){}
