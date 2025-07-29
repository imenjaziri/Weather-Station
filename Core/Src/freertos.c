/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "mygps.h"
#include "portable.h" // Pour xPortGetFreeHeapSize()
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
xQueueHandle GpsToIhm;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId IHM_TaskHandle;
osThreadId GPS_TaskHandle;
osThreadId SatellitePredicHandle;
osThreadId Sensors_TaskHandle;
osThreadId MonitorTaskHandle;
osThreadId Lora_TaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void Start_IHM_Task(void const * argument);
void Start_GPS_Task(void const * argument);
void Start_SatellitePrediction_Task(void const * argument);
void Start_Sensors_Task(void const * argument);
void StartMonitorTask(void const * argument);
void Start_Lora_Task(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
	*ppxIdleTaskStackBuffer = &xIdleStack[0];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	/* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
{
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
	*ppxTimerTaskStackBuffer = &xTimerStack[0];
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
	/* place for user code */
}
/* USER CODE END GET_TIMER_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/*SimpleMutex = xSemaphoreCreateMutex();

	if (SimpleMutex != NULL)
	{
	 HAL_UART_Transmit(&huart2, "Mutex Created\n\n", 15, 1000);
	}*/
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	GpsToIhm=xQueueCreate(10,sizeof(GPS_IHM));
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of IHM_Task */
  osThreadDef(IHM_Task, Start_IHM_Task, osPriorityIdle, 0, 512);
  IHM_TaskHandle = osThreadCreate(osThread(IHM_Task), NULL);

  /* definition and creation of GPS_Task */
  osThreadDef(GPS_Task, Start_GPS_Task, osPriorityIdle, 0, 512);
  GPS_TaskHandle = osThreadCreate(osThread(GPS_Task), NULL);

  /* definition and creation of SatellitePredic */
  osThreadDef(SatellitePredic, Start_SatellitePrediction_Task, osPriorityIdle, 0, 1024);
  SatellitePredicHandle = osThreadCreate(osThread(SatellitePredic), NULL);

  /* definition and creation of Sensors_Task */
  osThreadDef(Sensors_Task, Start_Sensors_Task, osPriorityIdle, 0, 512);
  Sensors_TaskHandle = osThreadCreate(osThread(Sensors_Task), NULL);

  /* definition and creation of MonitorTask */
  osThreadDef(MonitorTask, StartMonitorTask, osPriorityIdle, 0, 256);
  MonitorTaskHandle = osThreadCreate(osThread(MonitorTask), NULL);

  /* definition and creation of Lora_Task */
  osThreadDef(Lora_Task, Start_Lora_Task, osPriorityIdle, 0, 512);
  Lora_TaskHandle = osThreadCreate(osThread(Lora_Task), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
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

/* USER CODE BEGIN Header_Start_IHM_Task */
/**
 * @brief Function implementing the IHM_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_IHM_Task */
__weak void Start_IHM_Task(void const * argument)
{
  /* USER CODE BEGIN Start_IHM_Task */

	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END Start_IHM_Task */
}

/* USER CODE BEGIN Header_Start_GPS_Task */
/**
 * @brief Function implementing the GPS_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_GPS_Task */
__weak void Start_GPS_Task(void const * argument)
{
  /* USER CODE BEGIN Start_GPS_Task */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END Start_GPS_Task */
}

/* USER CODE BEGIN Header_Start_SatellitePrediction_Task */
/**
 * @brief Function implementing the SatellitePredic thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_SatellitePrediction_Task */
__weak void Start_SatellitePrediction_Task(void const * argument)
{
  /* USER CODE BEGIN Start_SatellitePrediction_Task */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END Start_SatellitePrediction_Task */
}

/* USER CODE BEGIN Header_Start_Sensors_Task */
/**
 * @brief Function implementing the Sensors_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_Start_Sensors_Task */
__weak void Start_Sensors_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Sensors_Task */
	/* Infinite loop */
	for(;;)
	{
		osDelay(1);
	}
  /* USER CODE END Start_Sensors_Task */
}

/* USER CODE BEGIN Header_StartMonitorTask */
/**
 * @brief Function implementing the MonitorTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartMonitorTask */
void StartMonitorTask(void const * argument)
{
  /* USER CODE BEGIN StartMonitorTask */
//	char buf[200];

	for(;;)
	{
		/*sprintf(buf,
			    "Stack: GPS=%lu B | IHM=%lu B |Satellite_predict=%lu B |Sensors=%lu B | Heap Left=%lu B \r\n LORA =%lu ",
				uxTaskGetStackHighWaterMark(GPS_TaskHandle) ,
				uxTaskGetStackHighWaterMark(IHM_TaskHandle),uxTaskGetStackHighWaterMark(SatellitePredicHandle),
				uxTaskGetStackHighWaterMark(Sensors_TaskHandle) ,
				(unsigned long)xPortGetFreeHeapSize(),uxTaskGetStackHighWaterMark(Lora_TaskHandle));

		HAL_UART_Transmit(&huart2, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
*/
		osDelay(10);
	}
  /* USER CODE END StartMonitorTask */
}

/* USER CODE BEGIN Header_Start_Lora_Task */
/**
* @brief Function implementing the Lora_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_Start_Lora_Task */
__weak void Start_Lora_Task(void const * argument)
{
  /* USER CODE BEGIN Start_Lora_Task */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END Start_Lora_Task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/*void SendUart(uint8_t *str)
{
xSemaphoreTake(SimpleMutex,portMAX_DELAY);
HAL_Delay(2000);
HAL_UART_Transmit(&huart2,str, strlen((char*)str), 100);
xSemaphoreGive(SimpleMutex);
}*/
/* USER CODE END Application */
