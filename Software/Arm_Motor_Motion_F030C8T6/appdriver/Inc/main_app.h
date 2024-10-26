/*
 * main_app.h
 *
 *  Created on: Jul 15, 2024
 *      Author: Kubilay
 */

#ifndef INC_MAIN_APP_H_
#define INC_MAIN_APP_H_

#include "main.h"

	  /*****************************************
	   * 		  MOTOR KONTROL KARTI		   *
	   *****************************************
	   *      Girişler   	|	  Çıkışlar	   *
	   *					|				   *
	   *   Hareket sensörü	|	  Speaker	   *
	   *   RS-485			|     Motor		   *
	   *					|	  Led_1		   *
	   *					|	  Led_2		   *
	   *					|	  Buzzer	   *
	   *****************************************

		===========				===========
		Alt Sistemler			Komut Listesi
		===========				===========

		Hoparlör				ON/OFF (?)
		Led_1					ON/OFF (?)
		Led_2					ON/OFF (?)
		Buzzer					ON/OFF (?)
		Pyro Sensörü			Time_Delay Adjust - Distance setting (?)
		Motor			        MANUEL/AUTO/STOP (?)
*/






/*
 * motor motion board id and data definitions
 */
#define LED_1_ID				(0x04U)
#define LED_2_ID				(0x05U)
#define BUZZER_ID				(0x06U)
#define SPEAKER_ID				(0x01U)
#define MOTOR_STOP_ID			(0x07U)
#define MOTOR_MANUEL_ID			(0x3DU)
#define MOTOR_AUTO_ID			(0x4FU)
#define SYSTEM_LED_TEST_ID		(0x08U)
#define SYSTEM_BUZZER_TEST_ID	(0x09U)
#define SYSTEM_SPEAKER_TEST_ID	(0x0AU)
#define SYSTEM_MOTOR_TEST_ID	(0x0BU)
#define SYSTEM_START_TEST_ID	(0x20U)
#define SYSTEM_STOP_TEST_ID		(0x22U)

#define LED_1_DATA_CHECK		(0x0BU)
#define LED_1_ENABLE			(ENABLE)
#define LED_1_DISABLE			(DISABLE)

#define LED_2_DATA_CHECK		(0x0CU)
#define LED_2_ENABLE			(ENABLE)
#define LED_2_DISABLE			(DISABLE)

#define BUZZER_DATA_CHECK		(0x0DU)
#define BUZZER_ENABLE			(ENABLE)
#define BUZZER_DISABLE			(DISABLE)

#define SPEAKER_DATA_CHECK		(0x0AU)
#define SPEAKER_ENABLE			(ENABLE)
#define SPEAKER_DISABLE			(DISABLE)

#define MOTOR_STOP_ENABLE		(ENABLE)
#define MOTOR_STOP_DATA_CHECK	(0x0DU)

#define MOTOR_FORWARD_POS(x)	(((uint8_t)((x & 0xF0U)))>> 4U)
#define MOTOR_BACKWARD_POS(x)	((uint8_t)(x & 0x0FU))

#define SYSTEM_LED_TEST_DATA_1		(0x10U)
#define SYSTEM_LED_TEST_DATA_2		(0x01U)
#define SYSTEM_BUZZER_TEST_DATA_1	(0x11U)
#define SYSTEM_BUZZER_TEST_DATA_2	(0x02U)
#define SYSTEM_SPEAKER_TEST_DATA_1	(0x12U)
#define SYSTEM_SPEAKER_TEST_DATA_2	(0x03U)
#define SYSTEM_MOTOR_TEST_DATA_1	(0x13U)
#define SYSTEM_MOTOR_TEST_DATA_2	(0x04U)
#define SYSTEM_START_TEST_DATA_1	(0x2EU)
#define SYSTEM_START_TEST_DATA_2	(0x25U)
#define SYSTEM_STOP_TEST_DATA_1		(0x2FU)
#define SYSTEM_STOP_TEST_DATA_2		(0x26U)

#define ENABLE			(1U)
#define DISABLE			(!ENABLE)

typedef struct
{
	uint8_t led_test_flag;
	uint8_t buzzer_test_flag;
	uint8_t speaker_test_flag;
	uint8_t motor_test_flag;
	uint8_t test_status;

}system_test_t;

/*
 * motor id structure
 */
typedef struct
{
	uint8_t auto_mode;
	uint8_t manuel_mode;
	uint8_t forward;
	uint8_t backward;
	uint8_t stop_mode;
	uint8_t speed;

}motor_t;



/*
 * motor motion main structure
 */
typedef struct
{
	uint8_t led_1_flag;
	uint8_t led_2_flag;
	uint8_t buzzer_flag;
	uint8_t speaker_flag;
	motor_t motor_flag;
	uint16_t crc;
	system_test_t test;

}motor_app_t;




/*
 * motor motion apis
 */
void motor_motion_init(void);
void motor_motion_control(void);
void motor_motion_process(void);
void motor_motion_reset(void);




#endif /* INC_MAIN_APP_H_ */
