/**
 * Copyright (c) 2009, David Sveningsson
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Blekinge Institute of Technology nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __LIBX10_H
#define __LIBX10_H

#include <stdint.h>

enum {
	x10_OK = 1,
	x10_ERROR = 0
};

enum x10_protocol_t {
	X10_CM15A
};

enum x10_device_type_t {
	X10_RELAY = (1 << 0),
	X10_DIMMER = (1 << 1)
};

enum x10_housecode_t {
	X10_HOUSE_A = 0,
	X10_HOUSE_B,
	X10_HOUSE_C,
	X10_HOUSE_D,
	X10_HOUSE_E,
	X10_HOUSE_F,
	X10_HOUSE_G,
	X10_HOUSE_H,
	X10_HOUSE_I,
	X10_HOUSE_J,
	X10_HOUSE_L,
	X10_HOUSE_M,
	X10_HOUSE_N,
	X10_HOUSE_O,
	X10_HOUSE_P
};

enum x10_event_type_t {
	X10_NO_EVENT = 0x00,
	X10_POWERLINE_EVENT = 0x5a,
	X10_MACRO_EVENT = 0x5b,
	X10_RF_EVENT = 0x5d,
	X10_UNKNOWN_EVENT = 0xff
};

enum x10_rf_event_type_t {
	X10_RF_ON,
	X10_RF_OFF,
	X10_RF_DIM,
	X10_RF_BRIGHT
};

typedef struct _x10_controller x10_controller_t;
typedef struct _x10_device x10_device_t;

typedef union {
	struct {
		uint8_t devicecode : 4;
		uint8_t housecode : 4;
	} bits ;
	uint8_t  value;
} x10_addr_t;

typedef struct {
	enum x10_event_type_t type;
	
	union {
		struct {
			x10_addr_t addr;
		} powerline;
		
		struct {
			x10_addr_t addr;
			enum x10_rf_event_type_t type;
		} rf;
	};
} x10_event_t;

x10_controller_t* x10_controller_open(enum x10_protocol_t protocol);
int x10_controller_close(x10_controller_t* controller);

x10_addr_t x10_addr(enum x10_housecode_t hc, unsigned int dc);
x10_addr_t x10_addr_from_string(const char* str);
const char* x10_addr_to_string(x10_addr_t addr, char buf[4]);

x10_device_t* x10_device_create(x10_controller_t* controller, x10_addr_t addr, enum x10_device_type_t type);
int x10_device_on(x10_device_t* device);
int x10_device_off(x10_device_t* device);
int x10_device_set(x10_device_t* device, unsigned int s);
int x10_device_dim_abs(x10_device_t* device, float s);
int x10_device_dim_delta(x10_device_t* device, float s);
int x10_device_release(x10_device_t* device);

int x10_poll_event(x10_controller_t* controller, x10_event_t* event);

const char* x10_get_error();

#endif /* __LIBX10_H */
