#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include "Timer.h"
#include <sys/neutrino.h>
#include "Client.h"
#include <hw/inout.h> // for in32() and out32();
#include <sys/mman.h> // for mmap_device_io();
#include <stdint.h> // for unit32 types

#define Bit_0 (1<<0)
#define Bit_1 (1<<1)
#define Bit_2 (1<<2)
#define Bit_3 (1<<3)
#define Bit_4 (1<<4)
#define Bit_5 (1<<5)
#define Bit_6 (1<<6)
#define Bit_7 (1<<7)
#define Bit_8 (1<<8)
#define Bit_9 (1<<9)
#define Bit_10 (1<<10)
#define Bit_11 (1<<11)
#define Bit_12 (1<<12)
#define Bit_13 (1<<13)
#define Bit_14 (1<<14)
#define Bit_15 (1<<15)
#define Bit_16 (1<<16)
#define Bit_17 (1<<17)
#define Bit_18 (1<<18)
#define Bit_19 (1<<19)
#define Bit_20 (1<<20)
#define Bit_21 (1<<21)
#define Bit_22 (1<<22)
#define Bit_23 (1<<23)
#define Bit_24 (1<<24)
#define Bit_25 (1<<25)
#define Bit_26 (1<<26)
#define Bit_27 (1<<27)
#define Bit_28 (1<<28)
#define Bit_29 (1<<29)
#define Bit_30 (1<<30)
#define Bit_31 (1<<31)
#define Bit_32 (1<<32)

#define BCM2711_GPIO1_BASE	0xFE200000	//GPIO base offset
#define BCM2711_GPIO_SIZE	0x000000FF	//size of the GPIO

#define D0 (1<<17) // Pin 11 - red lights 1
#define D1 (1<<27) // Pin 13 - red lights 2
#define D2 (1<<22) // Pin 15 - crossing alarm
#define D3 (1<<05) // Pin 29 - green lights
#define D4 (1<<06) // Pin 31 - red lights

#define Dw D0|D1|D2|D3|D4

#define GPFSEL0						0x00 // function select register 0 (GPIO9:GPIO0)
#define GPFSEL1						0x04 // function select register 1 (GPIO19:GPIO10)
#define GPFSEL2						0x08 // function select register 2 (GPIO29:GPIO20)
#define GPSET0 						0x1C // set register 0
#define GPCLR0 						0x28 // clear register 0

//#define GPIO_PUP_PDN_CNTRL_REG1		0xe8// pull up pull down register 1
//#define GPLEV0						0x34// get pin level register 0

void Setup_Pin(uintptr_t gpio_add, unsigned int clear_bits, unsigned int set_bits) {
	volatile uint32_t	val = 0;
	val = in32(gpio_add); //Read in initial
	printf("Initial register value = %#010x\n", val);

	val &= ~ (clear_bits | set_bits); // clear the bits that we might change
	val |= set_bits;

	printf("New register value     = %#010x\n", val);
	out32(gpio_add, val); // Assert the new value to the register
}

void Set_IO_High(uintptr_t gpio_add, unsigned int bits) {
	volatile uint32_t	val = 0;
	val = in32(gpio_add); //Read in initial val
	//printf("Initial register value = %#010x\n", val);

	val &= ~(Dw); // clear the bits that we might change
	val |= bits;  // set high

	//printf("New register value     = %#010x\n", val);
	out32(gpio_add, val); // Assert the new value to the register
}

// GPIO global pointers
uintptr_t gpio1_base;

enum states {
	State0, //Waiting for train
	State1, //Train crossing
	State2 //Crossing error
};
enum states CurState = State0; //starting state

//set state to 2 when failure is detected
#define FAIL_TO_STATE2() do { \
		printf("A failure was detected\n"); \
		printf("Waiting for failure to be dismissed by master controller\n"); \
		CurState = State2; \
		return NULL; \
}while(0)

bool gatesLowered;
bool redLight;
bool crossingAlarm;
bool trainSignalRed;
bool CrossingOpen;

pthread_mutex_t stateMutex = PTHREAD_MUTEX_INITIALIZER;

void *communication (void *data) { //communication thread
	ClientParam clientParam;

	my_data_t clientMsg;

	if(clientInit(&clientParam) == -1){
		printf("Failed to initialise client\n");
		return EXIT_FAILURE;
	}

	clientMsg.hdr.type    = 0x00;
	clientMsg.hdr.subtype = 0x00;

	// Client started, can start sending messages
	int rcvid;
	my_message_t msg;
	TimerParam timerParam;

	if(timerInit(&timerParam, 1) == -1){
		printf("Failed to initialise timer\n");
		return EXIT_FAILURE;
	}

	//printf("Timer started. Waiting for pulses...\n");

	int dataToSend = 0;
	while(1){
		rcvid = MsgReceive(timerParam.chid, &msg, sizeof(msg), NULL);
		if(rcvid == -1){
			printf("Error while receiving message\n");
			break;
		}
		// rcvid == 0 -> a pulse
		if(rcvid == 0){
			// can also check the pulse code here to make sure its from the timer
			if (msg.code == STATUS_TIMER) {
			    clientMsg.crossingHeartbeat = dataToSend; //heartbeat message
			    clientMsg.crossingState = (int)CurState; //state message
			    if (MsgSend(clientParam.serverCoid, &clientMsg, sizeof(clientMsg), NULL, 0) == -1) {
			    	printf("Failed to send message\n");
			    	return EXIT_FAILURE;
			    }
			    dataToSend += 1;
			}
		}
	}

	clientCleanUp(&clientParam);
	timerCleanUp(&timerParam);
}

void *railSensor (void *data) { //rail sensor read thread
	int key;

	printf("Enter t to simulate train detection, n to clear sensor\n");
	fflush(stdout);

	while ((key = getchar()) != EOF){
		pthread_mutex_lock(&stateMutex);

		if (CurState == State2){
			break;
		}else if (key == 't') {
			CurState = State1;
		} else if (key == 'n') {
			CurState = State0;
		}

		pthread_mutex_unlock(&stateMutex);
	}
	return NULL;
}

void *crossingOutput (void *data){ //crossing output thread
	gpio1_base = mmap_device_io(BCM2711_GPIO_SIZE, BCM2711_GPIO1_BASE);
	if (CrossingOpen){
		if (trainSignalRed){
				printf("The train signal is red\n");
				Set_IO_High(gpio1_base + GPSET0, D4); // set D4 to high
			} else {
				FAIL_TO_STATE2();
			}

		if (!gatesLowered) {
				printf("The gates are up\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (!redLight) {
				printf("Red lights are off\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (!crossingAlarm) {
				printf("The alarm is off\n");
			} else {
				FAIL_TO_STATE2();
			}
			printf("Waiting for train\n");

	}else if (!CrossingOpen){
		if (gatesLowered) {
			printf("Train detected\n");
				printf("The gates are down\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (redLight) {
				printf("Red lights are flashing\n");
			} else {
				FAIL_TO_STATE2();
			}

			if (crossingAlarm) {
				printf("The alarm is on\n");
			} else {
				FAIL_TO_STATE2();
			}
			if (!trainSignalRed){
				printf("The train signal is green\n");
				Set_IO_High(gpio1_base + GPSET0, D3); // set D4 to high
			} else {
				FAIL_TO_STATE2();
			}
	}
	return NULL;
}

int main(void) {


	volatile uint32_t	val = 0;

	gpio1_base = mmap_device_io(BCM2711_GPIO_SIZE, BCM2711_GPIO1_BASE);

	if( !gpio1_base ){
			// An error has occurred
			perror("Can't map Control Base Module / GPIO Base");
			printf("Mapping IO ERROR! Main Terminated...!\n");
			return EXIT_SUCCESS;
	}
	printf("I/O Port mapping is Successful!\n");

	printf("\nD0 set as output\n");
		// Configure D0 (GPIO17) 23:21 = 001
		Setup_Pin(gpio1_base + GPFSEL1, Bit_23 | Bit_22, Bit_21);

		printf("\nD1 set as output\n");
		// Configure D1 (GPIO27) 23:21 = 001
		Setup_Pin(gpio1_base + GPFSEL2, Bit_23 | Bit_22, Bit_21);

		printf("\nD2 set as output\n");
		// Configure D2 (GPIO22) 8:6 = 001
		Setup_Pin(gpio1_base + GPFSEL2, Bit_8 | Bit_7, Bit_6);

		printf("\nD3 set as output\n");
		// Configure D3 (GPIO05) 17:15 = 001
		Setup_Pin(gpio1_base + GPFSEL0, Bit_17 | Bit_16, Bit_15);

		printf("\nD4 set as output\n");
		// Configure D4 (GPIO06) 20:18 = 001
		Setup_Pin(gpio1_base + GPFSEL0, Bit_20 | Bit_19, Bit_18);

		// make all data zero
		Set_IO_High(gpio1_base + GPCLR0, Dw); // set all data bits to clear
		//usleep(500);
		Set_IO_High(gpio1_base + GPSET0, Dw); // set all data bits to high
		//usleep(500);
		Set_IO_High(gpio1_base + GPCLR0, Dw); // set all data bits to clear


	pthread_t th1, th2, th3;

	if (pthread_create (&th2, NULL, communication, NULL) != 0) {
				perror("pthread_create");
				return 1;
			}

	if (pthread_create (&th1, NULL, railSensor, NULL) != 0) {
			perror("pthread_create");
			return 1;
	}

	enum states lastState;

	while(1) {
		enum states state;

		pthread_mutex_lock(&stateMutex);
		state = CurState;
		pthread_mutex_unlock(&stateMutex);

		if (state != lastState) {
			Set_IO_High(gpio1_base + GPCLR0, Dw); // set all data bits to clear
			//usleep(500);
			Set_IO_High(gpio1_base + GPSET0, Dw); // set all data bits to high
			//usleep(500);
			Set_IO_High(gpio1_base + GPCLR0, Dw); // set all data bits to clear

			if (state == State0) {
				CrossingOpen = true;
				gatesLowered = false;
				//gatesLowered = true; //Simulate a component fail
				redLight = false;
				crossingAlarm = false;
				trainSignalRed = true;

			} else if (state == State1){
				CrossingOpen = false;
				gatesLowered = true;
				redLight = true;
				crossingAlarm = true;
				trainSignalRed = false;
			}else if (state == State2){
				Set_IO_High(gpio1_base + GPSET0, D4); // set D4 to high
				break;
			}
		if (pthread_create (&th3, NULL, crossingOutput, NULL) != 0) {
				perror("pthread_create");
				return 1;
		}
		pthread_join (th3, NULL);
		lastState = state;
		}
	}

	pthread_join (th2, NULL);
	pthread_join (th1, NULL);
}
