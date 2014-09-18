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

#include <hal_mcu.h>
#include <clock.h>
#include <timer1.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hal_rf.h>
#include <cc253x_rf.h>
#include <hal_rf.h>
#include <rf_pack.h>
#include <adc.h>
#include <pwr_mode_manager.h>
#include <spi_manager.h>
#include <lis331dlh.h>
#include <mpl115a.h>
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





	//send radio message
	while(1){
		leds_on(LED1|LED3);
		delay();
		leds_on(LED2);
		delay();
		leds_off(LED2);
		delay();
#if TRANSMITTER
	  send_message("Hello");
#endif
#if RECEIVER
	  receive_message();
#endif
	}
}
