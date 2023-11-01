/*
 * temperature_control.c
 *
 *  Created on: Sep 27, 2023
 *      Author: janok
 */

#include "main.h"
#include "temperature_control.h"
#include "thermistor.h"

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"


extern SemaphoreHandle_t duty_cycle_sem;
extern SemaphoreHandle_t goal_temp_sem;

extern TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef* heater_timer;
uint32_t heater_timer_channel = 0;

static float duty_cycle = 0;
static float goal_temp = 0;


void init_temperature_control(TIM_HandleTypeDef* htim, uint32_t channel) {
	heater_timer = htim;
	heater_timer_channel = channel;
	set_duty_cycle(0);
	//set_goal_temp(150.0);
	//HAL_TIM_Base_Start_IT(&htim1);
	//HAL_TIM_PWM_Start_IT(heater_timer, heater_timer_channel);
}

#define Ts		8.0f
#define Ti_fox	35.0f
#define T		(-Ts/Ti_fox)
#define wc		0.032f
#define Kc		(Ti_fox * wc)
#define beta	(1 + T + T * T / 2)

float current_temp_celsius;
float err = 0;
float fox = 0;

const float Kp = 3.33;
const float Ki = 0.0778;
const float Ti_PI = 42.86;
float integral_sum = 0.0f;

float compute_PI_controller(const float setpoint, const float measured_value, float* integral_sum) {
	float err = setpoint - measured_value;
	*integral_sum += err;
    float new_duty_cycle = (float)(Kp * err + (Ki / Ti_PI) * (*integral_sum));

    if (new_duty_cycle > 100) {
    	new_duty_cycle = 100.0;
    }
	else if (new_duty_cycle < 0) {
		new_duty_cycle = 0.0;
	}
    return new_duty_cycle;
}

float compute_foxboro_controller(const float setpoint, const float measured_value, float* fox) {
	float err = setpoint - measured_value;
	float new_duty_cycle = (uint8_t)(Kc * err + *fox);
	if (new_duty_cycle > 100) {
		new_duty_cycle = 100.0;
	}
	else if (new_duty_cycle < 0) {
		new_duty_cycle = 0.0;
	}
	*fox = beta * (*fox) + (1 - beta) * new_duty_cycle;
    return new_duty_cycle;
}

void task_temp_control(void* param) {
	while(1) {
		uint16_t current_temp_adc = get_last_hotend_temp_adc();
		current_temp_celsius = convert_temperature(current_temp_adc);
		float new_duty_cycle = compute_PI_controller(get_goal_temp(), current_temp_celsius, &integral_sum);
		set_duty_cycle(new_duty_cycle);
		vTaskDelay(100);
	}
}

void set_duty_cycle(float new_duty_cycle) {
	xSemaphoreTake(duty_cycle_sem, portMAX_DELAY);
	if (new_duty_cycle > 100) {
		new_duty_cycle = 100.0f;
	}
	else if (new_duty_cycle < 0) {
		new_duty_cycle = 0.0f;
	}
	duty_cycle = new_duty_cycle;
	xSemaphoreGive(duty_cycle_sem);
	modify_timer_compare(heater_timer, heater_timer_channel, new_duty_cycle);
}

float get_duty_cycle() {
	uint8_t tmp = 0;
	xSemaphoreTakeFromISR(duty_cycle_sem, NULL);
	tmp = duty_cycle;
	xSemaphoreGiveFromISR(duty_cycle_sem, NULL);
	return tmp;
}

void set_goal_temp(float new_goal_temp) {
	xSemaphoreTake(goal_temp_sem, portMAX_DELAY);
	goal_temp = new_goal_temp;
	xSemaphoreGive(goal_temp_sem);
}

float get_goal_temp() {
	uint8_t tmp = 0;
	xSemaphoreTakeFromISR(duty_cycle_sem, NULL);
	tmp = goal_temp;
	xSemaphoreGiveFromISR(duty_cycle_sem, NULL);
	return tmp;
}

void modify_timer_compare(TIM_HandleTypeDef* htim, uint32_t channel, const float duty_cycle) {
	uint16_t compare_value = (uint16_t)((duty_cycle * (float)htim->Init.Period) / 100.0f);
	__HAL_TIM_SET_COMPARE(htim, channel, compare_value);
}


#define ARRAY_SIZE	1000
uint16_t temperature_adc[ARRAY_SIZE];
float temperature_celsius[ARRAY_SIZE];
uint32_t counter = 0;
extern TIM_HandleTypeDef htim1;

void task_temp_log(void* param) {
	while(1) {
		uint16_t current_temp_adc = get_last_hotend_temp_adc();
		float current_temp_celsius = convert_temperature(current_temp_adc);
		temperature_adc[counter] = current_temp_adc;
		temperature_celsius[counter] = current_temp_celsius;
		if (current_temp_celsius > 200) {
			heater_emergency_stop();
			counter = 0;
		}
		counter++;
		if (counter == ARRAY_SIZE) {
			heater_emergency_stop();
			counter = 0;
		}
		vTaskDelay(200);
	}
}

void heater_turn_on() {
	if (get_duty_cycle() > 0) {
		HAL_GPIO_WritePin(HOTEND_GPIO_Port, HOTEND_Pin, GPIO_PIN_SET);
	}
}

void heater_turn_off() {
	if (get_duty_cycle() < 100) {
		HAL_GPIO_WritePin(HOTEND_GPIO_Port, HOTEND_Pin, GPIO_PIN_RESET);
	}
}

void heater_emergency_stop() {
	HAL_TIM_PWM_Stop_IT(heater_timer, heater_timer_channel);
	HAL_TIM_Base_Stop_IT(heater_timer);
	HAL_GPIO_WritePin(HOTEND_GPIO_Port, HOTEND_Pin, GPIO_PIN_RESET);
}

