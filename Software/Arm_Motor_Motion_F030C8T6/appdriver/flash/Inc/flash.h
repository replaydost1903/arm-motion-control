/*
 * flash.h
 *
 *  Created on: Jul 11, 2024
 *      Author: Kubilay
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "main.h"
#include "rs485.h"

#define Flash_address (uint32_t)(0x08000000+1024*63) // page 63 -> 0x0800FC00

typedef struct
{
	uint8_t addr_index;
	uint16_t ui16_buffer[5];
	uint32_t write_addr;
	uint16_t crc_val;

}flash_control_t;

//flash apis
void user_flash_write(flash_control_t*);
void flash_init(flash_control_t*);

//flash write and erase operations
void write_page(uint32_t,uint16_t);
void erase_page(uint32_t);
void erase_mass(void);
uint16_t read_flash(uint32_t);


#endif /* INC_FLASH_H_ */











