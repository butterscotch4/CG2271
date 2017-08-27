#include <Arduino.h>
#include <avr/io.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#define STACK_SIZE 200
#define ledBREAK 10
#define led1 6
#define led2 7
#define led3 8


#define PUSH_BUTTON1 2
#define PUSH_BUTTON2 3
#define SPEAKER 9
#define PTTM 0

unsigned long previousTime = 0;
volatile int stop_flag = 0;
volatile int speed = 0;
volatile int desired_speed = 0;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay1 = 500;
unsigned long debounceDelay2 = 500;

QueueHandle_t xQueueUART = 0;
QueueHandle_t xQueueDist = 0;

int remap(int val){
	return (int) (val * 255.0 / 1023.0);
}

void stopFlag(void){
	TickType_t xCurrTime = xTaskGetTickCount();
	if (stop_flag==1){

		if (xCurrTime - previousTime >= 1000){
			previousTime = xCurrTime;

			digitalWrite(ledBREAK, HIGH);

		}
		stop_flag = 0;

	}
}

void setSpeed(int dist){
	if(dist<2){
		speed = 0;
	}
	else if(dist<3){
		speed = 1;
	}
	else if(dist<4){
		speed = 2;
	}
	else{
		speed = 3;
	}

}
void speed_task(void *p)
{

	TickType_t xPrevTime;
	const TickType_t xPeriod = 1000;

	while(1){

		int safeDistance;
		xPrevTime = xTaskGetTickCount();

		xQueueReceive(xQueueDist, &safeDistance, portMAX_DELAY);

		if (speed == 0){
			digitalWrite(led1, LOW);
			digitalWrite(led2, LOW);
			digitalWrite(led3, LOW);
			digitalWrite(ledBREAK, LOW);
			tone(SPEAKER, 1519 );
		}

		else if (speed == 1){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, LOW);
			digitalWrite(led3, LOW);

			tone(SPEAKER, 1432 );
			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 2){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);

				setSpeed(safeDistance);
				stopFlag();
			}

		}

		else if (speed == 2){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, HIGH);
			digitalWrite(led3, LOW);
			tone(SPEAKER, 1136 );
			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 3){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);
				setSpeed(safeDistance);
				stopFlag();
			}

		}

		else if (speed == 3){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, HIGH);
			digitalWrite(led3, HIGH);
			tone(SPEAKER, 956);

			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 4){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);
				setSpeed(safeDistance);
				stopFlag();
			}

		}

		vTaskDelayUntil(&xPrevTime, xPeriod);
	}

}



void distance_task(void *p) {
	int distance = 0;
	TickType_t xPrevTime;
	const TickType_t xPeriod = 1000;
	xPrevTime = xTaskGetTickCount();
	while(1){
		int val;
		val = analogRead(PTTM);
		val = remap(val);
		if (val <= 64){
			distance = 1;

		}
		else if ((val>64) && (val<=128)){
			distance = 2;

		}
		else if ((val>128) && (val<=192)){
			distance = 3;
		}
		else if (val>192){
			distance = 4;
		}
		xQueueSend(xQueueDist, &distance, portMAX_DELAY);
		xQueueSend(xQueueUART, &distance, portMAX_DELAY);
		vTaskDelayUntil(&xPrevTime, xPeriod);
	}

}
void int0ISR(void)
{

	if((millis() - lastDebounceTime1) > debounceDelay1){
		if(speed < 3){
			speed++;
		}
		if(desired_speed < 3){
			desired_speed++;
		}
		lastDebounceTime1 = millis();
	}

}

void int1ISR()
{
	if((millis() - lastDebounceTime2) > debounceDelay2){
		if(speed > 0){
			speed--;
		}
		if(desired_speed > 0){
			desired_speed--;
		}
		lastDebounceTime2 = millis();
	}
}

void serialPrint(void *p){
	while(1){
		int taskDistance;
		xQueueReceive(xQueueUART, &taskDistance, portMAX_DELAY);
		Serial.print("Desired Speed: ");
		Serial.println(desired_speed);
		Serial.print("Current Speed: ");
		Serial.println(speed);
		Serial.print("Distance: ");
		if( taskDistance != 1)
			Serial.print(taskDistance);
		Serial.println("d");
	}
}


void setup() {
	Serial.begin(115200);
	pinMode(ledBREAK, OUTPUT);
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(led3, OUTPUT);
	pinMode(SPEAKER, OUTPUT);
	pinMode(PUSH_BUTTON1, INPUT);
	pinMode(PUSH_BUTTON2, INPUT);
	attachInterrupt(0,int0ISR, RISING);
	attachInterrupt(1,int1ISR, RISING);
}



void loop() {


	xQueueUART = xQueueCreate(10, sizeof(unsigned long));
	xQueueDist = xQueueCreate(10, sizeof(unsigned long));

	xTaskCreate(speed_task, "Task1", STACK_SIZE, NULL, 0, NULL);

	xTaskCreate(distance_task, "Task2", STACK_SIZE,  NULL, 1, NULL);

	xTaskCreate(serialPrint, "Task4", STACK_SIZE,  NULL, 2, NULL);

	vTaskStartScheduler();

}
#include <Arduino.h>
#include <avr/io.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#define STACK_SIZE 200
#define ledBREAK 10
#define led1 6
#define led2 7
#define led3 8


#define PUSH_BUTTON1 2
#define PUSH_BUTTON2 3
#define SPEAKER 9
#define PTTM 0

unsigned long previousTime = 0;
volatile int stop_flag = 0;
volatile int speed = 0;
volatile int desired_speed = 0;

unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay1 = 500;
unsigned long debounceDelay2 = 500;

QueueHandle_t xQueueUART = 0;
QueueHandle_t xQueueDist = 0;

int remap(int val){
	return (int) (val * 255.0 / 1023.0);
}

void stopFlag(void){
	TickType_t xCurrTime = xTaskGetTickCount();
	if (stop_flag==1){

		if (xCurrTime - previousTime >= 1000){
			previousTime = xCurrTime;

			digitalWrite(ledBREAK, HIGH);

		}
		stop_flag = 0;

	}
}

void setSpeed(int dist){
	if(dist<2){
		speed = 0;
	}
	else if(dist<3){
		speed = 1;
	}
	else if(dist<4){
		speed = 2;
	}
	else{
		speed = 3;
	}

}
void speed_task(void *p)
{

	TickType_t xPrevTime;
	const TickType_t xPeriod = 1000;

	while(1){

		int safeDistance;
		xPrevTime = xTaskGetTickCount();

		xQueueReceive(xQueueDist, &safeDistance, portMAX_DELAY);

		if (speed == 0){
			digitalWrite(led1, LOW);
			digitalWrite(led2, LOW);
			digitalWrite(led3, LOW);
			digitalWrite(ledBREAK, LOW);
			tone(SPEAKER, 1519 );
		}

		else if (speed == 1){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, LOW);
			digitalWrite(led3, LOW);

			tone(SPEAKER, 1432 );
			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 2){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);

				setSpeed(safeDistance);
				stopFlag();
			}

		}

		else if (speed == 2){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, HIGH);
			digitalWrite(led3, LOW);
			tone(SPEAKER, 1136 );
			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 3){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);
				setSpeed(safeDistance);
				stopFlag();
			}

		}

		else if (speed == 3){

			digitalWrite(led1, HIGH);
			digitalWrite(led2, HIGH);
			digitalWrite(led3, HIGH);
			tone(SPEAKER, 956);

			digitalWrite(ledBREAK, LOW);
			if(safeDistance < 4){
				stop_flag = 1;

				digitalWrite(ledBREAK, LOW);
				setSpeed(safeDistance);
				stopFlag();
			}

		}

		vTaskDelayUntil(&xPrevTime, xPeriod);
	}

}



void distance_task(void *p) {
	int distance = 0;
	TickType_t xPrevTime;
	const TickType_t xPeriod = 1000;
	xPrevTime = xTaskGetTickCount();
	while(1){
		int val;
		val = analogRead(PTTM);
		val = remap(val);
		if (val <= 64){
			distance = 1;

		}
		else if ((val>64) && (val<=128)){
			distance = 2;

		}
		else if ((val>128) && (val<=192)){
			distance = 3;
		}
		else if (val>192){
			distance = 4;
		}
		xQueueSend(xQueueDist, &distance, portMAX_DELAY);
		xQueueSend(xQueueUART, &distance, portMAX_DELAY);
		vTaskDelayUntil(&xPrevTime, xPeriod);
	}

}
void int0ISR(void)
{

	if((millis() - lastDebounceTime1) > debounceDelay1){
		if(speed < 3){
			speed++;
		}
		if(desired_speed < 3){
			desired_speed++;
		}
		lastDebounceTime1 = millis();
	}

}

void int1ISR()
{
	if((millis() - lastDebounceTime2) > debounceDelay2){
		if(speed > 0){
			speed--;
		}
		if(desired_speed > 0){
			desired_speed--;
		}
		lastDebounceTime2 = millis();
	}
}

void serialPrint(void *p){
	while(1){
		int taskDistance;
		xQueueReceive(xQueueUART, &taskDistance, portMAX_DELAY);
		Serial.print("Desired Speed: ");
		Serial.println(desired_speed);
		Serial.print("Current Speed: ");
		Serial.println(speed);
		Serial.print("Distance: ");
		if( taskDistance != 1)
			Serial.print(taskDistance);
		Serial.println("d");
	}
}


void setup() {
	Serial.begin(115200);
	pinMode(ledBREAK, OUTPUT);
	pinMode(led1, OUTPUT);
	pinMode(led2, OUTPUT);
	pinMode(led3, OUTPUT);
	pinMode(SPEAKER, OUTPUT);
	pinMode(PUSH_BUTTON1, INPUT);
	pinMode(PUSH_BUTTON2, INPUT);
	attachInterrupt(0,int0ISR, RISING);
	attachInterrupt(1,int1ISR, RISING);
}



void loop() {


	xQueueUART = xQueueCreate(10, sizeof(unsigned long));
	xQueueDist = xQueueCreate(10, sizeof(unsigned long));

	xTaskCreate(speed_task, "Task1", STACK_SIZE, NULL, 0, NULL);

	xTaskCreate(distance_task, "Task2", STACK_SIZE,  NULL, 1, NULL);

	xTaskCreate(serialPrint, "Task4", STACK_SIZE,  NULL, 2, NULL);

	vTaskStartScheduler();

}