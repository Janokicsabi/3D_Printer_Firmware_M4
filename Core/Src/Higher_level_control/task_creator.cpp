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
#include <Fan.h>
#include <Axis.h>
#include <Descartes_Axis.h>
#include <Command_control.h>
#include <Axis_commands.h>
#include <Temp_commands.h>
#include <Fan_commands.h>
#include "limit_switch.h"
#include "sd_card.h"
#include "temperature_control.h"
#include "thermistor.h"
#include "G_code_reader.h"
#include "Temp_controller.h"
#include <cpp_callback_wrap.h>
#include <Stepper.h>

//Hardware pointers
extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;

extern SPI_HandleTypeDef hspi1;

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim16;

//Hardware
SD_card* sd_card;
Axis* axes_array[NUM_OF_AXES];

//Semaphores
SemaphoreHandle_t temp_adc_sem;
SemaphoreHandle_t duty_cycle_sem;
SemaphoreHandle_t goal_temp_sem;

//Event flags
EventGroupHandle_t command_state;
//TODO REMOVE, TEST ONLY
EventGroupHandle_t event_test;

//Queues
xQueueHandle queue_command;

//TODO TEST
float current_temp_bed = 0.0f;


void cpp_wrap_task_creator(void* param) {
	task_creator(param);
}

float bed_temp_array_big[200];


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
	sd_card->open_file("test3");

	//Init

	Fan* fan_hotend = new Fan(FAN_HOTEND_GPIO_Port, FAN_HOTEND_Pin);
	Fan* fan_part_cooling = new Fan(FAN_PARTCOOLING_GPIO_Port, FAN_PARTCOOLING_Pin);
	fan_commands_init(fan_hotend, fan_part_cooling);

	//TODO ELLENŐIZNI A PARAMÉTEREKET!!!
	Stepper* stepper_X = new Stepper(STEP_X_GPIO_Port, STEP_X_Pin, DIR_X_GPIO_Port, DIR_X_Pin, 1.8, 8);
	Stepper* stepper_Y = new Stepper(STEP_Y_GPIO_Port, STEP_Y_Pin, DIR_Y_GPIO_Port, DIR_Y_Pin, 1.8, 8);
	Stepper* stepper_Z = new Stepper(STEP_Z_GPIO_Port, STEP_Z_Pin, DIR_Z_GPIO_Port, DIR_Z_Pin, 1.8, 8);
	Stepper* stepper_E = new Stepper(STEP_E_GPIO_Port, STEP_E_Pin, DIR_E_GPIO_Port, DIR_E_Pin, 1.8, 16);

	Limit_switch* limit_X = new Limit_switch(LIMIT_X_GPIO_Port, LIMIT_X_Pin, stepper_X);
	Limit_switch* limit_Y = new Limit_switch(LIMIT_Y_GPIO_Port, LIMIT_Y_Pin, stepper_Y);
	Limit_switch* limit_Z = new Limit_switch(LIMIT_Z_GPIO_Port, LIMIT_Z_Pin, stepper_Z);
	//TODO PARAMÉTEREKET ÁTÍRNI!!!
	Descartes_Axis* axis_X = new Descartes_Axis(stepper_X, limit_X, 5.0, 280.0, 40, 0, 2000, 7800);
	Descartes_Axis* axis_Y = new Descartes_Axis(stepper_Y, limit_Y, 5.0, 240.0, 40, 0, 200, 2000);
	Descartes_Axis* axis_Z = new Descartes_Axis(stepper_Z, limit_Z, 0.0, 180.0, 8, 0, 500, 3000);
	Axis* axis_E = new Axis(stepper_E, 4.637, 2000, 7200);
	Axis* axes[] = {axis_X, axis_Y, axis_Z, axis_E};
	axis_commands_init(&htim16, axis_X, axis_Y, axis_Z, axis_E);

	//TEMP_CONTROL
	Temp_controller* hotend_heater = new Temp_controller(HOTEND_GPIO_Port, HOTEND_Pin, &htim5, TIM_CHANNEL_1, 3.33, 0.0778, 42.86, 2.0, true);
	Temp_controller* bed_heater = new Temp_controller(BED_GPIO_Port, BED_Pin, &htim1, TIM_CHANNEL_1, 12.47, 0.3704, 40.9, 1.0, false);
	init_cpp_callback_wrap((void**)axes, (void*)hotend_heater, (void*)bed_heater);
	init_thermistor(&hadc1, &hadc2);
	temp_commands_init(hotend_heater, bed_heater);

	//TEST GCode reader & RTOS queue
	xTaskCreate(task_fill_message_queue, "G_CODE_READER", TASK_MID_STACK_SIZE, (void*)sd_card, TASK_LOW_PRIO, NULL);
	xTaskCreate(task_command_control, "COMMAND_RECEIVER", TASK_MID_STACK_SIZE, NULL, TASK_LOW_PRIO, NULL);


	//TEST FAN
	/*char read_instruction[30] = "M106 S229.5\n";
	Command* c = new Command();
	Command_struct* p_p;
	c->set_code_and_param_string(read_instruction);
	c->extract_params_from_command_string();
	p_p = c->get_params();
	execute_M106(p_p);*/

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
