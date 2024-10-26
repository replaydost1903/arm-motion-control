/*
 * main_app.c
 *
 *  Created on: Jul 15, 2024
 *      Author: Kubilay
 */
#include "main_app.h"


extern UART_HandleTypeDef huart1;
extern rs485_parser_t mobj;
extern flash_control_t fobj;
motor_app_t motor = {0};
_Bool pir_read;

//Speed and Delay Definitions
#define MOTOR_SPEED_MAX_DELAY	(10000U)

//system test functions
static void system_test_led(void);
static void system_test_buzzer(void);
static void system_test_speaker(void);
static void system_test_motor(void);
static void motor_led_1_process(void);
static void motor_led_2_process(void);
static void motor_buzzer_process(void);
static void motor_speaker_process(void);
static void motor_turn_process(void);


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	pir_read = HAL_GPIO_ReadPin(DIGITAL_IN1_GPIO_Port, DIGITAL_IN1_Pin);	//0 ise ve gelmişse 1-0 geçmiştir düşen kenar
																			//1 ise ve gelmişse 0-1 geçmiştir yükselen kenar
	if((GPIO_Pin == DIGITAL_IN1_Pin)&(pir_read == GPIO_PIN_SET))
	{
		pir_read = true;
	}
	else if((GPIO_Pin == DIGITAL_IN1_Pin)&(pir_read == GPIO_PIN_RESET))
	{
		pir_read = false;
	}
}

void motor_motion_init(void)
{
	rs485_receive_package();

	flash_init(&fobj);	//Bu fonksiyonda flasha yazılan bayrakların değerleri motor objesine atanacak
}

/*
 * main app file function
 */
void motor_motion_control(void)
{
	if(motor.test.test_status == ENABLE) //Sistem testi başladı , ana fonksiyonlar devre dışı
	{
		//Sistem testinden önce bütün alt sistemler sıfırlanıyor
		motor_motion_reset();	//Arayüzde start ve stop butonuna tıklandığında sistemin buton renklerinin default durumuna getirilicek
		if(motor.test.led_test_flag)
		{
			system_test_led();
		}
		else if(motor.test.buzzer_test_flag)
		{
			system_test_buzzer();
		}
		else if(motor.test.speaker_test_flag)
		{
			system_test_speaker();
		}
		else if(motor.test.motor_test_flag)
		{
			system_test_motor();
		}
		else
		{
			//TODO
		}
	}
	else if(motor.test.test_status == DISABLE) //Sistem testi durduysa test yapılmıyorsa ana fonksiyonları çalıştır
	{
		if(motor.motor_flag.auto_mode == ENABLE)
		{
			user_flash_write(&fobj); //motor modunun auto ya güncellenmesi

			motor_motion_process();	//hareket algılanana kadar diğer işlemler halledilsin

			if(pir_read == true) //Eğer hareket alınırsa motor auto modda stop komutu gelene kadar çalışır
			{
				rs485_transmit_package((uint8_t*)"MOTION DETECTED, AUTO MODE STARTING\n",sizeof("MOTION DETECTED, AUTO MODE STARTING\n"));

				while(motor.motor_flag.stop_mode != ENABLE)	///Arayüzden stop komutu gelene kadar çalışır
				{
					motor_turn_process(); //Motor auto modda çalışır iken diğer fonksiyonları bu fonk gerçekleştirir

					if(motor.test.test_status == ENABLE)	//Eğer motor auto modda çalışır iken test butonuna basılır ise buradan çıkar testi gerçekleştirir ve tekrar çalışmaya devam eder
					{
						motor_motion_reset();
						break;
					}
				}
	    		user_flash_write(&fobj); // motor auto moddan stop moduna güncellenmesi
			}
		}
		else if(motor.motor_flag.manuel_mode == ENABLE)
		{
			user_flash_write(&fobj); //motor modunun manuele güncellenmesi

			motor_motion_process();	//hareket algılanana kadar diğer işlemler halledilsin

			if(pir_read == true) //Eğer hareket alınırsa motor manuel modda 1 kere çalışır sonra tekrar komut bekler
			{
				rs485_transmit_package((uint8_t*)"MOTION DETECTED, MANUEL MODE STARTING\n",sizeof("MOTION DETECTED, MANUEL MODE STARTING\n"));

				motor_turn_process();
			}
		}
		else
		{
			user_flash_write(&fobj); //motor fonksiyonları harici işlemler burada yapılır

			motor_motion_process();	//Motor harici diğer fonksiyonlar burada yapılır
		}
	}
}

void motor_motion_reset(void)
{
	HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);
	HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 1);
	HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 1);
	HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
	HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
}

//System test functions
static void system_test_led(void)
{
	for(uint8_t i=0;i<10;i++)
	{
		if(motor.test.test_status == DISABLE)
		{
			HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 1);
			HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 1);
			break;
		}
		else
		{
			HAL_GPIO_TogglePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin);
			HAL_GPIO_TogglePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin);
			HAL_Delay(250);
		}
	}

	if(motor.test.test_status == DISABLE)
	{
		HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 1);
		HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 1);
		motor.test.led_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"LED TEST STOPPED\n",sizeof("LED TEST STOPPED\n"));
	}
	else
	{
		HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 1);
		HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 1);
		motor.test.test_status = DISABLE;
		motor.test.led_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"LED TEST SUCCESSFUL\n",sizeof("LED TEST SUCCESSFUL\n"));
	}
}
static void system_test_buzzer(void)
{
	for(uint8_t i=0;i<5;i++)
	{
		if(motor.test.test_status == DISABLE)
		{
			HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
			break;
		}
		else
		{
			HAL_GPIO_TogglePin(BUZZER_CTL_GPIO_Port	, BUZZER_CTL_Pin);
			HAL_Delay(300);
		}
	}

	if(motor.test.test_status == DISABLE)
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
		motor.test.buzzer_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"BUZZER TEST STOPPED\n",sizeof("BUZZER TEST STOPPED\n"));
	}
	else
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
		motor.test.test_status = DISABLE;
		motor.test.buzzer_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"BUZZER TEST SUCCESSFUL\n",sizeof("BUZZER TEST SUCCESSFUL\n"));
	}
}
static void system_test_speaker(void)
{
    uint32_t start_time = HAL_GetTick();
    uint32_t delay = 2000;

    HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);

    while((HAL_GetTick() - start_time) < delay)
    {
    	if(motor.test.test_status == DISABLE)
    	{
    		HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);
    		break;
    	}
    }

    start_time = HAL_GetTick();
    delay = 10000;
    HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 0);

    while((HAL_GetTick() - start_time) < delay)
    {
    	if(motor.test.test_status == DISABLE)
    	{
    		HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);
    		break;
    	}
    }

	if(motor.test.test_status == DISABLE)
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
		motor.test.speaker_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"SPEAKER TEST STOPPED\n",sizeof("SPEAKER TEST STOPPED\n"));
	}
	else
	{
	    HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);
		motor.test.test_status = DISABLE;
		motor.test.speaker_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"SPEAKER TEST SUCCESSFUL\n",sizeof("SPEAKER TEST SUCCESSFUL\n"));
	}
}
static void system_test_motor(void)
{
    uint32_t start_time = HAL_GetTick();
    uint32_t delay = 5000;

	HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	HAL_Delay(1);
	HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 1);

    while((HAL_GetTick() - start_time) < delay)
    {
    	if(motor.test.test_status == DISABLE)
    	{
    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
    		break;
    	}
    }

    start_time = HAL_GetTick();

	HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	HAL_Delay(1);
	HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 1);

    while((HAL_GetTick() - start_time) < delay)
    {
    	if(motor.test.test_status == DISABLE)
    	{
    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
    		break;
    	}
    }

	if(motor.test.test_status == DISABLE)
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
		motor.test.motor_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"MOTOR TEST STOPPED\n",sizeof("MOTOR TEST STOPPED\n"));
	}
	else
	{
		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
		motor.test.test_status = DISABLE;
		motor.test.motor_test_flag = DISABLE;
		rs485_transmit_package((uint8_t*)"MOTOR TEST SUCCESSFUL\n",sizeof("MOTOR TEST SUCCESSFUL\n"));
	}
}
static void motor_led_1_process(void)
{
	if(motor.led_1_flag == ENABLE)
	{
		HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 0);
	}
	else if(motor.led_1_flag == DISABLE)
	{
		HAL_GPIO_WritePin(LED_OUT_1_GPIO_Port, LED_OUT_1_Pin, 1);
	}
}
static void motor_led_2_process(void)
{
	if(motor.led_2_flag == ENABLE)
	{
		HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 0);
	}
	else if(motor.led_2_flag == DISABLE)
	{
		HAL_GPIO_WritePin(LED_OUT_2_GPIO_Port, LED_OUT_2_Pin, 1);
	}
}
static void motor_buzzer_process(void)
{
	if(motor.buzzer_flag == ENABLE)
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 1);
	}
	else if(motor.buzzer_flag == DISABLE)
	{
		HAL_GPIO_WritePin(BUZZER_CTL_GPIO_Port, BUZZER_CTL_Pin, 0);
	}
}
static void motor_speaker_process(void)
{
	if(motor.speaker_flag == ENABLE)
	{
		HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 0);
	}
	else if(motor.speaker_flag == DISABLE)
	{
		HAL_GPIO_WritePin(DFPLAYER_POWER_EN_GPIO_Port, DFPLAYER_POWER_EN_Pin, 1);
	}
}

void motor_motion_process(void)
{
	motor_led_1_process();
	motor_led_2_process();
	motor_buzzer_process();
	motor_speaker_process();
}

static void motor_turn_process(void)
{
	if(motor.motor_flag.auto_mode == ENABLE)
	{
		//H köprüsü resetleniyor
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);

		//Delay ayarları için değişken tanımlandı
	    uint32_t start_time;
	    float delay = (float)(MOTOR_SPEED_MAX_DELAY - (39.2156 * motor.motor_flag.speed)); //Bu değer min 0 max 255 olabilir

	    //ileri yönde dönüş
		for(uint8_t turn=0;turn<motor.motor_flag.forward;turn++)
		{
			if(motor.test.test_status == ENABLE)
			{
				motor_motion_reset();
				break;
			}

			if(motor.motor_flag.stop_mode == ENABLE)
			{
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
				user_flash_write(&fobj); //motorun auto moddan stop moduna güncellenmesi
	    		break;
			}
			else if(motor.motor_flag.stop_mode == DISABLE)
			{
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_Delay(1);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 1);

				start_time = HAL_GetTick();

			    while((HAL_GetTick() - start_time) < delay)
			    {
			    	if(motor.motor_flag.stop_mode == ENABLE)
			    	{
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
						user_flash_write(&fobj); //motorun auto moddan stop moduna güncellenmesi
			    		break;
			    	}
			    	else if(motor.test.test_status == ENABLE)
			    	{
			    		motor_motion_reset();
			    		break;
			    	}
			    	else
			    	{
			    		motor_motion_process();
			    		user_flash_write(&fobj); // motor auto modda iken herhangi bir komut gelirse güncellemek için
			    	}
			    }
			 }
		}

		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);

		//geri yönde dönüş
		for(uint8_t turn=0;turn<motor.motor_flag.backward;turn++)
		{
			if(motor.test.test_status == ENABLE)
			{
				motor_motion_reset();
				break;
			}

			if(motor.motor_flag.stop_mode == ENABLE)
			{
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
				user_flash_write(&fobj); //motorun auto moddan stop moduna güncellenmesi
	    		break;
			}
			else if(motor.motor_flag.stop_mode == DISABLE)
			{
				start_time = HAL_GetTick();
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	    		HAL_Delay(1);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 1);

			    while((HAL_GetTick() - start_time) < delay)
			    {
			    	if(motor.motor_flag.stop_mode == ENABLE)
			    	{
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
						user_flash_write(&fobj); //motorun auto moddan stop moduna güncellenmesi
			    		break;
			    	}
			    	else if(motor.test.test_status == ENABLE)
			    	{
			    		motor_motion_reset();
			    		break;
			    	}
			    	else
			    	{
			    		motor_motion_process(); //motor otomatik moddayken de eğer başka bir kullanıcı komutu gelirse onu çalıştırır
			    		user_flash_write(&fobj); // motor auto modda iken herhangi bir komut gelirse güncellemek için
			    	}
			    }
			}
		}

		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	}
	else if(motor.motor_flag.manuel_mode == ENABLE)
	{
		//H köprüsü resetleniyor
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);

		//Delay ayarları için değişken tanımlandı
	    uint32_t start_time;
	    float delay = (float)(MOTOR_SPEED_MAX_DELAY - (39.2156 * motor.motor_flag.speed)); //Bu değer min 0 max 255 olabilir

	    //ileri yönde dönüş
		for(uint8_t turn=0;turn<motor.motor_flag.forward;turn++)
		{
			if(motor.test.test_status == ENABLE)
			{
				motor_motion_reset();
				break;
			}

			if(motor.motor_flag.stop_mode == ENABLE)
			{
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
				user_flash_write(&fobj); //motorun manuel moddan stop moduna güncellenmesi
	    		break;
			}
			else if(motor.motor_flag.stop_mode == DISABLE)
			{
				start_time = HAL_GetTick();
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_Delay(1);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 1);

			    while((HAL_GetTick() - start_time) < delay)
			    {
			    	if(motor.motor_flag.stop_mode == ENABLE)
			    	{
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
						user_flash_write(&fobj); //motorun manuel moddan stop moduna güncellenmesi
			    		break;
			    	}
			    	else if(motor.test.test_status == ENABLE)
			    	{
			    		motor_motion_reset();
			    		break;
			    	}
			    	else
			    	{
			    		motor_motion_process();
			    		user_flash_write(&fobj); // motor auto modda iken herhangi bir komut gelirse güncellemek için
			    	}
			    }
			 }
		}

		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);

		//geri yönde dönüş
		for(uint8_t turn=0;turn<motor.motor_flag.backward;turn++)
		{
			if(motor.test.test_status == ENABLE)
			{
				motor_motion_reset();
				break;
			}

			if(motor.motor_flag.stop_mode == ENABLE)
			{
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
				user_flash_write(&fobj); //motorun manuel moddan stop moduna güncellenmesi
	    		break;
			}
			else if(motor.motor_flag.stop_mode == DISABLE)
			{
				start_time = HAL_GetTick();
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	    		HAL_Delay(1);
	    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 1);

			    while((HAL_GetTick() - start_time) < delay)
			    {
			    	if(motor.motor_flag.stop_mode == ENABLE)
			    	{
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);
			    		HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
						user_flash_write(&fobj); //motorun manuel moddan stop moduna güncellenmesi
			    		break;
			    	}
			    	else if(motor.test.test_status == ENABLE)
			    	{
			    		motor_motion_reset();
			    		break;
			    	}
			    	else
			    	{
			    		motor_motion_process(); //motor manuel moddayken de eğer başka bir kullanıcı komutu gelirse onu çalıştırır
			    		user_flash_write(&fobj); // motor manuel modda iken herhangi bir komut gelirse güncellemek için
			    	}
			    }
			}
		}

	HAL_GPIO_WritePin(MOTOR_DRIVE_2_GPIO_Port, MOTOR_DRIVE_2_Pin, 0);		//Manuel mod bitince motor sıfırlanmalıdır
	HAL_GPIO_WritePin(MOTOR_DRIVE_1_GPIO_Port, MOTOR_DRIVE_1_Pin, 0);
	//motor.motor_flag.manuel_mode = DISABLE;	//Kullanıcı motoru manuel modda çalıştırmak için 1 kere daha komut göndermelidir
  }
}








