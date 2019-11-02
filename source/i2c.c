#include <stdio.h>
#include <stdint.h>
#include <fsl_common.h>
#include <math.h>

#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "MKL25Z4.h"
#include "fsl_debug_console.h"
#include "i2c.h"
#include "tmp102.h"

uint8_t data[10], dis_count;

void i2c_init(void)
{
	SIM->SCGC4 |= SIM_SCGC4_I2C1_MASK;	//Clock to i2C1 module is enabled
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; //Clock enabled to Port E

	PORTE->PCR[0] |= PORT_PCR_MUX(6);	//PE0 and 1 configured with alternative functionality 6 (I2C1)
	PORTE->PCR[1] |= PORT_PCR_MUX(6);

	I2C1->F = I2C_F_ICR(0x11) | I2C_F_MULT(0);
	I2C1->C1 |= (I2C_C1_IICEN_MASK);

	I2C1->C2 |= I2C_C2_HDRS_MASK;
	I2C1->SLTH |= I2C_SLTH_SSLT(0x01);
}

#define I2C_M_START   I2C1->C1 |= I2C_C1_MST_MASK
#define I2C_M_STOP    I2C1->C1 &= ~I2C_C1_MST_MASK
#define I2C_M_RSTART  I2C1->C1 |= I2C_C1_RSTA_MASK
#define I2C_TRAN      I2C1->C1 |= I2C_C1_TX_MASK
#define I2C_REC       I2C1->C1 &= ~I2C_C1_TX_MASK
#define NACK          I2C1->C1 |= I2C_C1_TXAK_MASK
#define ACK           I2C1->C1 &= ~I2C_C1_TXAK_MASK
#define WAIT      while((I2C1->S & I2C_S_IICIF_MASK)==0) {}	\
					       I2C1->S |= I2C_S_IICIF_MASK;

int i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr)
{
    uint8_t i;
    int temperature, temperatureC, temperatureF;

    I2C_TRAN;
    I2C_M_START;

    I2C1->D = dev_addr;	//write
    I2C_WAIT();
    I2C1->D = reg_addr;
    I2C_WAIT();

    I2C_M_RSTART;		//Repeated Start

    I2C1->D = (dev_addr|0x1);		//Read
    I2C_WAIT();

    I2C_REC;  //Setting it in Tx Mode
    ACK; //Make sure TX ACK

    data[0] = I2C1->D;  //dummy read for internal store
    I2C_WAIT();

    data[0] = I2C1->D;
    I2C_WAIT();

    NACK;

    data[1] = I2C1->D;
    I2C_WAIT();

    I2C_M_STOP;

    for(i=0;i<2;i++)
    {
    	PRINTF("\n\rdata[%d] = %d",i,data[i]);
    }
    if(data[1] == 0 || dis_count < 0)
    {
    	return -1;
    }
    else
    {
    	//dis_count = 0;
        temperature = ((data[0] << 4) + (data[1] >> 4));
        temperatureC = temperature * 0.0625;
        temperatureF = (temperatureC * 1.8) + 32;
        //PRINTF("\n\rTemperature in Celsius: %dC",temperatureC);
        return temperatureF;
    }
}

void I2C_WAIT(void)
{
	  dis_count = 255;
	  while((I2C1->S & I2C_S_IICIF_MASK)==0)
	  {
		if((I2C1->C1 & I2C_C1_TXAK_MASK) == 0)
		{
				PRINTF("\n\rACK received!");
				dis_count--;
				if(dis_count < 0)
				{
					//break;
					I2C1->S |= I2C_S_IICIF_MASK;
					return;
				}
		}
	  }
	  I2C1->S |= I2C_S_IICIF_MASK;
	  return;
}

void i2c_write_byte(uint8_t dev, uint8_t reg, uint8_t data)
{
	volatile int32_t config;

	I2C_TRAN;
	I2C_M_START;
	I2C1->D = dev;
	I2C_WAIT();

	I2C1->D = reg;
	I2C_WAIT();

	I2C1->D = data;
	I2C_WAIT();
	I2C_M_STOP;

	config = i2c_read_byte(0x90,0x01);
	PRINTF("For POST data written is 0x%x\n\r",config);

}

void i2c_write_bytes(uint8_t dev, uint8_t reg, uint8_t data[], uint8_t bytes)
{
	volatile int i, config;

	I2C_TRAN;
	I2C_M_START;
	I2C1->D = dev;
	I2C_WAIT();

	I2C1->D = reg;  //config register address
	I2C_WAIT();

	I2C_M_RSTART;
	I2C1->D = (dev|0x0);
	I2C_WAIT();

	for(i = (bytes-1) ;i >= 0; i--) //for writing msb first
	{
		I2C1->D = data[i];
		I2C_WAIT();

	}

	I2C_M_STOP;

//	config = i2c_read_byte(0x90,0x01);
//	PRINTF("For POST data written is 0x%x\n\r",config);
}

uint8_t i2c_read_byte(uint8_t dev, uint8_t reg)
{
	uint32_t data;

	I2C_TRAN;
	I2C_M_START;
	I2C1->D = dev;

	I2C_WAIT();

	I2C1->D = reg;

	I2C_WAIT();


	I2C_M_RSTART;
	I2C1->D = (dev|0x1);

	I2C_WAIT();

	I2C_REC;
	NACK;

	data = I2C1->D;

	WAIT;

	I2C_M_STOP;
    data = I2C1->D;

    //data = data >> 8;

    PRINTF("For POST data read is 0x%x\n\r",data);

	return data;
}

void init_alert(void)
{
    uint8_t tlo[2],thi[2];

    tlo[0] = 0x00;
    tlo[1] = 0x00;
    i2c_write_bytes(0x90, 0x02, tlo, 2);

    thi[0] = 0xFF;
    thi[1] = 0xF0;
    i2c_write_bytes(0x90, 0x03, thi, 2);

}
