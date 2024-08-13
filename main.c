

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
#include <time.h>


#define configUSE_TRACE_FACILITY    1
#define configGENERATE_RUN_TIME_STATS  1

SemaphoreHandle_t xHelpTaskSemaphore;

QueueHandle_t xEchoQueue;  // 명령을 전달할 Queue
SemaphoreHandle_t xEchoMutex;

#include <ctype.h>
// Simulation of the CPU hardware sleeping mode
// Idle task hook, 지우면 안됨
void vApplicationIdleHook( void )
{
    usleep( 15000 );
}



void Help_command( void ){
    printf("Task1 is running\n");
}

void Time_command(void){
     time_t rawtime;
    struct tm * timeinfo;
    char buffer[80];

    // 현재 시간을 얻기
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // 현재 시간 포맷팅 (yyyy/mm/dd hh:mm:ss)
    strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S", timeinfo);

    // 현재 시간의 마이크로초 부분 얻기
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    int microseconds = ts.tv_nsec / 1000; // nanoseconds to microseconds

    // 결과 출력
    printf("현재 시간은 %s.%04d 입니다.\n\n", buffer, microseconds);
}

void Echo_command(void){

    printf("콘솔에 출력할 메세지를 입력하세요: ");

    char message;
    char inputBuffer[100];
    int index = 0;
    for (;;){
        message = getchar();
        if(message != '\n'){
            if (isprint(message)) {
                inputBuffer[index++] = message;
            } 
        }
        else{
            xQueueSend(xEchoQueue, inputBuffer, 0);
            index = 0;
            memset(inputBuffer, 0, sizeof(inputBuffer));    
            break;        
        }

    }
    
}

// Function to display task list
void Task_command(void) {
    char taskListBuffer[1024];
    char *line;


    char name[16];
    char state[8];
    const char *STATE;
    int priority;
    int stack;
    int taskID;
    vTaskList(taskListBuffer);
    line = strtok(taskListBuffer, "\n");

    printf("     NAME      | PERIOD | PRIORITY |  STATE  |\n");
    while (line != NULL) {
        // Extract task details from the line using sscanf
        sscanf(line, "%15[^\t] %8s %d %d %d", 
                   name, 
                   state, 
                   &priority, 
                   &stack, 
                   &taskID);

        switch(state[0]){
            case 'B':
                STATE = "Blocked";
                break;
            case 'R':
                STATE = "Running";
                break;
            case 'D':
                STATE = "Deleted";
                break;
            case 'S':
                STATE = "Suspended";
                break;
            case 'r':
                STATE = "Ready";
                break;
            default: 
                STATE = "Unknown";
                break;

        }
        printf("%-14s | %-6d | %-10s\n",
                name,
                priority,
                STATE);
        
        // Get the next line
        line = strtok(NULL, "\n");
    }
    printf("\n");
}

void EchoTask(void *pvParameters){
    char messageBuffer[50];

    for (;;) {
        if (xQueueReceive(xEchoQueue, messageBuffer, portMAX_DELAY)) {
            printf("입력한 메세지: %s\n\n", messageBuffer);  // 큐에서 받은 데이터 출력
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
                Help_command();
            }
            else if (strcmp(inputBuffer, "time") == 0) {
                Time_command();
            }
            else if (strcmp(inputBuffer, "task") == 0) {
                Task_command();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                Echo_command();
            }
            else if (strcmp(inputBuffer, "echo") == 0) {
                Echo_command();
            }                                    
            else if (strcmp(inputBuffer, "echo") == 0) {
                Echo_command();
            }
            else {
                printf("ERROR: 등록된 커맨드가 아닙니다.\n");
            }


            
            index = 0;
            memset(inputBuffer, 0, sizeof(inputBuffer));
        }
        
    }
}


int main( void )
{
    xEchoQueue = xQueueCreate(10, 100);
    xEchoMutex = xSemaphoreCreateMutex();


	xTaskCreate( InputTask, "Input Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate(EchoTask, "EchoTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	vTaskStartScheduler();
}