

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
/* Local includes. */
#include "console.h" // posix 환경에서 console을 사용하기 위한 헤더파일. printf 대신 사용



SemaphoreHandle_t xHelpTaskSemaphore;


#include <ctype.h>
// Simulation of the CPU hardware sleeping mode
// Idle task hook, 지우면 안됨
void vApplicationIdleHook( void )
{
    usleep( 15000 );
}



void HelpTask( void ){
    printf("Task1 is running\n");
}

void EchoTask(void){

    printf("콘솔에 출력할 메세지를 입력하세요: ");

    char message;
    char inputBuffer[10];
    int index = 0;
    for (;;){
        message = getchar();
        if(message != '\n'){
            if (isprint(message)) {
                inputBuffer[index++] = message;
            } 
        }
        else{
            printf("입력한 메세지: ");
            printf("hi %c\n", message);
            printf("helo %c\n", inputBuffer[0]);
            printf("%d\n", index);
            break;
        }

    }
    
}


void InputTask( void *pvParameters )
{
	char ch;
    char inputBuffer[10];
    int index = 0;
    for(;;){
        ch = getchar();  // 사용자로부터 문자 입력 받기

        // 입력된 문자가 인쇄 가능한 문자인지 확인
        if(ch != '\n'){
            if (isprint(ch)) {
                inputBuffer[index++] = ch;
            } 
        }
        else{
            if (strcmp(inputBuffer, "help") == 0) {
                HelpTask();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                EchoTask();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                EchoTask();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                EchoTask();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                EchoTask();
            }                                    
            else if (strcmp(inputBuffer, "echo") == 0) {
                EchoTask();
            }
            else {
                printf("ERROR: 등록된 커맨드가 아닙니다.");
            }


            
            index = 0;
            memset(inputBuffer, 0, sizeof(inputBuffer));
        }
        
    }
}


int main( void )
{


	xTaskCreate( InputTask, "Input Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	vTaskStartScheduler();
}