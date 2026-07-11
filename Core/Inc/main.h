/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32wlxx_hal.h"

#include "stm32wlxx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
#define RTC_PREDIV_A ((1<<(15-RTC_N_PREDIV_S))-1)
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S ((1<<RTC_N_PREDIV_S)-1)
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define PUMP_CONTROL_Pin GPIO_PIN_5
#define PUMP_CONTROL_GPIO_Port GPIOB
#define Buzzer_Pin GPIO_PIN_2
#define Buzzer_GPIO_Port GPIOC
#define FE_CTRL3_Pin GPIO_PIN_3
#define FE_CTRL3_GPIO_Port GPIOC
#define FE_CTRL2_Pin GPIO_PIN_5
#define FE_CTRL2_GPIO_Port GPIOC
#define Deblock_Pin GPIO_PIN_9
#define Deblock_GPIO_Port GPIOA
#define KOMUNIKACE_Pin GPIO_PIN_12
#define KOMUNIKACE_GPIO_Port GPIOB
#define ERROR_LED_Pin GPIO_PIN_1
#define ERROR_LED_GPIO_Port GPIOC
#define FE_CTRL1_Pin GPIO_PIN_4
#define FE_CTRL1_GPIO_Port GPIOC
#define AUTOMATIKA_ON_Pin GPIO_PIN_7
#define AUTOMATIKA_ON_GPIO_Port GPIOA
#define CERPADLO_ON_Pin GPIO_PIN_4
#define CERPADLO_ON_GPIO_Port GPIOA
#define AUTOMATIKA_ON_LED_GPIO_Port GPIOA
#define AUTOMATIKA_ON_LED_Pin  GPIO_PIN_5


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
