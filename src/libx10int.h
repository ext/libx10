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

#ifndef __LIBX10INT_H
#define __LIBX10INT_H

#include <usb.h>

static const int VENDOR_ID = 0x0BC7;
static const int PRODUCT_ID = 0x0001;

enum x10_function_t {
	X10_ALLUOFF = 0,
	X10_ALLLON,
	X10_ON,
	X10_OFF,
	X10_DIM,
	X10_BRIGHT,
	X10_ALLLOFF,
	X10_EXTENDEDCODE,
	X10_HAILREQ,
	X10_HAILACK,
	X10_PDIML,
	X10_PDIMH,
	X10_EXTENDDATA,
	X10_STATON,
	X10_STATOFF,
	X10_STATREQ
};

typedef struct {
	uint8_t size;
	uint8_t header;
	
	uint8_t code : 4;
	uint8_t housecode : 4;
	
	char data[0];
} x10_command_t;

typedef struct {
	int (*send_command)(x10_device_t* device, x10_command_t* command);
} x10_backend_t;

struct _x10_controller {
	struct {
		struct usb_device* device;
		struct usb_dev_handle* handle;
	} usb;
	
	x10_backend_t backend;
};

struct _x10_device {
	x10_controller_t* controller;
	enum x10_device_type_t type;
	x10_addr_t addr;
	union {
		float fval;
		int ival;
	};
};

typedef struct {
	const char* str;
} x10_error_t;

x10_addr_t x10_get_addr(const char* addr);

struct usb_device* x10_find_device(struct usb_bus* busses, int offset);

int cm15a_create_backend(x10_backend_t* backend);

x10_command_t* x10_command_function(enum x10_function_t function, uint8_t housecode, float dim, uint8_t n, ...);

enum x10_event_type_t x10_decode_message(x10_event_t* event, uint8_t size, uint8_t* message);
void x10_decode_message_rf(x10_event_t* event, uint8_t size, uint8_t* message);

int x10_set_error(const char* fmt, ...);

#endif /* __LIBX10INT_H */
