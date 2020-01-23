#include "include/protocoltranslator.h"
#include "mbed-trace/mbed_trace.h"
#include "pv_endian.h"

#define TRACE_GROUP "PT"

size_t ProtocolTranslator::_read(uint8_t* message, size_t message_size){
    if(memcpy(message, &_buffer[_index],message_size)==NULL){
        mbed_tracef(TRACE_ACTIVE_LEVEL_ERROR, TRACE_GROUP,"Can not read message");
        return MBED_ERROR_ALLOC_FAILED;
    }
    _index +=message_size;
    return message_size;
}

size_t ProtocolTranslator::_write(uint8_t* message, size_t message_size){
   // return _blesdahandler->write(message, message_size);
}

bool ProtocolTranslator::is_token_detected(){
    for(int i = 0; i< FTCD_MSG_HEADER_TOKEN_SIZE_BYTES; i++){
        if(_buffer[i]!=_message_header[i]){
            return false;
        }
    }
     _index +=FTCD_MSG_HEADER_TOKEN_SIZE_BYTES;
        return true;
}

uint32_t ProtocolTranslator::read_message_size(){
    uint32_t message_size = (_buffer[_index]<<24)+
                            (_buffer[++_index]<<16)+
                            (_buffer[++_index]<<8)+
                            _buffer[++_index];
    mbed_tracef(TRACE_ACTIVE_LEVEL_INFO,TRACE_GROUP, "Index of buffer - %d, message size: %d ",_index,message_size);
    return message_size;
}
// this is just for test..Similar data comes from SDA. As we haven;t implemented the protocol over BLE yet..I am testing the code
// hardcoding the similar buffer that comes from SDA.
void ProtocolTranslator::init(){
    uint8_t request[497] = {0x6d, 0x62, 0x65, 0x64, 0x64, 0x62, 0x61, 0x70,0,0,1,229,130, 3, 89,1, 224, 210, 132, 67, 161, 1, 38, 160, 89, 1, 148, 131, 27, 227, 112,
    189, 255, 182, 146, 168, 85, 131, 1, 105, 99, 111, 110, 102, 105, 103, 117,114, 101, 129, 101, 104, 101, 108, 108, 111, 89, 1, 116, 216, 61,  210, 132,
    67, 161, 1, 38, 160, 89, 1, 38, 169, 12, 120, 26, 114, 101, 97, 100, 45, 100, 97, 116, 97, 32, 99, 111, 110, 102, 105, 103 ,117 ,114, 101, 32,117,112,
    100, 97, 116, 101, 24, 25, 161, 1, 164, 1, 2, 32, 1, 33, 88, 32, 136, 22, 235, 120, 187, 220, 214, 6, 53, 211, 254, 166, 251, 242, 121, 194, 13  ,71,
    100, 224, 151, 3 ,128, 151, 244, 76 ,155, 221, 139, 98, 3, 142, 34, 88, 32, 241, 37, 253, 184, 167, 134, 42, 158, 108, 129, 168, 33, 95, 117, 236, 226,
    87, 130, 148, 180, 147, 22, 249, 62, 99, 239, 230, 114, 82, 86, 183, 147, 2, 120, 32, 48, 49, 54, 97, 101, 52, 57, 97, 102, 98, 54, 51, 57, 50, 52 ,54,
    55, 99, 101, 99, 50, 56, 53, 52, 48, 48, 48, 48, 48, 48, 48, 48, 3, 129, 120, 35, 101 ,112, 58, 48, 49, 54 ,101, 101, 97, 100, 50, 57, 51, 101, 98 ,57,
    50, 54, 99, 97, 53, 55, 98, 97, 57, 50, 55, 48, 51, 99, 48, 48, 48, 48, 48, 4,26, 94, 33, 106, 185, 5, 26, 94, 32, 25, 57 ,6 ,26, 94, 32, 25, 57, 1,
    120, 72 ,109, 98, 101, 100, 46, 116, 97, 46, 49 ,54 ,68 ,51 ,52 ,48 ,49, 49, 48, 67, 53,53, 56 ,66 ,56 ,65 ,65 ,66 ,70 ,65 ,65 ,52 ,55 ,68 ,66 ,48 ,48,
    52 ,67 ,66 ,52 ,50 ,57 ,56 ,48 ,66 ,56 ,51 ,68 ,55 ,49 ,66 ,54 ,49 ,69 ,69 ,56 ,52 ,67 ,50 ,68 ,68 ,67 ,70 ,70 ,57 ,68 ,69 ,48 ,70 ,55 ,56 ,55 ,69 ,7,
    80 ,1 ,111 ,173 ,98 ,136 ,198 ,154 ,152 ,58 ,118 ,1 ,189 ,0 ,0 ,0 ,0 ,88 ,64 ,49 ,95 ,181 ,233 ,62 ,60 ,143 ,115 ,133 ,243 ,100 ,198 ,40 ,239 ,144 ,179,
    190 ,128 ,58 ,128 ,70 ,109 ,219 ,248 ,168 ,151 ,18 ,237 ,196 ,118 ,10 ,46 ,24 ,182 ,92 ,91 ,201 ,191 ,82 ,25, 91 ,10 ,4 ,164 ,113 ,175 ,231 ,98 ,11,
    153 ,225 ,2, 57 ,217 ,3 ,218 ,174 ,172 ,203 ,108 ,166 ,31, 215 ,139 ,88 ,64 ,137 ,245 ,191 ,6 ,170 ,2 ,208 ,78 ,237 ,48 ,140 ,55 ,125 ,227 ,77 ,223,
    158 ,106 ,206, 149 ,52 ,22, 142 ,94 ,14 ,164, 8 ,69, 97, 119 ,94 ,234 ,200 ,137 ,158 ,57 ,232 ,159 ,243 ,206 ,255 ,61 ,100 ,14, 180 ,160 ,210 ,73 ,105,
    25 ,137 ,133 ,203 ,184 ,47 ,211 ,39 ,246 ,240 ,124 ,152 ,165 ,81 ,31};

    _buffer = request;
    if(is_token_detected() == false){
        mbed_tracef(TRACE_ACTIVE_LEVEL_INFO,TRACE_GROUP,"Token not detected");
    }
    else {
    printf("Token is detected\r\n");
    }
    uint32_t message_size = read_message_size();

    mbed_tracef(TRACE_ACTIVE_LEVEL_INFO,TRACE_GROUP, "message size: %d",message_size);

}