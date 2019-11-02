#ifndef _I2C_H_

#define _I2C_H_

#endif

void i2c_init(void);
//int i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr, uint8_t data[], int8_t data_count);
void i2c_write_byte(uint8_t dev, uint8_t reg, uint8_t data);
int i2c_read_bytes_POST(uint8_t dev_addr, uint8_t reg_addr);
uint8_t i2c_read_byte(uint8_t dev, uint8_t reg);

