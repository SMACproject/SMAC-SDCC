/*
 * leds-arch.c
 *
 * Created on: Jun 26, 2014
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

#include "cc253x.h"
#include "dev/led.h"

#define LED1_PIN	P1_0
#define LED2_PIN	P1_1
#define LED3_PIN	P1_4

void leds_arch_init(void) {
	P1SEL = P1SEL & 0xFE;
	P1DIR = P1DIR | 0x01;//P1SEL selects function P1DIR selects I/O

	P1SEL = P1SEL & 0xFD;
	P1DIR = P1DIR | 0x02;

	P1SEL = P1SEL & 0xEF;
	P1DIR = P1DIR | 0x10;
}
  void
leds_arch_on(unsigned char leds)
{
  LED1_PIN = leds & 0x01;
  LED2_PIN = (leds & 0x02) >> 1;
  LED3_PIN = (leds & 0x04) >> 2;
}
/*---------------------------------------------------------------------------*/

