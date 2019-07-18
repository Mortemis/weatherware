/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "tim.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId buzzerTaskHandle;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartBuzzerTask(void const * argument);
void StartAlarmTask(void const * argument);
void StartBlinkTask(void const * argument);
void init_buzz(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);


void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  osThreadDef(blinkTask, StartBlinkTask, osPriorityNormal, 0, 128);
  osThreadCreate(osThread(blinkTask), NULL);

  init_buzz();
  //osThreadDef(alarmTask, StartAlarmTask, osPriorityNormal, 0, 128);
  //osThreadCreate(osThread(alarmTask), NULL);
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{

  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
	uint8_t buzz_flag = 0;

void StartBlinkTask(void const * argument) {
	while (1) {
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
		osDelay(200);
	}
}
void StartBuzzerTask(void const * argument) {
	while (1) {
		while (buzz_flag == 0) {
		}

		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
		uint32_t period = 10000;
		uint32_t step = -100;
		TIM4->CCR1 = 300;
		uint32_t counter = 0;
		while (1) {
			TIM4->ARR = period;
			period += step;
			//if (period >= 10000) step = -100;
			if (period <= 6000) {
				if (counter > 3)
					break;
				counter++;
				period = 10000;  //step = 100;
			}
			osDelay(6);
		}

		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
		buzz_flag=0;
	}
	//vTaskDelete(NULL);
}

void StartAlarmTask(void const * argument) {
	osThreadDef(buzzerTask, StartBuzzerTask, osPriorityNormal, 0, 128);
	//uint8_t ip[4]={10,10,10,180};
	//static uint16_t port=47796;
	while (1) {
		osDelay(5000);
		buzzerTaskHandle = osThreadCreate(osThread(buzzerTask), NULL);
	}
	vTaskDelete(NULL);
}

void init_buzz(void) {
	osThreadDef(buzzerTask, StartBuzzerTask, osPriorityNormal, 0, 128);
	buzzerTaskHandle = osThreadCreate(osThread(buzzerTask), NULL);
}



/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
