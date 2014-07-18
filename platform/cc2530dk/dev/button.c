/*
 * button.c
 *
 *  Created on: Jul 17, 2014
 *      Author: Administrator
 */
#include "cc253x.h"
#include "compiler.h"

void button_init(void) {
	P0SEL = P0SEL & 0xFD;
	P0DIR = P0DIR & 0xFD;
}

