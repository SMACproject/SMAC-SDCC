/*
 * radio_test.c
 *
 * Created on: Sep 11, 2014
 *     Author: Jianing Liu
 *
 * Copyright (c) 2014, STORM Lab, Vanderbilt University
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of the copyright holder.
 * 
 */

#include "dev/led.h"
#include <stdio.h>
#include "config.h"

#define TRANSMITTER 1
#define RECEIVER 0

#include "adc.h"
#include <stdbool.h>
#include "cc253x_rf.h"
#include <clock.h>
//#include <timer1.h>
//MODE defined in timer1.h will cause syntax error
#include "hal_mcu.h"
#include "hal_rf.h"
#include "hal_types.h"
#include "lis331dlh.h"
#include "mpl115a.h"
#include "pwr_mode_manager.h"
#include "rf_pack.h"
#include "spi_manager.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "compiler.h"
#include "cc253x.h"
#include "sfr-bits.h"

#define DUMMY     0x00;

//  Check if can be removed
#define LOB(x) (*((unsigned char *)&(x)))
#define M1B(x) (*((unsigned char *)&(x)+1))
#define M2B(x) (*((unsigned char *)&(x)+2))
#define HIB(x) (*((unsigned char *)&(x)+3))

// Application parameters
#define RF_CHANNEL                5      // 2.4 GHz RF channel

// BasicRF address definitions
#define PAN_ID               0x2005
#define PC_ADDR              0xACC0
#define SENSOR_ADDR          0x0CCA

#define APP_PAYLOAD_LENGTH   30

// BasicRF Commands
#define CONNECT                           'R'
#define ACK_RECEIVER                      'C'
#define ACK_WD                            'D'
#define ACK_RECEIVER2                     'c'
#define WD_PKG_DETECTED                   'k'
#define SLEEP_MESSAGE                     'S'
#define SLEEP_ACK                         's'
#define WD_START_REQ                      'q'
#define ENABLE_ACQUISITION                'E'
#define ENABLE_ACQUISITION_ACK            'e'
#define CONTINUE                          'x'
#define HOLD_ON                           'h'
#define PAUSE                             'J'

// Device Status
#define WAIT_CONNECT           0x00
#define WD_CONNECTED           0x01
#define ALMOUST_READY          0x02
#define READY                  0x03
#define SLEEP                  0x04
#define ENABLE_ACQ             0x05

unsigned char __xdata SPI1TxBuf[256];
unsigned char __xdata SPI1RxBuf[256];

unsigned char __xdata SPI1TxIndex;
unsigned char __xdata SPI1RxIndex;

static uint8 pTxData[APP_PAYLOAD_LENGTH];
static uint8 pRxData[APP_PAYLOAD_LENGTH];
static basicRfCfg_t basicRfConfig;
static RF_PACKAGE_t data;
bool RF_Timeout_Status = 0;
unsigned int retry_counter =0;

unsigned char device_status = 0x00;
unsigned char valid_package = 0x00;
unsigned int battery= 0;
unsigned long counter_loop = 0;

unsigned char ADC1_VAL[2]={0};
unsigned char ADC2_VAL[2]={0};
unsigned char ADC3_VAL[2]={0};
unsigned char ADC4_VAL[2]={0};
unsigned char ADC5_VAL[2]={0};
unsigned char ADC6_VAL[2]={0};

unsigned char acc[6] = {0};
unsigned char coefficients_stored[16] = {0};

bool  RF_Timeout(uint16 us_to_wait);

void delay(void){
	volatile int i;
	i = 0;
	while (i < 32000) {
		i++;
	}
}

void user_main(void) {
	halMcuInit();   // Set Main Clock source to XOSC AND RCOSC
	EA = 1;         // Global interrupt enable
	device_status = WAIT_CONNECT;

	// Config basicRF

	basicRfConfig.panId = PAN_ID;
	basicRfConfig.channel = RF_CHANNEL;
	basicRfConfig.ackRequest = FALSE;

	// Initialize BasicRFpRF_PACKAGE
	basicRfConfig.myAddr = SENSOR_ADDR;
	basicRfInit(&basicRfConfig);
	basicRfReceiveOn();

	// Clear the Structure ...
	RF_Package_Init(&data);
	data.PAYLOAD_DATA_SENSOR[0]=CONNECT;
	RF_Package_Compose(&data);

	// COPY OF THE PACKAGE
	memcpy(pTxData,data.PAYLOAD_DATA,APP_PAYLOAD_LENGTH);

	// WAIT CONNECT:loop not working correctly
	/*while(device_status == WAIT_CONNECT ) {
	    // Send The package
	    basicRfSendPacket(PC_ADDR, pTxData, APP_PAYLOAD_LENGTH);
        printf("Wait connect\n");
	    RF_Timeout_Status = RF_Timeout(5000);
	    if ( (basicRfReceive(pRxData, APP_PAYLOAD_LENGTH,NULL)  >0) && (RF_Timeout_Status == FALSE)  ) {
	        printf("Enter first if\n");
	    	if ( (pRxData[0] == 0x01) &&  (pRxData[29] == 0x04)  && (pRxData[11] == ACK_RECEIVER) ){
	          printf("Enter second if\n");
	    	  valid_package = 0x01;
	          RF_Timeout_Status = TRUE;
	          memset(pRxData,0,APP_PAYLOAD_LENGTH);
	          memset(pTxData,0,APP_PAYLOAD_LENGTH);
	          // Prepare the structure for the next status
	          RF_Package_Init(&data);
	          data.PAYLOAD_DATA_SENSOR[0]=ACK_WD;
	          RF_Package_Compose(&data);
	          memcpy(pTxData,data.PAYLOAD_DATA,APP_PAYLOAD_LENGTH);
	           // Move the device to the new status
	          device_status = WD_CONNECTED;
	        }
	    }
	    else {
	    	printf("Enter else\n");
	        valid_package = 0x00;
	        device_status = WAIT_CONNECT;
	        SleepTimerInit(15);
	        halMcuSetLowPowerMode(2);
	    }
	  }*/
	printf("leave while\n");
	// WAIT CONNECTED
	while (device_status == WD_CONNECTED ) {

		basicRfSendPacket(PC_ADDR, pTxData, APP_PAYLOAD_LENGTH);
		printf("Wait connected\n");
	    RF_Timeout_Status = RF_Timeout(5000);
	    if ( (basicRfReceive(pRxData, APP_PAYLOAD_LENGTH,NULL)  >0) && (RF_Timeout_Status == FALSE)  ) {
	      if ( (pRxData[0] == 0x01) &&  (pRxData[29] == 0x04)  && (pRxData[11] == ACK_RECEIVER2)) {
	       valid_package = 0x01;
	       RF_Timeout_Status = TRUE;
	       memset(pRxData,0,APP_PAYLOAD_LENGTH);
	       memset(pTxData,0,APP_PAYLOAD_LENGTH);
	       // Prepare the structure for the next status
	       RF_Package_Init(&data);
	       data.PAYLOAD_DATA_SENSOR[0]=WD_PKG_DETECTED;
	       RF_Package_Compose(&data);
	       memcpy(pTxData,data.PAYLOAD_DATA,APP_PAYLOAD_LENGTH);
	       device_status = ALMOUST_READY;
	       }
	     }
	    else {
	      valid_package = 0x00;
	      device_status = WD_CONNECTED;
	      retry_counter = retry_counter + 1;
	      if (retry_counter == 200) goto dev_sleep;
	     }
	}

	// ALMOUST READY
	while (device_status == ALMOUST_READY ) {

		RF_Timeout_Status = RF_Timeout(5000);
		printf("Almost ready\n");
		if (( basicRfReceive(pRxData, APP_PAYLOAD_LENGTH,NULL)  >0 ) && (RF_Timeout_Status == FALSE))  {
	    	if ( (pRxData[0] == 0x01) &&  (pRxData[29] == 0x04)  && (pRxData[11] == WD_START_REQ))  {
	    		valid_package = 0x01;
	            RF_Timeout_Status = TRUE;
	            // Send The package
	            basicRfSendPacket(PC_ADDR, pTxData, APP_PAYLOAD_LENGTH);
	            memset(pTxData,0,APP_PAYLOAD_LENGTH);
	            memset(pRxData,0,APP_PAYLOAD_LENGTH);
	            // Prepare the structure for the next status
	            RF_Package_Init(&data);
	            device_status = READY;
	            retry_counter =0;
	          }
	      }
	    else {
	    	valid_package = 0x00;
	        device_status = ALMOUST_READY;
	        retry_counter = retry_counter + 1;
	        if (retry_counter == 1000) goto dev_sleep;
	    }
	}

	// READY
	while (device_status == READY) {
		RF_Timeout_Status = RF_Timeout(5000);
		printf("Ready\n");
	    // SLEEP
	    if (( basicRfReceive(pRxData, APP_PAYLOAD_LENGTH,NULL)  >0 ) && (RF_Timeout_Status == FALSE))  {
	    	if ( (pRxData[0] == 0x01) &&  (pRxData[29] == 0x04)  && (pRxData[11] == SLEEP_MESSAGE)) {
	    		valid_package = 0x01;
	            RF_Timeout_Status = TRUE;
	            memset(pRxData,0,APP_PAYLOAD_LENGTH);
	            data.PAYLOAD_DATA_SENSOR[0]=SLEEP_ACK;
	            RF_Package_Compose(&data);
	            // COPY OF THE PACKAGE
	            memcpy(pTxData,data.PAYLOAD_DATA,APP_PAYLOAD_LENGTH);
	            // Send The package
	            basicRfSendPacket(PC_ADDR, pTxData, APP_PAYLOAD_LENGTH);
	            device_status = SLEEP;
	            retry_counter =0;
	        }

	        if ( (pRxData[0] == 0x01) &&  (pRxData[29] == 0x04)  && (pRxData[11] == ENABLE_ACQUISITION )  ){
	            valid_package = 0x01;
	            RF_Timeout_Status = TRUE;
	            SPI1_Switch_SSN(2,HIGH);
	            memset(pRxData,0,APP_PAYLOAD_LENGTH);
	            memcpy(data.PAYLOAD_DATA_SENSOR,coefficients_stored,16);
	            RF_Package_Compose(&data);
	            // COPY OF THE PACKAGE
	            memcpy(pTxData,data.PAYLOAD_DATA,APP_PAYLOAD_LENGTH);
	            // Send The package
	            basicRfSendPacket(PC_ADDR, pTxData, APP_PAYLOAD_LENGTH);
	            RF_Package_Init(&data);
	            halMcuWaitUs(2000);
	            device_status = ENABLE_ACQ;
	            retry_counter =0;
	        }
	        // Insert Here all the device Mode

	    }

	    else {
	    	valid_package = 0x00;
	        device_status = READY;
	        retry_counter = retry_counter + 1;
	        if (retry_counter == 400) goto dev_sleep;
	    }

	}


	// SLEEP
	while (device_status == SLEEP ) {
		printf("Sleep\n");
	  dev_sleep:
	  halMcuReset();
	}



	//send radio message
	while(1){
		printf("LEDs\n");
		leds_on(LED1|LED3);
		delay();
		leds_on(LED2);
		delay();
		leds_off(LED2);
		delay();
		//printf("Hello World\n");
		/*#if TRANSMITTER
	  	  send_message("Hello");
		#endif
		#if RECEIVER
	  	  receive_message();
		#endif*/

	}
}

bool  RF_Timeout(uint16 us_to_wait){
  bool Timeout_Expired = FALSE;
  while (!basicRfPacketIsReady()) {
    us_to_wait = us_to_wait - 1;
    halMcuWaitUs(1);
    if (us_to_wait == 0 ) {
    Timeout_Expired = TRUE;
    break;
    }
  }
  return  Timeout_Expired;
}
