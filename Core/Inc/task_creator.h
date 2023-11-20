/*
 * main_task.h
 *
 *  Created on: Oct 15, 2023
 *      Author: janok
 */

#ifndef INC_TASK_CREATOR_H_
#define INC_TASK_CREATOR_H_


#include "main.h"

//Task prio defs
#define TASK_HIGH_PRIO	tskIDLE_PRIORITY + 3
#define TASK_MID_PRIO	tskIDLE_PRIORITY + 2
#define TASK_LOW_PRIO	tskIDLE_PRIORITY + 1

//Task stack size defs
#define TASK_BIG_STACK_SIZE		2048
#define TASK_MID_STACK_SIZE		512
#define TASK_SMALL_STACK_SIZE	128

#define NUM_OF_AXES				4		//X+Y+Z+E

#define MESSAGE_QUEUE_SIZE		50

#define READY_FOR_NEXT_COMMAND		(1 << 0)

#define MOTOR_X_FINISHED	(1 << 0)
#define MOTOR_Y_FINISHED	(1 << 1)
#define MOTOR_Z_FINISHED	(1 << 2)
#define MOTOR_E_FINISHED	(1 << 3)


#ifdef __cplusplus
extern "C" void cpp_wrap_task_creator(void* param);
#endif


void task_creator(void* param);


#endif /* INC_TASK_CREATOR_H_ */
