// ----------------------------------------------------------------------------
// Copyright 2017-2019 ARM Ltd.
//  
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//  
//     http://www.apache.org/licenses/LICENSE-2.0
//  
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// ----------------------------------------------------------------------------

#include "frag_buff.h"

bool is_last_fragment(Frag_buff* buff){
    return (buff->more_frag == 0);
}
bool populate_header(Frag_buff* buff, const uint8_t* data) {
    memcpy(buff, &(data[0]), START_DATA_BYTE);
    return true;
}
bool populate_data(Frag_buff* buff, const uint8_t* data) {
    if((buff==NULL) || (data ==NULL)){
        return false;
    }
    buff->payload =  (uint8_t*)malloc(buff->frag_length*sizeof(uint8_t));
    if(buff->payload){
        memcpy(&(buff->payload[0]), &(data[START_DATA_BYTE]),buff->frag_length);
        return true;
    }
    else{
        return false;
    }
}

bool populate_fragment_data(Frag_buff* buff, const uint8_t* data){
    if((buff==NULL) || (data ==NULL)){
        return false;
    }
    buff->payload =  (uint8_t*)malloc(buff->frag_length*sizeof(uint8_t));
    if(buff->payload){
        memcpy(&(buff->payload[0]), &(data[0]),buff->frag_length);
        return true;
    }
    else{
        return false;
    }
}

Frag_buff get_buff(uint8_t seq_num, uint8_t type, uint8_t more_frag, uint8_t frag_num, uint8_t frag_length, uint16_t length)
{
    Frag_buff buff;
    buff.seq_num = seq_num;
    buff.type = type;
    buff.more_frag = more_frag;
    buff.frag_num = frag_num;
    buff.frag_length = frag_length;
    buff.length = length;
    return buff;
}

bool get_header(uint8_t* arr,Frag_buff* buff){
    if((arr==NULL)||(buff == NULL)){
        return false;
    }
    memcpy(&(arr[0]),&(buff[0]), START_DATA_BYTE);
    return true;
}


bool get_payload(uint8_t* arr,Frag_buff* buff){
    if((arr==NULL)||(buff == NULL)){
        return false;
    }
    memcpy(&(arr[0]),&(buff[START_DATA_BYTE]), buff->frag_length);
    return true;
}

bool free_buff(Frag_buff* buff){
    if(buff->payload==NULL){
        return false;//already free
    }
    free(buff->payload);
    return true;
}

