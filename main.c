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



SemaphoreHandle_t xHelpTaskSemaphore;

QueueHandle_t xEchoQueue;  // 명령을 전달할 Queue
SemaphoreHandle_t xInputMutex;
SemaphoreHandle_t xStartMutex;

#include <ctype.h>
// Simulation of the CPU hardware sleeping mode
// Idle task hook, 지우면 안됨
void vApplicationIdleHook( void )
{
    usleep( 15000 );
}

// Task 함수 선언
void EventTask1(void *pvParameters){
    printf("Event task 1: 이벤트 1가 설정되었습니다.\n");
    vTaskDelay(10000);
    vTaskDelete(NULL); 
};
void EventTask2(void *pvParameters){
    printf("Event task 2: 이벤트 2가 설정되었습니다.\n");
    vTaskDelay(10000);
    vTaskDelete(NULL); 
};
void EventTask3(void *pvParameters){
    printf("Event task #: 이벤트 3가 설정되었습니다.\n");
    vTaskDelay(10000);
    vTaskDelete(NULL); 
};
void EventTask4(void *pvParameters){
    printf("Event task 4: 이벤트 4가 설정되었습니다.\n");
    vTaskDelay(10000);
    vTaskDelete(NULL); 
};

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
            // 뮤텍스 잠금
            if (xSemaphoreTake(xInputMutex, portMAX_DELAY) == pdTRUE) {
                printf("입력한 메세지: %s\n\n", messageBuffer);  // 큐에서 받은 데이터 출력
                xSemaphoreGive(xInputMutex);  // 뮤텍스 해제
            }
        }
    }
}

void Start_view_command(void){
    if (xSemaphoreTake(xInputMutex, portMAX_DELAY) == pdTRUE) {
        for(int i = 0; i<5; i++){
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
            printf("-%s.%04d\n", buffer, microseconds);

            Task_command();

            vTaskDelay(1000);        
        }
        xSemaphoreGive(xInputMutex);
    }
    printf("출력이 종료되었습니다. 지금부터 입력이 허용됩니다.\n\n");
}

void Event_command(void){
    printf("설정할 이벤트 비트를 4 bit 이진수로 입력하세요: ");

    char event_bit[5];
    int right_input = 1;
    int input_come = 1;

    if (xSemaphoreTake(xInputMutex, portMAX_DELAY) == pdTRUE) {
        while(input_come){
            if (fgets(event_bit, sizeof(event_bit), stdin) != NULL) {
                size_t len = strlen(event_bit);
            
                if (len > 0 && event_bit[len - 1] == '\n') {
                    event_bit[len - 1] = '\0';
                }
                for(int i = 0;i<4;i++){
                    if(event_bit[i] != '1' && event_bit[i] != '0'){
                        printf("ERROR: 4 bit 이진수를 입력하세요!\n\n");
                        return;
                    }
                }

                input_come = 0;

            }
        }
        xSemaphoreGive(xInputMutex);

        if (event_bit[0] == '1') {
            xTaskCreate(EventTask1, "Event Task1", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        }
        if (event_bit[1] == '1') {
            xTaskCreate(EventTask2, "Event Task2", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        }
        if (event_bit[2] == '1') {
            xTaskCreate(EventTask3, "Event Task3", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        }
        if (event_bit[3] == '1') {
            xTaskCreate(EventTask4, "Event Task4", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
        }
    }

}

void InputTask( void *pvParameters )
{
    char command[10];
    
    for(;;){
        if (xSemaphoreTake(xInputMutex, portMAX_DELAY) == pdTRUE) {

            if (fgets(command, sizeof(command), stdin) != NULL) {
                xSemaphoreGive(xInputMutex);
                size_t len = strlen(command);
 

                if (len > 0 && command[len - 1] == '\n') {
                    command[len - 1] = '\0';
                }

                printf("%s", command);
                if (strcmp(command, "help") == 0) {
                    Help_command();
                }
                else if (strcmp(command, "time") == 0) {
                    Time_command();
                }
                else if (strcmp(command, "task") == 0) {
                    Task_command();
                }
                else if (strcmp(command, "echo") == 0) {
                    Echo_command();
                }
                else if (strcmp(command, "event") == 0) {
                    Event_command();
                }                                    
                else if (strcmp(command, "start_view") == 0) {
                    Start_view_command();
                }
                else if (strlen(command) != 0) {
                    printf("ERROR: 등록된 커맨드가 아닙니다.\n");
                }

                memset(command, 0, sizeof(command));
            }
            xSemaphoreGive(xInputMutex);

        }
        
    }

}


int main( void )
{
    xEchoQueue = xQueueCreate(10, 100);
    xInputMutex = xSemaphoreCreateMutex();
    xStartMutex = xSemaphoreCreateMutex();



	xTaskCreate( InputTask, "Input Task", configMINIMAL_STACK_SIZE, NULL, 1, NULL );
    xTaskCreate(EchoTask, "EchoTask", configMINIMAL_STACK_SIZE, NULL, 1, NULL);

	vTaskStartScheduler();
}