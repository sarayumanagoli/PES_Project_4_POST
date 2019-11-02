#ifndef _I2C_H_

#define _I2C_H_

#endif

void i2c_init(void);
void I2C_WAIT(void);
void init_alert(void);
int32_t i2c_read_bytes(uint8_t dev_addr, uint8_t reg_addr);
void i2c_write_byte(uint8_t dev, uint8_t reg, uint8_t data);
void i2c_write_bytes(uint8_t dev, uint8_t reg, uint8_t data[], uint8_t bytes);
uint8_t i2c_read_byte(uint8_t dev, uint8_t reg);

