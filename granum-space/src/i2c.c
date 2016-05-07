#include "i2c.h"

#include <avr/io.h>
#include <util/delay.h>

#include "uart-debug.h"

#define I2C_START_TRANSFERED 0x10
#define I2C_RESTART_TRANSFERED 0x08
#define I2C_SLAW_ACK 0x18
#define I2C_SLAW_NO_ACK 0x20
#define I2C_SLAR_ACK 0x40
#define I2C_SLAR_NO_ACK 0x48
#define I2C_READ_ACK 0x50
#define I2C_READ_NO_ACK 0x58
#define I2C_WRITE_ACK 0x28
#define I2C_WRITE_NO_ACK 0x30
#define I2C_ARB_LOST 0x38

#define I2C_TIMEOUT_COUNT 10
#define I2C_TIMEOUT_US 10


i2c_error_t i2c_status_to_error(uint8_t status)
{
	switch (status)
	{
	case I2C_ARB_LOST:
		return I2C_arb_lost;

	case I2C_SLAW_NO_ACK:
	case I2C_SLAR_NO_ACK:
		return I2C_nack_recivied;

	case I2C_WRITE_NO_ACK:
	case I2C_READ_NO_ACK:
		return I2C_nack_recivied;

	};

	return I2C_wrong_status;
}


void i2c_init()
{
	TWBR = 24; //3; // 8 для 16мгц
}


int i2c_start()
{
	TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);

	for(int i = 0; i < I2C_TIMEOUT_COUNT; i++){
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_START_TRANSFERED || status == I2C_RESTART_TRANSFERED)	{
				return 0;
			} else {
				return i2c_status_to_error(status);
			}
		}
	}

	return I2C_timeout;
}


int i2c_stop()
{
	TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);

	for(int i = 0; i < I2C_TIMEOUT_COUNT; i++){
		if(TWCR & (1<<TWINT))
			return 0;

		_delay_us(I2C_TIMEOUT_US);
	}

	return  I2C_timeout;
}


int i2c_send_slaw(uint8_t slave_addr, bool read_access)
{
	TWDR = (slave_addr << 1) | read_access;
	TWCR = (1<<TWINT) | (1<<TWEN);

	for(int i = 0;i < I2C_TIMEOUT_COUNT; i++ ) {
		if (TWCR & (1<<TWINT)) {
			uint8_t status = TWSR & 0xF8;
			if (status == I2C_SLAW_ACK || status == I2C_SLAR_ACK)	{
				return 0;
			} else {
				return i2c_status_to_error(status);
			}
		} else {
			_delay_us(I2C_TIMEOUT_US);
		}
	}

	return I2C_timeout;
}



int i2c_write(const void * data_ptr, size_t data_size)
{
	const uint8_t * byte_ptr = (const uint8_t * )data_ptr;
	for(int i = 0; i < data_size; i++){
		TWDR =  byte_ptr[i];
		TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < I2C_TIMEOUT_COUNT; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_WRITE_ACK) {
					break;
				} else {
					DEBUG("status == 0x%X\r\n", status);
					return i2c_status_to_error(status);
				}
			} else {
				_delay_us(I2C_TIMEOUT_US);
			}
		}

		if(timeout)	return I2C_timeout;

	}

	return 0;
}


int i2c_read(void * data_ptr, size_t data_size, bool NACK_at_end)
{
	uint8_t * byte_ptr = (uint8_t * )data_ptr;

	for(int i = 0; i < data_size; i++){

		if(!(i == data_size-1))
			TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
		else
			TWCR = (1<<TWINT) | (1<<TWEN);

		bool timeout = true;
		for(int j = 0; j < I2C_TIMEOUT_COUNT; j++ ) {
			if (TWCR & (1<<TWINT)) {
				timeout = false;
				uint8_t status = TWSR & 0xF8;
				if (status == I2C_READ_ACK || status == I2C_READ_NO_ACK) {
					break;
				} else {
					DEBUG("status == 0x%X\r\n", status);
					return i2c_status_to_error(status);
				}
			} else {
				_delay_us(I2C_TIMEOUT_US);
			}
		}

		if(timeout)	return I2C_timeout;
		byte_ptr[i] = TWDR;
	}

	return 0;
}
