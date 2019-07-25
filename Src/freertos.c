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
#include "stdlib.h"
#include "tim.h"
#include "adc.h"
#include "ds18b20/ds18b20.h"
#include "bmp280.h"
#include "i2c.h"
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


BMP280_HandleTypedef bmp280;
float pressure, temperature, humidity;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartBuzzerTask(void const * argument);
void StartAlarmTask(void const * argument);
void StartBlinkTask(void const * argument);
void StartADCTask(void const * argument);
void StartBMPTask(void const * argument);
void init_buzz(void);
void init_photores(void);
void init_alarm(void);
void init_bmp(void);
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

  Ds18b20_Init(osPriorityNormal);


  init_bmp();
  init_buzz();
  init_photores();
  init_alarm();
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
uint32_t photores_value = 2048;
uint8_t buzz_flag = 0;
uint8_t alarm_flag = 0;

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
				if (counter >= 2)
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

	while (1) {
		if (alarm_flag == 0) {
			HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
		}
		while (alarm_flag == 0) {
			osDelay(5);
		}
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
		while (alarm_flag == 1) {
			TIM4->CCR1 = 250;//value*16;//258;//value*1024;
			TIM4->ARR = photores_value*16;//65535-photores_value*16;
			osDelay(5);
		}
	}

}

void StartADCTask(void const * argument) {
	while (1) {
		HAL_ADC_Start(&hadc1);
		//HAL_ADC_PollForConversion(&hadc1, 100);
		photores_value = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Stop(&hadc1);
		osDelay(5);
	}
}

void StartBMPTask(void const * argument) {
	bmp280_init_default_params(&bmp280.params);
	bmp280.addr = BMP280_I2C_ADDRESS_0;
	bmp280.i2c = &hi2c1;
	uint16_t size;
	uint8_t Data[256];
	char bmp_data[256] = {0};
	while (!bmp280_init(&bmp280, &bmp280.params)) {
			size = sprintf((char *)Data, "BMP280 initialization failed\n");
			//HAL_UART_Transmit(&huart1, Data, size, 1000);
			osDelay(2000);
	}
	bool bme280p = bmp280.id == BME280_CHIP_ID;



	while (1) {

		osDelay(100);
		while (!bmp280_read_float(&bmp280, &temperature, &pressure, &humidity)) {
				//	size = sprintf((char *)Data,
				//			"Temperature/pressure reading failed\n");
					//HAL_UART_Transmit(&huart1, Data, size, 1000);
					//HAL_Delay(2000);
					osDelay(100);
				}
				//sprintf(bmp_data, "Pressure: %f Pa, Temperature: 2f C",
				//				pressure, temperature);

				//size = sprintf((char *)Data,"Pressure: %.2f Pa, Temperature: %.2f C",
				//		pressure, temperature);
				//HAL_UART_Transmit(&huart1, Data, size, 1000);
				if (bme280p) {
				//size = sprintf((char *)Data,", Humidity: %.2f\n", humidity);
					//HAL_UART_Transmit(&huart1, Data, size, 1000);
				}

				else {
				//	size = sprintf((char *)Data, "\n");
					//HAL_UART_Transmit(&huart1, Data, size, 1000);
				}

		osDelay(2000);
	}
}

void init_buzz(void) {
	osThreadDef(buzzerTask, StartBuzzerTask, osPriorityNormal, 0, 128);
	buzzerTaskHandle = osThreadCreate(osThread(buzzerTask), NULL);
}

void init_photores(void) {
	osThreadDef(ADCTask, StartADCTask, osPriorityNormal, 0, 128);
	osThreadCreate(osThread(ADCTask), NULL);
}

void init_alarm(void) {
	osThreadDef(alarmTask, StartAlarmTask, osPriorityNormal, 0, 128);
	osThreadCreate(osThread(alarmTask), NULL);
}

void init_bmp(void) {
	osThreadDef(bmpTask, StartBMPTask, osPriorityNormal, 0, 256);
	osThreadCreate(osThread(bmpTask), NULL);
}


/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
