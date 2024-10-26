/*
 * rs485.c
 *
 *  Created on: May 16, 2024
 *      Author: Kubilay
 */
#include "rs485.h"
#include "checksum.h"

uint8_t id_list[] = {LED_1_ID,LED_2_ID,BUZZER_ID,SPEAKER_ID,MOTOR_STOP_ID,MOTOR_MANUEL_ID,MOTOR_AUTO_ID,SYSTEM_LED_TEST_ID,SYSTEM_BUZZER_TEST_ID,SYSTEM_SPEAKER_TEST_ID,SYSTEM_MOTOR_TEST_ID,SYSTEM_START_TEST_ID,SYSTEM_STOP_TEST_ID};

extern UART_HandleTypeDef huart1;

rs485_parser_t mobj={0};
extern motor_app_t motor;

//helper function prototypes
static void rs485_state_handler_HEADER(rs485_parser_t*);
static void rs485_state_handler_COMMA(rs485_parser_t*);
static void rs485_state_handler_TRAILER(rs485_parser_t*);
static void rs485_state_handler_PACKET_CHECK(rs485_parser_t*);
static void rs485_crc_control(rs485_parser_t*);
static void mobj_erase(rs485_parser_t*);
static void mobj_flag_reset(rs485_parser_t*);

void rs485_transmit_package(uint8_t *ptr,uint16_t size)
{
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_SET);
	HAL_UART_Transmit(&huart1,(uint8_t*)ptr,size,2000);
	HAL_GPIO_WritePin(RS485_DIR_GPIO_Port, RS485_DIR_Pin, GPIO_PIN_RESET);
}
void rs485_receive_package(void)
{
	/*
	 *  Serial Arayüzden 1 byte geldiğinde UART_IRQHANDLERdan dönen 1 byte veriyi al
	 */
	HAL_UART_Receive_IT(&huart1,&mobj.receive_data, 1);

	/*
	 *  State machine ile parser kodunu uygula
	 */
	rs485_format_control(&mobj);

	/*
	 *  State machine uygulama bayrak kontrolü
	 */
	rs485_crc_control(&mobj);
}
void rs485_format_control(rs485_parser_t* mobj)
{
	switch(mobj->receive_data)
	{
	case '$':
		rs485_state_handler_HEADER(mobj);
		break;
	case ',':
		rs485_state_handler_COMMA(mobj);
		break;
	case '!':
		rs485_state_handler_TRAILER(mobj);
		break;
	default:
		rs485_state_handler_PACKET_CHECK(mobj);
		break;
	}
}
static void rs485_state_handler_HEADER(rs485_parser_t* mobj)
{
	if(mobj->ui8index >= 2)
	{
		mobj->active_state = DATA_CHECHKED;
		rs485_state_handler_PACKET_CHECK(mobj);
	}
	else
	{
		mobj->flag = FORMAT_BUSY;
		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;
	}
}
static void rs485_state_handler_COMMA(rs485_parser_t* mobj)
{
	mobj->flag = FORMAT_BUSY;

	if(mobj->ui8buffer[mobj->ui8index-1] == '$')
	{//uart paket formatındaki 1.virgül
		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		mobj->active_state = COMMAND_CHECHKED;
	}
	else if((mobj->ui8buffer[mobj->ui8index-3] == '$') & (mobj->ui8buffer[mobj->ui8index-2] == ','))
	{//uart paket formatındaki 2.virgül
		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		mobj->active_state = DATA_CHECHKED;
	}
	else if((mobj->ui8buffer[mobj->ui8index-6] == '$') & (mobj->ui8buffer[mobj->ui8index-5] == ',') & \
			(mobj->ui8buffer[mobj->ui8index-3] == ','))
	{//uart paket formatındaki 3.virgül
		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		mobj->active_state = TRAILER_CHECHKED;
	}
	else
	{
		mobj->flag = FORMAT_ERROR;
	}
}
static void rs485_state_handler_PACKET_CHECK(rs485_parser_t* mobj)
{
	if(mobj->active_state == COMMAND_CHECHKED)
	{
		mobj->flag = FORMAT_BUSY;

		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		mobj->active_state = NOT_CHECHKED;
	}
	else if(mobj->active_state == DATA_CHECHKED)
	{
		mobj->flag = FORMAT_BUSY;

		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		if(mobj->ui8index == 5)	//birden fazla $ işareti gelmesine karşı önlem (sorun = örneğin 6.elemanda $ olabilir)
		{
			mobj->active_state = DATA_CHECHKED;
		}
		else if(mobj->ui8index == 6)
		{
			mobj->active_state = NOT_CHECHKED;
		}
		else
		{
			mobj->flag = FORMAT_ERROR;
		}
	}
	else if(mobj->active_state == TRAILER_CHECHKED)
	{
		mobj->flag = FORMAT_BUSY;

		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		mobj->active_state = NOT_CHECHKED;
	}
	else if(mobj->active_state == CRC_CHECHKED)
	{
		mobj->flag = FORMAT_BUSY;

		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;

		if(mobj->ui8index == CRC_Pos_2 + 1)
		{
			mobj->flag = FORMAT_OK; //2 byte crc alımı tamamlandı paket formatı alımı bitti
		}
	}
	else if(mobj->active_state == NOT_CHECHKED) //Bu ifade hem ilk başta gelen sıfır için ana nesneyi sıfırlar
	{											//hem de ilk değer sayı olarak gelirse buraya girer
		mobj->flag = FORMAT_ERROR;
	}
}
static void rs485_state_handler_TRAILER(rs485_parser_t* mobj)
{
	if(mobj->active_state == TRAILER_CHECHKED)
	{
		mobj->ui8buffer[mobj->ui8index++] = mobj->receive_data;
		mobj->active_state = CRC_CHECHKED;
	}
	else
	{
		mobj->flag = FORMAT_ERROR;
	}
}
static void rs485_crc_control(rs485_parser_t* mobj)
{
	if(mobj->flag == FORMAT_OK)
	{
		mobj->ui16crc = crc_16((const unsigned char*)mobj->ui8buffer,BUFFER_SIZE-2);

		uint16_t crc_check = ((mobj->ui8buffer[8] << 8U) | mobj->ui8buffer[9]);

		if(crc_check == mobj->ui16crc)
		{
			mobj_flag_reset(mobj);
			rs485_find_user_command(mobj);
		}
		else
		{
			mobj_erase(mobj);
			rs485_transmit_package((uint8_t*)"CRC HATALI , BIT BOZULMASI VAR ORTAMDA GURULTU OLABILIR!!!\n",sizeof("CRC HATALI , BIT BOZULMASI VAR ORTAMDA GURULTU OLABILIR!!!\n"));
		}
	}
	else if(mobj->flag == FORMAT_ERROR)
	{
		mobj_erase(mobj);
		rs485_transmit_package((uint8_t*)"FORMAT HATALI CRC HATALI TEKRAR KOMUT GONDERIN !!!\n",sizeof("FORMAT HATALI CRC HATALI TEKRAR KOMUT GONDERIN !!!\n"));
	}
}
static void mobj_erase(rs485_parser_t* mobj)
{
	for(mobj->ui8index=0;mobj->ui8index<BUFFER_SIZE;mobj->ui8index++)
	{
		mobj->ui8buffer[mobj->ui8index]=0;
	}
	mobj->active_state=NOT_CHECHKED;
	mobj->receive_data=0;
	mobj->ui16crc=0;
	mobj->ui8index=0;
	mobj->crc = CRC_ERROR;
}
static void mobj_flag_reset(rs485_parser_t* mobj)
{
	mobj->active_state=NOT_CHECHKED;
	mobj->receive_data=0;
	mobj->ui16crc=0;
	mobj->ui8index=0;
	mobj->crc = CRC_OK;
}
void rs485_find_user_command(rs485_parser_t* mobj)
{
	//Id and data(1-2) find
	for(;mobj->ui8index<_arraylength(id_list);mobj->ui8index++)
	{
		if(id_list[mobj->ui8index] == mobj->ui8buffer[COMMAND_Pos])	//Find id
		{
			switch(id_list[mobj->ui8index])	//id bulundu
			{
			case(LED_1_ID):
				if((mobj->ui8buffer[DATA1_Pos] == LED_1_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == LED_1_ENABLE))
				{
					//Led1 Enable Status
					motor.led_1_flag = ENABLE;
					rs485_transmit_package((uint8_t*)"LED-1 ENABLE\n",sizeof("LED-1 ENABLE\n"));
				}
				else if((mobj->ui8buffer[DATA1_Pos] == LED_1_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == LED_1_DISABLE))
				{
					//Led1 Disable Status
					motor.led_1_flag = DISABLE;
					rs485_transmit_package((uint8_t*)"LED-1 DISABLE\n",sizeof("LED-1 DISABLE\n"));
				}
				break;
			case(LED_2_ID):
				if((mobj->ui8buffer[DATA1_Pos] == LED_2_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == LED_2_ENABLE))
				{
					//Led2 Enable Status
					motor.led_2_flag = ENABLE;
					rs485_transmit_package((uint8_t*)"LED-2 ENABLE\n",sizeof("LED-2 ENABLE\n"));
				}
				else if((mobj->ui8buffer[DATA1_Pos] == LED_2_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == LED_2_DISABLE))
				{
					//Led2 Disable Status
					motor.led_2_flag = DISABLE;
					rs485_transmit_package((uint8_t*)"LED-2 DISABLE\n",sizeof("LED-2 DISABLE\n"));
				}
				break;
			case(BUZZER_ID):
				if((mobj->ui8buffer[DATA1_Pos] == BUZZER_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == BUZZER_ENABLE))
				{
					//Buzzer Enable Status
					motor.buzzer_flag = ENABLE;
					rs485_transmit_package((uint8_t*)"BUZZER ENABLE\n",sizeof("BUZZER ENABLE\n"));
				}
				else if((mobj->ui8buffer[DATA1_Pos] == BUZZER_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == BUZZER_DISABLE))
				{
					//Buzzer Disable Status
					motor.buzzer_flag = DISABLE;
					rs485_transmit_package((uint8_t*)"BUZZER DISABLE\n",sizeof("BUZZER DISABLE\n"));
				}
				break;
			case(SPEAKER_ID):
				if((mobj->ui8buffer[DATA1_Pos] == SPEAKER_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == SPEAKER_ENABLE))
				{
					//Speaker Enable Status
					motor.speaker_flag = ENABLE;
					rs485_transmit_package((uint8_t*)"SPEAKER ENABLE\n",sizeof("SPEAKER ENABLE\n"));
				}
				else if((mobj->ui8buffer[DATA1_Pos] == SPEAKER_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == SPEAKER_DISABLE))
				{
					//Speaker Disable Status
					motor.speaker_flag = DISABLE;
					rs485_transmit_package((uint8_t*)"SPEAKER DISABLE\n",sizeof("SPEAKER DISABLE\n"));
				}
				break;
			case(MOTOR_STOP_ID):
				if((mobj->ui8buffer[DATA1_Pos] == MOTOR_STOP_DATA_CHECK) && (mobj->ui8buffer[DATA2_Pos] == MOTOR_STOP_ENABLE))
				{
					//Motor Stop Enable Status
					motor.motor_flag.manuel_mode= DISABLE;
					motor.motor_flag.stop_mode = ENABLE;
					motor.motor_flag.auto_mode = DISABLE;
					motor.motor_flag.backward = DISABLE;
					motor.motor_flag.forward = DISABLE;
					motor.motor_flag.speed = DISABLE;
					rs485_transmit_package((uint8_t*)"MOTOR STOP\n",sizeof("MOTOR STOP\n"));
				}
				break;
			case (MOTOR_MANUEL_ID):
					//Motor Manuel Mod Enable Status
					motor.motor_flag.manuel_mode= ENABLE;
					motor.motor_flag.stop_mode = DISABLE;
					motor.motor_flag.auto_mode = DISABLE;
					motor.motor_flag.backward = MOTOR_BACKWARD_POS(mobj->ui8buffer[DATA2_Pos]);
					motor.motor_flag.forward = MOTOR_FORWARD_POS(mobj->ui8buffer[DATA2_Pos]);
					motor.motor_flag.speed = mobj->ui8buffer[DATA1_Pos];
					rs485_transmit_package((uint8_t*)"MOTOR MANUEL\n",sizeof("MOTOR MANUEL\n"));
					break;
			case (MOTOR_AUTO_ID):
					//Motor Auto Mod Enable Status
					motor.motor_flag.manuel_mode= DISABLE;
					motor.motor_flag.stop_mode = DISABLE;
					motor.motor_flag.auto_mode = ENABLE;
					motor.motor_flag.backward = MOTOR_BACKWARD_POS(mobj->ui8buffer[DATA2_Pos]);
					motor.motor_flag.forward = MOTOR_FORWARD_POS(mobj->ui8buffer[DATA2_Pos]);
					motor.motor_flag.speed = mobj->ui8buffer[DATA1_Pos];
					rs485_transmit_package((uint8_t*)"MOTOR AUTO\n",sizeof("MOTOR AUTO\n"));
					break;
			case (SYSTEM_LED_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_LED_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_LED_TEST_DATA_2))
					{
						motor.test.led_test_flag = ENABLE;
					}
					break;
			case (SYSTEM_BUZZER_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_BUZZER_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_BUZZER_TEST_DATA_2))
					{
						motor.test.buzzer_test_flag = ENABLE;
					}
					break;
			case (SYSTEM_SPEAKER_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_SPEAKER_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_SPEAKER_TEST_DATA_2))
					{
						motor.test.speaker_test_flag = ENABLE;
					}
					break;
			case (SYSTEM_MOTOR_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_MOTOR_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_MOTOR_TEST_DATA_2))
					{
						motor.test.motor_test_flag = ENABLE;
					}
					break;
			case (SYSTEM_START_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_START_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_START_TEST_DATA_2))
					{
						motor.test.test_status = ENABLE;
					}
					break;
			case (SYSTEM_STOP_TEST_ID):
					if((mobj->ui8buffer[DATA1_Pos] == SYSTEM_STOP_TEST_DATA_1) && (mobj->ui8buffer[DATA2_Pos] == SYSTEM_STOP_TEST_DATA_2))
					{
						motor.test.test_status = DISABLE;
					}
					break;
			default:
				break;
			}
			break;
		}
	}
	mobj->ui8index=0;
}

























