#include "user_interface.h"
#include "user_config.h"
#include "espconn.h"
#include "at_custom.h"
#include "osapi.h"

// digitalWrite([pin], [value])
void ICACHE_FLASH_ATTR
at_setSSLSizeCmd(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter (uint16_t)
    // buffer size
    flag = at_get_next_int_dec(&pPara, &result, &err);
    
    if (err > 0){
	at_port_print("Bad input");
        at_response_error();
        return;
    }

    uint16_t _size = result;

    // Don't go any further if the buffer size is > 8k
    if (_size > 8192){
        os_printf("Invalid SSL buffer size\n");
	at_response_error();
	return;
    }

    // ssl client, 0x01 - client
    bool isSuccess = espconn_secure_set_size(0x01, _size);
    if(!isSuccess){
	os_printf("Setting SSL buffer size failed\n");
	at_response_error();
	return;
    }

    sint16 _checksize = espconn_secure_get_size(0x01);
    if(_checksize != _size){
	os_printf("SSL buffer size does not match\n");
	at_response_error();
	return;
    }

    os_printf("Current SSL buffer size is %d bytes\n", _checksize);
    os_printf("Setting SSL buffer size success\n");
    at_response_ok();
}
