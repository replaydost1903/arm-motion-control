/*
 * rs485.h
 *
 *  Created on: May 16, 2024
 *      Author: Kubilay
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_


#include "main.h"

//array length define function
#define _arraylength(_Array) (sizeof(_Array) / sizeof(_Array[0]))

/*Define Definitions*/
#define BUFFER_SIZE	10					/*!< Number of bytes to be sent to rs485 >!*/
#define POLYNOMIAL_VALUE 0x03FD			/*!< CRC Polynomial Value >!*/

/*Format Positions*/
#define HEADER_Pos		(0U)
#define COMMA1_Pos		(1U)
#define COMMAND_Pos		(2U)
#define COMMA2_Pos		(3U)
#define DATA1_Pos		(4U)
#define DATA2_Pos		(5U)
#define COMMA3_Pos		(6U)
#define TRAILER_Pos		(7U)
#define CRC_Pos_1		(8U)
#define CRC_Pos_2		(9U)

/*
 * rs485 crc control flag
 */
typedef enum
{
	CRC_ERROR,
	CRC_OK

}crc_check;

/*
 * rs485 communication control flag
 */
typedef enum
{
	FORMAT_ERROR,
	FORMAT_OK,
	FORMAT_BUSY

}rs485_flag;

/*
 * rs485 communication control state
 */
typedef enum
{
	NOT_CHECHKED,
	COMMAND_CHECHKED,
	DATA_CHECHKED,
	TRAILER_CHECHKED,
	CRC_CHECHKED

}rs485_state_t;

/*
 * rs485 main object
 */
typedef struct
{
	uint8_t ui8index;
	uint8_t ui8buffer[BUFFER_SIZE];
	uint16_t ui16crc;
	uint8_t receive_data;
	rs485_state_t	active_state;
	rs485_flag flag;
	crc_check crc;

}rs485_parser_t;


/*rs485 apis*/
void rs485_transmit_package(uint8_t*,uint16_t);
void rs485_receive_package(void);
void rs485_find_user_command(rs485_parser_t*);

/*state machine apis*/
void rs485_format_control(rs485_parser_t*);
void rs485_find_user_command();

#endif /* INC_RS485_H_ */





