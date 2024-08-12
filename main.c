

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"




#include "queue.h"
#include "semphr.h"
#include "FreeRTOS_CLI.h"


/* Local includes. */
#include "console.h" // posix 환경에서 console을 사용하기 위한 헤더파일. printf 대신 사용

// Simulation of the CPU hardware sleeping mode
// Idle task hook, 지우면 안됨
void vApplicationIdleHook( void )
{
    usleep( 15000 );
}



// CLI Command function
static BaseType_t prvHelpCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
    // 도움말 메시지를 출력합니다.
    snprintf(pcWriteBuffer, xWriteBufferLen, "help는 FreeRTOS에서 도움말 명령어입니다.\r\n");
    return pdFALSE; // 하나의 출력만 필요하므로 pdFALSE 반환
}

// CLI Command definition
static const CLI_Command_Definition_t xHelpCommand = {
    "help",                    // 명령어 텍스트
    "help: 도움말을 제공합니다.\r\n", // 도움말 텍스트
    prvHelpCommand,            // 명령어 함수
    0                          // 매개변수 개수
};

// CLI 명령어 등록 함수
void vRegisterCLICommands(void) {
    FreeRTOS_CLIRegisterCommand(&xHelpCommand); // help 명령어 등록
}

// 메인 함수
int main(void) {
    // 기타 시스템 초기화 코드...
    
    vRegisterCLICommands(); // CLI 명령어 등록
    
    // RTOS 스케줄러 시작
    vTaskStartScheduler();
    
    // 실행이 이곳에 도달하면 안 됩니다.
    for (;;);
}