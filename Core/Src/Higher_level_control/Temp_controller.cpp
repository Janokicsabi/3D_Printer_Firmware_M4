/*
 * Temp_controller.cpp
 *
 *  Created on: Oct 31, 2023
 *      Author: janok
 */

#include "Temp_controller.h"
#include "thermistor.h"
#include <algorithm>

float global_temp_hotend = 0.0f;
float global_temp_bed = 0.0f;
float global_duty = 0.0f;
bool global_is_goal = false;

Temp_controller::Temp_controller(GPIO_TypeDef* heater_port, uint16_t heater_pin, TIM_HandleTypeDef* timer,
		uint32_t timer_channel, float Kp, float Ki, float Ti_PI, float goal_error_tolerance, bool is_soft_pwm) {
	this->heater_port = heater_port;
	this->heater_pin = heater_pin;
	this->timer = timer;
	this->timer_channel = timer_channel;
	this->Kp = Kp;
	this->Ki = Ki;
	this->Ti_PI = Ti_PI;
	this->goal_error_tolerance = goal_error_tolerance;
	this->is_soft_pwm = is_soft_pwm;

	set_duty_cycle(0.0f);
	goal_temp = 0.0f;
	current_temp_celsius = 0.0f;
	integral_sum = 0.0f;
	sem_temp_array = xSemaphoreCreateBinary();
	xSemaphoreGive(sem_temp_array);

	std::fill(last_measured_temps, last_measured_temps+TEMP_ARRAY_SIZE, 0.0f);
	temp_index = 0;
}

Temp_controller::~Temp_controller() {

}

float Temp_controller::compute_PI_controller() {
	float err = goal_temp - current_temp_celsius;

	if (duty_cycle > 0 && duty_cycle < 100) {
		integral_sum += err;
	}

	float new_duty_cycle = (float)(Kp * err + (Ki / Ti_PI) * integral_sum);

    if (new_duty_cycle > 100) {
    	new_duty_cycle = 100.0f;
    }
	else if (new_duty_cycle < 0) {
		new_duty_cycle = 0.0f;
	}
    return new_duty_cycle;
}

TIM_HandleTypeDef* Temp_controller::get_timer() {
	return this->timer;
}

void Temp_controller::set_duty_cycle(float new_duty_cycle) {
	xSemaphoreTake(duty_cycle_sem, portMAX_DELAY);
	if (new_duty_cycle > 100) {
		new_duty_cycle = 100.0f;
	}
	else if (new_duty_cycle < 0) {
		new_duty_cycle = 0.0f;
	}
	duty_cycle = new_duty_cycle;
	xSemaphoreGive(duty_cycle_sem);
	modify_timer_compare();
}

float Temp_controller::get_duty_cycle() {
	uint8_t tmp = 0;
	xSemaphoreTakeFromISR(duty_cycle_sem, NULL);
	tmp = duty_cycle;
	xSemaphoreGiveFromISR(duty_cycle_sem, NULL);
	return tmp;
}

void Temp_controller::set_goal_temp(float new_goal_temp) {
	xSemaphoreTake(goal_temp_sem, portMAX_DELAY);
	if (new_goal_temp < MIN_GOAL_TEMP) {
		new_goal_temp = MIN_GOAL_TEMP;
	} else if (new_goal_temp > MAX_GOAL_TEMP) {
		new_goal_temp = MAX_GOAL_TEMP;
	}
	goal_temp = new_goal_temp;
	xSemaphoreGive(goal_temp_sem);
}

//TODO: Ezt itt megnézni, kezelni, miért ISR?
float Temp_controller::get_goal_temp() {
	uint8_t tmp = 0;
	xSemaphoreTakeFromISR(duty_cycle_sem, NULL);
	tmp = goal_temp;
	xSemaphoreGiveFromISR(duty_cycle_sem, NULL);
	return tmp;
}

void Temp_controller::set_current_temp_celius(float current_temp_celsius) {
	this->current_temp_celsius = current_temp_celsius;
	xSemaphoreTake(sem_temp_array, 0);
	last_measured_temps[temp_index] = current_temp_celsius;
	xSemaphoreGive(sem_temp_array);
	temp_index++;
	if (temp_index == TEMP_ARRAY_SIZE) {
		temp_index = 0;
	}
}

float Temp_controller::get_current_temp_celius() {
	return this->current_temp_celsius;
}

//TODO szemaforral védeni a last_measured_temps tömböt!!!
bool Temp_controller::is_goal_temp_reached() {
	xSemaphoreTake(sem_temp_array, 0);
	for (uint32_t i = 0; i < TEMP_ARRAY_SIZE; i++) {
		if (last_measured_temps[i] < (goal_temp - goal_error_tolerance) ||
				last_measured_temps[i] > (goal_temp + goal_error_tolerance)) {
			xSemaphoreGive(sem_temp_array);
			return false;
		}
	}
	xSemaphoreGive(sem_temp_array);
	return true;
}

void Temp_controller::modify_timer_compare() {
	uint16_t compare_value = (uint16_t)((get_duty_cycle() * (float)timer->Init.Period) / 100.0f);
	__HAL_TIM_SET_COMPARE(timer, timer_channel, compare_value);
}

void Temp_controller::heater_pin_on() {
	if (is_soft_pwm && get_duty_cycle() > 0) {
		HAL_GPIO_WritePin(heater_port, heater_pin, GPIO_PIN_SET);
	}
}

void Temp_controller::heater_pin_off() {
	if (is_soft_pwm && get_duty_cycle() < 100) {
		HAL_GPIO_WritePin(heater_port, heater_pin, GPIO_PIN_RESET);
	}
}

void Temp_controller::reset_controller_variables() {
	integral_sum = 0.0f;
}

void Temp_controller::heater_timer_start() {
	HAL_TIM_Base_Init(this->timer);
	HAL_TIM_PWM_Start_IT(timer, timer_channel);
	HAL_TIM_Base_Start_IT(timer);
}

void Temp_controller::heater_timer_stop() {
	HAL_TIM_Base_Init(this->timer);
	HAL_TIM_PWM_Stop_IT(timer, timer_channel);
	HAL_TIM_Base_Stop_IT(timer);
	HAL_GPIO_WritePin(heater_port, heater_pin, GPIO_PIN_RESET);
}

//TODO: Túlmelegedés védelmet átgondol!
void task_hotend_control(void* param) {
	while(1) {
		Temp_controller* hotend_controller = (Temp_controller*)param;
		uint16_t current_temp_adc = get_last_hotend_temp_adc();
		float current_temp_celsius = convert_temperature(current_temp_adc);

		//TODO Majd eltávolítani
		global_temp_hotend = current_temp_celsius;
		global_is_goal = ((Temp_controller*)param)->is_goal_temp_reached();

		if (current_temp_celsius > MAX_GOAL_TEMP) {
			hotend_controller->heater_timer_stop();
			while (current_temp_celsius > MAX_GOAL_TEMP) {
				vTaskDelay(100);
			}
			hotend_controller->heater_timer_start();
		}

		hotend_controller->set_current_temp_celius(current_temp_celsius);
		float new_duty_cycle = hotend_controller->compute_PI_controller();
		//TODO majd eltávlít
		global_duty = new_duty_cycle;
		hotend_controller->set_duty_cycle(new_duty_cycle);
		vTaskDelay(100);
	}
}

void task_bed_control(void* param) {
	while(1) {
		Temp_controller* bed_controller = (Temp_controller*)param;
		uint16_t current_temp_adc = get_last_bed_temp_adc();
		float current_temp_celsius = convert_temperature(current_temp_adc);

		//TODO Majd eltávolítani
		global_temp_bed = current_temp_celsius;
		global_is_goal = ((Temp_controller*)param)->is_goal_temp_reached();

		if (current_temp_celsius > MAX_GOAL_TEMP) {
			bed_controller->heater_timer_stop();
			while (current_temp_celsius > MAX_GOAL_TEMP) {
				vTaskDelay(100);
			}
			bed_controller->heater_timer_start();
		}

		bed_controller->set_current_temp_celius(convert_temperature(current_temp_adc));
		float new_duty_cycle = bed_controller->compute_PI_controller();
		//TODO majd eltávlít
		global_duty = new_duty_cycle;
		bed_controller->set_duty_cycle(new_duty_cycle);
		vTaskDelay(100);
	}
}

