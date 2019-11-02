#include <stdio.h>
#include <stdint.h>

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "i2c.h"
#include "tmp102.h"

int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitBootPeripherals();
    BOARD_InitDebugConsole();

    PRINTF("Hello World\n");

    int32_t tempF;
    int post_test;
    uint8_t regvalue[2] = {0x62,0xA0};

    i2c_init();

    post_test = i2c_read_byte(0x90,0x01);
    PRINTF("Config register value : 0x%x\n\r", post_test);

    if(post_test == 98 || post_test == 96)
    	PRINTF("POST passed!\n\r");
    else
    	PRINTF("POST failed!\n\r");

    i2c_write_bytes(0x90,0x01,regvalue,2);

    post_test = i2c_read_byte(0x90,0x01);
    PRINTF("Config register value : 0x%x\n\r", post_test);

    post_test = i2c_read_byte(0x90,0x02);
    PRINTF("TLO value : 0x%x\n\r", post_test);

    post_test = i2c_read_byte(0x90,0x03);
    PRINTF("THI value : 0x%x\n\r", post_test);

    init_alert();

    post_test = i2c_read_byte(0x90,0x02);
    PRINTF("TLO value : 0x%x\n\r", post_test);

    post_test = i2c_read_byte(0x90,0x03);
    PRINTF("THI value : 0x%x\n\r", post_test);

    while(1)
    {
    	tempF = i2c_read_bytes(DEVICE_ADDR,REG_ADDR);
    	if(tempF == -1)
    	{
    		PRINTF("\n\rDisconnected!");
    	}
    	else
    	{
    		PRINTF("\n\rTemperature in Fahrenheit: %dF",tempF);
    	}
    }
    return 0 ;
}
