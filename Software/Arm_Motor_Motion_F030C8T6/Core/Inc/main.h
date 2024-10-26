/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "rs485.h"
#include "flash.h"
#include "main_app.h"
#include "checksum.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOF
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOF
#define DIGITAL_IN1_Pin GPIO_PIN_3
#define DIGITAL_IN1_GPIO_Port GPIOA
#define DIGITAL_IN1_EXTI_IRQn EXTI2_3_IRQn
#define DIGITAL_IN2_Pin GPIO_PIN_4
#define DIGITAL_IN2_GPIO_Port GPIOA
#define DIGITAL_IN2_EXTI_IRQn EXTI4_15_IRQn
#define MOTOR_DRIVE_1_Pin GPIO_PIN_12
#define MOTOR_DRIVE_1_GPIO_Port GPIOB
#define MOTOR_DRIVE_2_Pin GPIO_PIN_13
#define MOTOR_DRIVE_2_GPIO_Port GPIOB
#define BUZZER_CTL_Pin GPIO_PIN_14
#define BUZZER_CTL_GPIO_Port GPIOB
#define DFPLAYER_POWER_EN_Pin GPIO_PIN_15
#define DFPLAYER_POWER_EN_GPIO_Port GPIOB
#define RS485_TX_Pin GPIO_PIN_9
#define RS485_TX_GPIO_Port GPIOA
#define RS485_RX_Pin GPIO_PIN_10
#define RS485_RX_GPIO_Port GPIOA
#define RS485_DIR_Pin GPIO_PIN_11
#define RS485_DIR_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define LED_OUT_1_Pin GPIO_PIN_6
#define LED_OUT_1_GPIO_Port GPIOB
#define LED_OUT_2_Pin GPIO_PIN_7
#define LED_OUT_2_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
