/*
 * main_task.cpp
 *
 *  Created on: Oct 15, 2023
 *      Author: janok
 */

#include <task_creator.h>

//OS includes
#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

//Other file includes
#include <Motor.h>
#include <Axis.h>
#include <Command_control.h>
#include <Axis_commands.h>
#include "limit_switch.h"
#include "sd_card.h"
#include "temperature_control.h"
#include "thermistor.h"
#include "G_code_reader.h"
#include "Temp_controller.h"
#include <cpp_callback_wrap.h>

//Hardware pointers
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim8;

//Hardware
SD_card* sd_card;
Axis* axes_array[NUM_OF_AXES];
Temp_controller* hotend_heater;
Temp_controller* bed_heater;

//Semaphores
SemaphoreHandle_t temp_adc_sem;
SemaphoreHandle_t duty_cycle_sem;
SemaphoreHandle_t goal_temp_sem;

//Event flags
EventGroupHandle_t command_state;

//Queues
xQueueHandle queue_command;


void cpp_wrap_task_creator(void* param) {
	task_creator(param);
}


void task_creator(void* param) {
	//Semaphores
	temp_adc_sem = xSemaphoreCreateBinary();
	xSemaphoreGive(temp_adc_sem);
	duty_cycle_sem = xSemaphoreCreateBinary();
	xSemaphoreGive(duty_cycle_sem);
	goal_temp_sem = xSemaphoreCreateBinary();
	xSemaphoreGive(goal_temp_sem);

	//Flags
	command_state = xEventGroupCreate();
	xEventGroupSetBits(command_state, READY_FOR_NEXT_COMMAND);

	//Queues
	queue_command = xQueueCreate(MESSAGE_QUEUE_SIZE, sizeof(Command_struct));

	sd_card = new SD_card();
	sd_card->open_file("test");

	//Init
	init_thermistor(&hadc1, &hadc2);
	//init_temperature_control(&htim1, TIM_CHANNEL_2);

	//TODO ELLENŐIZNI A PARAMÉTEREKET!!!
	Motor* motor_X = new Motor(STEP_X_GPIO_Port, STEP_X_Pin, DIR_X_GPIO_Port, DIR_X_Pin, &htim4, TIM_CHANNEL_1, 8, true);
	Motor* motor_Y = new Motor(STEP_Y_GPIO_Port, STEP_Y_Pin, DIR_Y_GPIO_Port, DIR_Y_Pin, &htim3, TIM_CHANNEL_1, 8, false);
	Motor* motor_Z = new Motor(STEP_Z_GPIO_Port, STEP_Z_Pin, DIR_Z_GPIO_Port, DIR_Z_Pin, &htim2, TIM_CHANNEL_2, 8, false);
	Motor* motor_E = new Motor(STEP_E_GPIO_Port, STEP_E_Pin, DIR_E_GPIO_Port, DIR_E_Pin, &htim8, TIM_CHANNEL_2, 16, false);

	Limit_switch* limit_X = new Limit_switch(LIMIT_X_GPIO_Port, LIMIT_X_Pin, motor_X);
	Limit_switch* limit_Y = new Limit_switch(LIMIT_Y_GPIO_Port, LIMIT_Y_Pin, motor_Y);
	Limit_switch* limit_Z = new Limit_switch(LIMIT_Z_GPIO_Port, LIMIT_Z_Pin, motor_Z);
	//TODO PARAMÉTEREKET ÁTÍRNI!!!
	Axis* axis_X = new Axis(motor_X, limit_X, 30.0, 280.0, 2.0, 20, 0);
	Axis* axis_Y = new Axis(motor_Y, limit_Y, 60.0, 240.0, 2.0, 20, 0);
	Axis* axis_Z = new Axis(motor_Z, limit_Z, 0.0, 180.0, 2.0, 20, 0);
	Axis* axis_E = new Axis(motor_E, nullptr, 0.0, 0.0, 2.0, 20, 0);
	Axis* axes[] = {axis_X, axis_Y, axis_Z, axis_E};
	axis_commands_init(axis_X, axis_Y, axis_Z, axis_E);
	init_cpp_callback_wrap((void**)axes);


	//TEST GCode reader & RTOS queue
	//xTaskCreate(task_fill_message_queue, "G_CODE_READER", TASK_MID_STACK_SIZE, (void*)sd_card, TASK_LOW_PRIO, NULL);
	//xTaskCreate(task_command_control, "COMMAND_RECEIVER", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);

	//TEST MOTOR
	char read_instruction[30] = "G1 Z30.0 F1000\n";
	Command* c = new Command();
	Command_struct* p_p;
	c->set_code_and_param_string(read_instruction);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_G1(p_p);

	/*char read_instruction2[30] = "G1 X0.0 Z0.0 F2000\n";
	c->set_code_and_param_string(read_instruction2);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_G1(p_p);*/


	/*
	//Tasks
	xTaskCreate(task_temp_log, "TEMP_LOG_TASK", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_temp_read, "TEMP_READ_TASK", TASK_MID_STACK_SIZE, NULL, TASK_MID_PRIO, NULL);
	xTaskCreate(task_temp_control, "TEMP_CONTROL_TASK", TASK_MID_STACK_SIZE, NULL, TASK_HIGH_PRIO, NULL);

	xTaskCreate(task_fill_message_queue, "FILL_QUEUE_TASK", TASK_MID_STACK_SIZE, sd_card, TASK_LOW_PRIO, NULL);
	//TODO Visszatenni az alábbi sort
	//xTaskCreate(task_motor_control, "MOTOR_CONTROL_TASK", TASK_MID_STACK_SIZE, NULL, TASK_HIGH_PRIO, NULL);
	 */
}

