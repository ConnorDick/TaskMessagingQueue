/*
 * CPE-555 Real-Time and Embedded Systems
 * Stevens Institute of Technology
 * Spring 2022
 *
 * FreeRTOS Template
 *
 * Name: Connor Dick
 */

/* Standard includes */
#include <stdio.h>
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "Task.h"
#include "Queue.h"
#include "Semphr.h"

/* global variables */
QueueHandle_t timeQueue;
int minutes;
int seconds;
struct MinuteSecond {
    int Minutes;
    int Seconds;
} xMessage;

/* function prototypes */
void TimeTask() {

	//struct to hold mins and seconds data
    struct MinuteSecond pxMessage;
	//assign minutes and seconds data from scanf to pxMessage
    pxMessage.Minutes = minutes;
    pxMessage.Seconds = seconds;
	xQueueReset(timeQueue);
    for (;;) {
        //time is up check
		if ((pxMessage.Seconds == 0) && (pxMessage.Minutes == 0)) {
            //time is up - end scheduler and return fromm function
			printf("Time is up!\n");
			vTaskEndScheduler();
			return;
		}
        //send data to queue
		if(xQueueSend(timeQueue, (void *) &pxMessage, 1000/portTICK_RATE_MS) != 1){
			printf("Failed writing data to queue.\n");
		}
        //subtract seconds and check if minutes must be decremented
		pxMessage.Seconds -= 1;
		if ((pxMessage.Seconds == -1) && (pxMessage.Minutes != 0)) {
			pxMessage.Minutes -= 1;
			pxMessage.Seconds = 59;
		}
    }
}

void DisplayTask() {
    struct MinuteSecond pxRxedMessage;
    for (;;) {
        //recieve data from queue and do not block if data is not ready
		if (xQueueReceive(timeQueue, &(pxRxedMessage), 0) != 1) {
            //no data available - return from function
			printf("Failed to read queue. No data available.\n");
			return;
		}
		else {
			minutes = pxRxedMessage.Minutes;
			seconds = pxRxedMessage.Seconds;
			printf("TimeRemaining: %d %d\n", minutes, seconds);
			xQueueReset(timeQueue);
		}
		vTaskDelay(1000/portTICK_RATE_MS);
   }
}

/* main application code */
int main( void ) {
	setbuf(stdout, NULL);
	int status;
	int stackDepth = 1000;

    //get input from user
    printf("Please enter minutes to count down and press enter.\n");
    printf("Minutes: ");
    scanf("%i", &minutes);
    printf("\n");
    printf("Please enter seconds to count down and press enter.\n");
    printf("Seconds: ");
    scanf("%i", &seconds);
    printf("\n");

    //create timeQueue and check if null
    timeQueue = xQueueCreate(1, sizeof(struct MinuteSecond));
    if (timeQueue == NULL) {
    	printf("Failed to create queue.\n");
    }

    //create TimeTask and check if NULL
    status = xTaskCreate(TimeTask, "Holds info for minutes and seconds",
                stackDepth, NULL, 1, NULL);
    if (status == 0) {
        printf("TimeTask not initialized properly.\n");
        return 0;
    }

    //create DisplayTask and check if NULL
    status = xTaskCreate(DisplayTask, "Handles information display\n",
                stackDepth, NULL, 1, NULL);

    if (status == 0) {
        printf("DisplayTask not initialized properly.\n");
        return 0;
    }

    //start task scheduler -- note that both tasks are at same priority
    vTaskStartScheduler();

    //if all goes well we will never enter this loop because the scheduler is started
    for(;;) {
    	printf("Error with task scheduler.\n");
    }

	return 0;
}

