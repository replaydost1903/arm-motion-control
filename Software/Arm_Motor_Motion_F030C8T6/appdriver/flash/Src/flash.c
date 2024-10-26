/*
 * flash.c
 *
 *  Created on: Jul 11, 2024
 *      Author: Kubilay
 */

#include "flash.h"
#include "main_app.h"


_Bool flash_guard = true;
extern rs485_parser_t mobj;
extern motor_app_t motor;
flash_control_t fobj={0};
motor_app_t motor_cpy = {0};
_Bool result;


static void write_operations(flash_control_t *);
static _Bool two_struct_compare(void);
static void copy_struct(void);

void write_page(uint32_t addr,uint16_t data)
{

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	 /* For robust implementation, add here time-out management */
	}

	if ((FLASH->CR & FLASH_CR_LOCK) != 0) /* (2) */
	{
		FLASH->KEYR = 0x45670123; /* (3) */
		FLASH->KEYR = 0xCDEF89AB;
	}

	FLASH->CR |= FLASH_CR_PG; /* (1) */

	*(__IO uint16_t*)(addr) = data; /* (2) */

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
	{
	/* For robust implementation, add here time-out management */
	}

	if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (4) */
	{
		FLASH->SR = FLASH_SR_EOP; /* (5) */
	}
	else
	{
	 /* Manage the error cases */
	}

	FLASH->CR &= ~FLASH_CR_PG; /* (6) */

}

void erase_page(uint32_t page_addr)
{

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	 /* For robust implementation, add here time-out management */
	}

	if ((FLASH->CR & FLASH_CR_LOCK) != 0) /* (2) */
	{
		FLASH->KEYR = 0x45670123; /* (3) */
		FLASH->KEYR = 0xCDEF89AB;
	}

	FLASH->CR |= FLASH_CR_PER; /* (1) */

	FLASH->AR = page_addr; /* (2) */

	FLASH->CR |= FLASH_CR_STRT; /* (3) */

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (4) */
	{
	/* For robust implementation, add here time-out management */
	}

	if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (5) */
	{
		FLASH->SR = FLASH_SR_EOP; /* (6)*/
	}
	else
	{
		/* Manage the error cases */
	}

	FLASH->CR &= ~FLASH_CR_PER; /* (7) */

}

void erase_mass(void)
{

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (1) */
	{
	 /* For robust implementation, add here time-out management */
	}

	if ((FLASH->CR & FLASH_CR_LOCK) != 0) /* (2) */
	{
		FLASH->KEYR = 0x45670123; /* (3) */
		FLASH->KEYR = 0xCDEF89AB;
	}

	FLASH->CR |= FLASH_CR_MER; /* (1) */

	FLASH->CR |= FLASH_CR_STRT; /* (2) */

	while ((FLASH->SR & FLASH_SR_BSY) != 0) /* (3) */
	{
	 /* For robust implementation, add here time-out management */
	}

	if ((FLASH->SR & FLASH_SR_EOP) != 0) /* (4)*/
	{
		FLASH->SR = FLASH_SR_EOP; /* (5) */
	}
	else
	{
	 /* Manage the error cases */
	}

	FLASH->CR &= ~FLASH_CR_MER; /* (6) */
}

void flash_init(flash_control_t* fobj)
{

	uint16_t buff[5]={0};

	fobj->addr_index=0;
	fobj->write_addr=(uint32_t)Flash_address;

	for(uint8_t indx=0;indx < 5;indx++)
	{
		buff[indx] = read_flash(fobj->write_addr + indx*2);
	}

	fobj->crc_val = crc_16((const unsigned char*)buff,3);

	rs485_transmit_package((uint8_t*)"reading the values on the flash memory please wait 2 second ...\n",sizeof("reading the values on the flash memory please wait 2 second ...\n"));
	HAL_Delay(2000);

	if(fobj->crc_val == buff[4])
	{
		fobj->crc_val = 0;

		rs485_transmit_package((uint8_t*)"CRC VALUE CORRECT\n",sizeof("CRC VALUE CORRECT\n"));

		motor.led_1_flag   = (((buff[0]) & (0xFF00)) >> 8U);
		motor.led_2_flag   = ((buff[0]) & (0x00FF));
		motor.buzzer_flag  = (((buff[1]) & (0xFF00)) >> 8U);
		motor.speaker_flag = ((buff[1]) & (0x00FF));

		if(motor.led_1_flag == ENABLE)
		{
		  rs485_transmit_package((uint8_t*)"LED-1 STARTING\n",sizeof("LED-1 STARTING\n"));
		  HAL_Delay(200);
		}
		else
		{
		  rs485_transmit_package((uint8_t*)"LED-1 NOT STARTING\n",sizeof("LED-1 NOT STARTING\n"));
		  HAL_Delay(200);
		}

		if(motor.led_2_flag == ENABLE)
		{
		  rs485_transmit_package((uint8_t*)"LED-2 STARTING\n",sizeof("LED-2 STARTING\n"));
		  HAL_Delay(200);
		}
		else
		{
		  rs485_transmit_package((uint8_t*)"LED-2 NOT STARTING\n",sizeof("LED-2 NOT STARTING\n"));
		  HAL_Delay(200);
		}


		if(motor.buzzer_flag == ENABLE)
		{
		  rs485_transmit_package((uint8_t*)"BUZZER STARTING\n",sizeof("BUZZER STARTING\n"));
		  HAL_Delay(200);
		}
		else
		{
		  rs485_transmit_package((uint8_t*)"BUZZER NOT STARTING\n",sizeof("BUZZER NOT STARTING\n"));
		  HAL_Delay(200);
		}

		if(motor.speaker_flag == ENABLE)
		{
		  rs485_transmit_package((uint8_t*)"SPEAKER STARTING\n",sizeof("SPEAKER STARTING\n"));
		  HAL_Delay(200);
		}
		else
		{
		  rs485_transmit_package((uint8_t*)"SPEAKER NOT STARTING\n",sizeof("SPEAKER NOT STARTING\n"));
		  HAL_Delay(200);
		}

		if(((buff[3]) & (0x00FF)) == MOTOR_AUTO_ID)
		{
		  motor.motor_flag.stop_mode = DISABLE;
		  motor.motor_flag.manuel_mode = DISABLE;
		  motor.motor_flag.auto_mode = (((buff[2]) & (0xFF00)) >> 8U);
		  motor.motor_flag.forward = (((buff[2]) & (0x00F0)) >> 4U);
		  motor.motor_flag.backward = ((buff[2]) & (0x000F));
		  motor.motor_flag.speed = ((((buff[3]) & (0xFF00))) >> 8U);
		  rs485_transmit_package((uint8_t*)"AUTO MODE STARTING\n",sizeof("AUTO MODE STARTING\n"));
		}
		else if(((buff[3]) & (0x00FF)) == MOTOR_MANUEL_ID)
		{
		  motor.motor_flag.stop_mode = DISABLE;
		  motor.motor_flag.manuel_mode = (((buff[2]) & (0xFF00)) >> 8U);
		  motor.motor_flag.auto_mode = DISABLE;
		  motor.motor_flag.forward = (((buff[2]) & (0x00F0)) >> 4U);
		  motor.motor_flag.backward = ((buff[2]) & (0x000F));
		  motor.motor_flag.speed = (((buff[3]) & (0xFF00)) >> 8U);
		  rs485_transmit_package((uint8_t*)"MANUEL MODE STARTING\n",sizeof("MANUEL MODE STARTING\n"));
		}
		else if(((buff[3]) & (0x00FF)) == MOTOR_STOP_ID)
		{
		  motor.motor_flag.stop_mode = (((buff[2]) & (0xFF00)) >> 8U);
		  motor.motor_flag.manuel_mode = DISABLE;
		  motor.motor_flag.auto_mode = DISABLE;
		  motor.motor_flag.forward = (((buff[2]) & (0x00F0)) >> 4U);
		  motor.motor_flag.backward = ((buff[2]) & (0x000F));
		  motor.motor_flag.speed = (((buff[3]) & (0xFF00)) >> 8U);
		  rs485_transmit_package((uint8_t*)"STOP MODE STARTING\n",sizeof("STOP MODE STARTING\n"));
		}
	}
	else
	{
		fobj->crc_val = 0;

		rs485_transmit_package((uint8_t*)"CRC VALUE INCORRECT\n",sizeof("CRC VALUE INCORRECT\n"));
	}

}

/*
 * Flash hafızada seçilen page alanına , doğru formatta gelen paketlerin sırasıyla yazılmasını sağlar
 *
 * Dikkat edilmesi gerekenler
 * 	-page alanının temel adresi
 * 	-paketin boyutu ve uzunluğu (bir seferde flash adresine sadece 16bit kod yazılabilir)
 * 	-index boyutu ve uzunluğu
 * 	-max program boyutu
 * 	-flash birimine ait hata bayraklarının anlamları
 */

void user_flash_write(flash_control_t*  fobj)
{
	  if(flash_guard)
	  {
		  //Komut geldiğinde yazılmadan önce flash adresi siliniyor
		  erase_page(fobj->write_addr);
		  HAL_Delay(1);
		  write_operations((flash_control_t*)fobj);
		  copy_struct();
		  flash_guard = false;
	  }
	  else if(!flash_guard)
	  {
		  //Bu fonksiyon sadece komut değişikliğinde flash hafızaya yazılmasını sağlar ve flashın ömrünü uzatır
		  result = two_struct_compare();

		  copy_struct();

		  if(!result) //Komut değişikliği olduysa flasha yazar
		  {
			  //Komut geldiğinde yazılmadan önce flash adresi siliniyor
			  erase_page(fobj->write_addr);
			  HAL_Delay(1);
			  write_operations((flash_control_t*)fobj);
		  }
	  }
}

uint16_t read_flash(uint32_t  adr)
{
  uint16_t * Pntr = (uint16_t *)adr;
  return(*Pntr);
}

static _Bool two_struct_compare(void)
{
	if(motor.led_1_flag != motor_cpy.led_1_flag)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.led_2_flag != motor_cpy.led_2_flag)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.buzzer_flag != motor_cpy.buzzer_flag)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.speaker_flag != motor_cpy.speaker_flag)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.motor_flag.manuel_mode != motor_cpy.motor_flag.manuel_mode)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.motor_flag.auto_mode != motor_cpy.motor_flag.auto_mode)
	{
		return false; //iki struct değerleri farklı
	}
	else if(motor.motor_flag.stop_mode != motor_cpy.motor_flag.stop_mode)
	{
		return false; //iki struct değerleri farklı
	}
	else
	{
		return true; //iki struct değerleri aynı
	}
}

static void write_operations(flash_control_t *fobj)
{
	  //Her komut geldiğinde güncellenen değerler flasha burada yazılıyor
	  while(fobj->addr_index < 9)
	  {
		  switch(fobj->addr_index)
		  {													//Index değerleri düzeltilicek
		  case(0):
				  fobj->ui16_buffer[fobj->addr_index] = (motor.led_1_flag << 8U) | (motor.led_2_flag);
				  write_page(fobj->write_addr + fobj->addr_index , (motor.led_1_flag << 8U) | (motor.led_2_flag));
				  ++fobj->addr_index;
				  break;
		  case(2):
				  fobj->ui16_buffer[fobj->addr_index-1] = (motor.buzzer_flag << 8U) | (motor.speaker_flag);
				  write_page(fobj->write_addr + fobj->addr_index , (motor.buzzer_flag << 8U) | (motor.speaker_flag));
				  ++fobj->addr_index;
				  break;
		  case(4):
				  if(motor.motor_flag.auto_mode == ENABLE)
				  {
					  write_page(fobj->write_addr + fobj->addr_index , (motor.motor_flag.auto_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward));
					  ++fobj->addr_index;
					  ++fobj->addr_index;
					  write_page(fobj->write_addr + fobj->addr_index ,((motor.motor_flag.speed << 8U) | MOTOR_AUTO_ID)); //auto mode olduğunu anlamak için MOTOR_AUTO_ID orladık
					  ++fobj->addr_index;
					  fobj->ui16_buffer[fobj->addr_index-5] = (motor.motor_flag.auto_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward);
					  fobj->ui16_buffer[fobj->addr_index-4] = ((motor.motor_flag.speed << 8U) | MOTOR_AUTO_ID);
				  }
				  else if(motor.motor_flag.manuel_mode == ENABLE)
				  {
					  write_page(fobj->write_addr + fobj->addr_index , (motor.motor_flag.manuel_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward));
					  ++fobj->addr_index;
					  ++fobj->addr_index;
					  write_page(fobj->write_addr + fobj->addr_index , ((motor.motor_flag.speed << 8U) | MOTOR_MANUEL_ID)); //manuel mode olduğunu anlamak için MOTOR_MANUEL_ID orladık
					  ++fobj->addr_index;
					  fobj->ui16_buffer[fobj->addr_index-5] = (motor.motor_flag.manuel_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward);
					  fobj->ui16_buffer[fobj->addr_index-4] = ((motor.motor_flag.speed << 8U) | MOTOR_MANUEL_ID);
				  }
				  else if(motor.motor_flag.stop_mode == ENABLE)
				  {
					  write_page(fobj->write_addr + fobj->addr_index , (motor.motor_flag.stop_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward));
					  ++fobj->addr_index;
					  ++fobj->addr_index;
					  write_page(fobj->write_addr + fobj->addr_index , ((motor.motor_flag.speed << 8U) | MOTOR_STOP_ID)); //stop mode olduğunu anlamak için MOTOR_STOP_ID orladık
					  ++fobj->addr_index;
					  fobj->ui16_buffer[fobj->addr_index-5] = (motor.motor_flag.stop_mode << 8U) | ((motor.motor_flag.forward << 4U) | motor.motor_flag.backward);
					  fobj->ui16_buffer[fobj->addr_index-4] = ((motor.motor_flag.speed << 8U) | MOTOR_STOP_ID);
				  }
				  else
				  {
					  write_page(fobj->write_addr + fobj->addr_index , 0x0000);
					  ++fobj->addr_index;
					  ++fobj->addr_index;
					  write_page(fobj->write_addr + fobj->addr_index , 0x0000); //herhangi bir motor komutu gelmedi
					  ++fobj->addr_index;
				  }
				 break;
		  case(8):
					  fobj->crc_val = crc_16((const unsigned char*)fobj->ui16_buffer,3);
					  fobj->ui16_buffer[fobj->addr_index-4] = fobj->crc_val;
					  write_page(fobj->write_addr + fobj->addr_index , fobj->crc_val);
		  default:
			  break;
		  }
		  ++fobj->addr_index;
	  }
	  fobj->addr_index=0;
}

static void copy_struct(void)
{
	  motor_cpy.led_1_flag = motor.led_1_flag;
	  motor_cpy.led_2_flag = motor.led_2_flag;
	  motor_cpy.buzzer_flag = motor.buzzer_flag;
	  motor_cpy.speaker_flag = motor.speaker_flag;
	  motor_cpy.motor_flag.manuel_mode = motor.motor_flag.manuel_mode;
	  motor_cpy.motor_flag.auto_mode = motor.motor_flag.auto_mode;
	  motor_cpy.motor_flag.stop_mode = motor.motor_flag.stop_mode;
	  motor_cpy.motor_flag.forward = motor.motor_flag.forward;
	  motor_cpy.motor_flag.backward = motor.motor_flag.backward;
	  motor_cpy.motor_flag.speed = motor.motor_flag.speed;
}


















