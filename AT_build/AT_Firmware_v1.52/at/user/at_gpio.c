#include "user_interface.h"
#include "user_config.h"
#include "gpio.h"
#include "at_custom.h"
#include "osapi.h"

uint32_t pinMap[] = {
	PERIPHS_IO_MUX_GPIO0_U, // 0
	0, // 1 (TX, can't change)
	PERIPHS_IO_MUX_GPIO2_U, // 2
	0, // 3 (RX, can't change)
	PERIPHS_IO_MUX_GPIO4_U, // 4
	PERIPHS_IO_MUX_GPIO5_U, // 5
	0, // 6
	0, // 7
	0, // 8
	0, // 9
	0, // 10
	0, // 11
	PERIPHS_IO_MUX_MTDI_U, // 12
	PERIPHS_IO_MUX_MTCK_U, // 13
	PERIPHS_IO_MUX_MTMS_U, // 14
	PERIPHS_IO_MUX_MTDO_U // 15
};

uint32_t pinFunction[] = {
	FUNC_GPIO0,
	0,
	FUNC_GPIO2,
	0,
	FUNC_GPIO4,
	FUNC_GPIO5,
	0,
	0,
	0,
	0,
	0,
	0,
	FUNC_GPIO12,
	FUNC_GPIO13,
	FUNC_GPIO14,
	FUNC_GPIO15
};

uint8_t pinValid(int pin)
{
	// Don't go any further if the pin is un-usable or non-existant
	if ((pin > 16) || (pinMap[pin] == 0))
		return 0;

	return 1;
}

// digitalWrite([pin], [value])
void ICACHE_FLASH_ATTR
at_setupWritePinCmd(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter (uint8_t)
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);
    // flag must be true because there are more parameters
    if (flag == FALSE)
	{
        at_response_error();
        return;
    }

	uint8_t pin = result;

	// Don't go any further if the pin is un-usable or non-existant
	if (!pinValid(pin))
	{
		at_response_error();
		return;
	}

	PIN_FUNC_SELECT(pinMap[pin], pinFunction[pin]);
	//GPIO_OUTPUT_SET(pin, 0);

	if (*pPara++ != ',') { // skip ','
		at_response_error();
		return;
	}

	flag = at_get_next_int_dec(&pPara, &result, &err);

	uint8_t value = result;

	if (value == 1)
	{
		GPIO_OUTPUT_SET(pin, 1);
		//os_sprintf(buffer, "%d:HIGH\r\n", pin);
	}
	else if (value == 0)
	{
		GPIO_OUTPUT_SET(pin, 0);
		//os_sprintf(buffer, "%d=LOW\r\n", pin);
	}
	else
	{
		at_response_error();
		return;
	}

    if (*pPara != '\r') {
        at_response_error();
        return;
    }

	//at_port_print(buffer);
	at_response_ok();
}

// digitalRead([pin])
void ICACHE_FLASH_ATTR
at_setupReadPinCmd(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    uint8 buffer[32] = {0};
    pPara++; // skip '='

    //get the first parameter (uint8_t)
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be true because there are more parameters
    //if ((flag == FALSE) && (result > 0) )
	if (err > 0)
	{
		at_port_print("Bad input");
        at_response_error();
        return;
    }

	uint8_t pin = result;
	// Don't go any further if the pin is un-usable or non-existant
	if (!pinValid(pin))
	{
		at_response_error();
		return;
	}

	PIN_FUNC_SELECT(pinMap[pin], pinFunction[pin]);
	//GPIO_DIS_OUTPUT(pin);
	//PIN_PULLUP_DIS(pinMap[pin]);

	uint8_t value = GPIO_INPUT_GET(pin);
	if (value == 1)
	{
		os_sprintf(buffer, "%d:HIGH\r\n", pin);
	}
	else if (value == 0)
	{
		os_sprintf(buffer, "%d:LOW\r\n", pin);
	}
	else
	{
		at_response_error();
		return;
	}

	at_port_print(buffer);
	at_response_ok();
}
