

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Local includes. */
#include "console.h" // posix 환경에서 console을 사용하기 위한 헤더파일. printf 대신 사용

// Simulation of the CPU hardware sleeping mode
// Idle task hook, 지우면 안됨

TaskHandle_t xTask3Handle;

void vApplicationIdleHook( void )
{
    usleep( 15000 );
}

// Task 1 정의
void vTask1( void *pvParameters )
{
	const char *pcTaskName = "Task 1: zzzzz\r\n";

	for( ;; )
	{
		console_print( pcTaskName );
        vTaskDelay( 1000 );
	}
}
// Task 2 정의
void vTask2( void *pvParameters )
{
	const char *pcTaskName = "Task 2: 알람! 알람! 알람!\r\n";
	for( ;; )
	{
		console_print( pcTaskName );
        vTaskDelay( 5000 );  // 5초 주기
		vTaskResume(xTask3Handle);

	}
}



// Task 3 정의
void vTask3( void *pvParameters )
{
	const char *pcTaskName = "Task 3: 5분만...\r\n";

	for( ;; )
	{
		console_print( pcTaskName );
        vTaskDelay( 1000 );  // 5초 주기
		vTaskSuspend(xTask3Handle);

	}
}

int main( void )
{
    console_init(); 

	xTaskCreate( vTask1, "Task 1", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( vTask2, "Task 2", configMINIMAL_STACK_SIZE, NULL, 3, NULL );
	//xTaskCreate( vTask3, "Task 3", configMINIMAL_STACK_SIZE, NULL, 2, NULL );
  

    // Task 3 생성 후 핸들 저장
    xTaskCreate( vTask3, "Task 3", configMINIMAL_STACK_SIZE, NULL, 2, &xTask3Handle );
    


	


	vTaskStartScheduler();
	for( ;; );
}