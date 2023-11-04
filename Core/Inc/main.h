/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FAN_HOTEND_Pin GPIO_PIN_14
#define FAN_HOTEND_GPIO_Port GPIOC
#define FAN_PARTCOOLING_Pin GPIO_PIN_15
#define FAN_PARTCOOLING_GPIO_Port GPIOC
#define EN_Motor_Pin GPIO_PIN_1
#define EN_Motor_GPIO_Port GPIOC
#define STEP_X_Pin GPIO_PIN_2
#define STEP_X_GPIO_Port GPIOC
#define DIR_X_Pin GPIO_PIN_3
#define DIR_X_GPIO_Port GPIOC
#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOA
#define SD_CLK_Pin GPIO_PIN_5
#define SD_CLK_GPIO_Port GPIOA
#define SD_MISO_Pin GPIO_PIN_6
#define SD_MISO_GPIO_Port GPIOA
#define SD_MOSI_Pin GPIO_PIN_7
#define SD_MOSI_GPIO_Port GPIOA
#define HOTEND_Pin GPIO_PIN_5
#define HOTEND_GPIO_Port GPIOC
#define STEP_E_Pin GPIO_PIN_0
#define STEP_E_GPIO_Port GPIOB
#define DIR_E_Pin GPIO_PIN_1
#define DIR_E_GPIO_Port GPIOB
#define LIMIT_Z_Pin GPIO_PIN_6
#define LIMIT_Z_GPIO_Port GPIOC
#define LIMIT_Y_Pin GPIO_PIN_7
#define LIMIT_Y_GPIO_Port GPIOC
#define LIMIT_X_Pin GPIO_PIN_8
#define LIMIT_X_GPIO_Port GPIOC
#define BED_Pin GPIO_PIN_8
#define BED_GPIO_Port GPIOA
#define DIR_Z_Pin GPIO_PIN_15
#define DIR_Z_GPIO_Port GPIOA
#define STEP_Z_Pin GPIO_PIN_3
#define STEP_Z_GPIO_Port GPIOB
#define STEP_Y_Pin GPIO_PIN_4
#define STEP_Y_GPIO_Port GPIOB
#define DIR_Y_Pin GPIO_PIN_5
#define DIR_Y_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
