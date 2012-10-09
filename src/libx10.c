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

#include "config.h"
#include "libx10.h"
#include "libx10int.h"

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

static const char to_x10_code[16] = {0x6, 0xE, 0x2, 0xA, 0x1, 0x9, 0x5, 0xD, 0x7, 0xF, 0x3, 0xB, 0x0, 0x8, 0x4, 0xc };
static const char from_x10_code[16] = {0xC, 0x4, 0x2, 0xA, 0xE, 0x6, 0x0, 0x8, 0xD, 0x5, 0x3, 0xB, 0xF, 0x7, 0x1, 0x9};

static x10_error_t x10_error;

int is_x10_device(struct usb_device* dev){
	return dev->descriptor.idVendor == VENDOR_ID && dev->descriptor.idProduct == PRODUCT_ID;
}

struct usb_device* x10_find_device(struct usb_bus* busses, int offset){
	for( struct usb_bus* bus = busses; bus; bus = bus->next ){
		for( struct usb_device* dev = bus->devices; dev; dev = dev->next ){
			if ( !is_x10_device(dev) ){
				continue;
			}
			
			if ( offset-- > 0 ){
				continue;
			}
			
			return dev;
		}
	}
	
	return 0;
}

x10_controller_t* x10_controller_open(enum x10_protocol_t protocol){
	struct usb_bus* busses;
	
	usb_init();
	usb_find_busses();
	usb_find_devices();
	
	busses = usb_get_busses();
	
	struct usb_device* dev = x10_find_device(busses, 0);
	
	if ( !dev ){
		x10_set_error("Failed to initialize libusb.");
		return 0;
	}
	
	usb_dev_handle* handle = usb_open(dev);
	
	if ( !handle ){
		x10_set_error("Failed to open usb device.");
		return 0;
	}
	
	if ( usb_claim_interface(handle, 0) != 0 ){
		x10_set_error("Failed to claim usb device.");
	}
	
	x10_controller_t* controller = (x10_controller_t*)malloc(sizeof(x10_controller_t));
	
	controller->usb.device = dev;
	controller->usb.handle = handle;
	
	switch ( protocol ){
		case X10_CM15A:
			cm15a_create_backend(&controller->backend); break;
		default:
			x10_set_error("Unknown backend (0x%02x).", protocol);
			return 0;
	}
	
	return controller;
}

int x10_controller_close(x10_controller_t* controller){
	if ( !controller ){
		return x10_OK;
	}
	
	if ( !usb_release_interface(controller->usb.handle, 0) ){
		return x10_set_error("Failed to release usb interface.");
	}
	
	usb_close(controller->usb.handle);
	free(controller);
	
	return x10_OK;
}

x10_addr_t x10_addr(enum x10_housecode_t hc, unsigned int dc){
	// This is fscked but the order is random to me. See CM15a documentation.
	x10_addr_t tmp;
	tmp.bits.housecode = to_x10_code[hc];
	tmp.bits.devicecode = to_x10_code[dc-1];
	
	return tmp;
}

x10_addr_t x10_addr_from_string(const char* str){
	// @note Validate string
	
	char hc;
	int dc;
	
	sscanf(str, "%c%d", &hc, &dc);
	
	// Convert to lowercase and subtract from 'a' to get numerical value
	hc = tolower(hc) - 'a';
	
	return x10_addr(hc, dc);
}

const char* x10_addr_to_string(x10_addr_t addr, char buf[4]){
	char hc = from_x10_code[addr.bits.housecode] + 'A';
	int dc = from_x10_code[addr.bits.devicecode] + 1;
	sprintf(buf, "%c%d", hc, dc);
	
	return buf;
}

int x10_set_error(const char* fmt, ...){
	char* buf;
	va_list arg;
	va_start(arg, fmt);
	vasprintf(&buf, fmt, arg);
	va_end(arg);
	
	x10_error.str = buf;
	return x10_ERROR;
}

const char* x10_get_error(){
	return x10_error.str;
}

x10_device_t* x10_device_create(x10_controller_t* controller, x10_addr_t addr, enum x10_device_type_t type){
	x10_device_t* device = (x10_device_t*)malloc(sizeof(x10_device_t));
	device->controller = controller;
	device->type = type;
	device->addr = addr;
	device->ival = -1;
	
	return device;
}

int x10_device_release(x10_device_t* device){
	free(device);
	
	return x10_OK;
}

x10_command_t* x10_command_function(enum x10_function_t function, uint8_t housecode, float dim, uint8_t n, ...){
	x10_command_t* command = (x10_command_t*)malloc(sizeof(x10_command_t) + n);

	command->size = 2 + n;
		
	//uint8_t dim_bits = dim * 31;
	
	command->header = 0x06;
	command->housecode = housecode;
	command->code = function;
	
	va_list arg;
	va_start(arg, n);
	int p = 0;
	while ( n > 0 ){
		command->data[p++] = (uint8_t)va_arg(arg, unsigned int);
		n--;
	}
	
	return command;
}

int x10_device_on(x10_device_t* device){
	return device->controller->backend.send_command(device, x10_command_function(X10_ON, device->addr.bits.housecode, 0.0f, 0));
}

int x10_device_off(x10_device_t* device){
	return device->controller->backend.send_command(device, x10_command_function(X10_OFF, device->addr.bits.housecode, 0.0f, 0));
}

int x10_device_set(x10_device_t* device, unsigned int s){
	return device->controller->backend.send_command(device, x10_command_function(s ? X10_ON : X10_OFF, device->addr.bits.housecode, 0.0f, 0));
}

int x10_device_dim_abs(x10_device_t* device, float s){
	if ( device->ival == -1 ){
		if ( !device->controller->backend.send_command(device, x10_command_function(X10_BRIGHT, device->addr.bits.housecode, 1.0f, 1, 0xFF))){
			// Error already set
			return x10_ERROR;
		}
		device->fval = 1.0f;
	}
	
	float delta = s - device->fval;
	return x10_device_dim_delta(device, delta);
}

int x10_device_dim_delta(x10_device_t* device, float s){
	if ( device->fval + s > 1.0f ){
		s = 1.0f - device->fval;
		device->fval = 1.0f;
	} else {
		device->fval += s;
	}
	
	uint8_t sd = (uint8_t)(fabs(s) * 0xFF);
	
	if ( s > 0 ){
		printf("brighten with %f (%02x)\n", s, sd);
		return device->controller->backend.send_command(device, x10_command_function(X10_BRIGHT, device->addr.bits.housecode, s, 1, sd));
	} else {
		printf("dim with %f (%02x)\n", s, sd);
		return device->controller->backend.send_command(device, x10_command_function(X10_DIM, device->addr.bits.housecode, s, 1, sd));
	}
}

int x10_poll_event(x10_controller_t* controller, x10_event_t* event){
	static const unsigned int max_size = 100;
	uint8_t message[max_size];
	
	int n = usb_interrupt_read(controller->usb.handle, 0x81, (char*)message, max_size, 1000);
	
	if ( n > 0 ){
		printf(" <- ");
		for ( int i = 0; i < n; i++ ){
			printf(" %02x", message[i]);
		}
		printf("\n");
		
		if ( x10_decode_message(event, n, message) != X10_NO_EVENT ){
			usleep(1000*100);
			return 1;
		}
	}
	usleep(1000*100);
	
	return 0;
}

enum x10_event_type_t x10_decode_message(x10_event_t* event, uint8_t size, uint8_t* message){
	// Ack
	if ( size == 1 && message[0] == 0x55 ){
		return X10_NO_EVENT;
	}
	
	uint8_t type = message[0];
	
	switch ( type ){
		case 0x5a:
			event->type = X10_POWERLINE_EVENT;
			break;
		case 0x5b:
			event->type = X10_MACRO_EVENT;
			break;
		case 0x5d:
			x10_decode_message_rf(event, size, message);
			break;
		default:
			event->type = X10_UNKNOWN_EVENT;
	}
	
	return event->type != X10_UNKNOWN_EVENT ? 1 : 0;
}

void x10_decode_message_rf(x10_event_t* event, uint8_t size, uint8_t* message){
	assert( size == 6 );
	assert( message[1] == 0x20 );
	
	event->type = X10_RF_EVENT;
	event->rf.addr.bits.housecode = message[2] >> 4;
	
	int is_dim = message[4] & 0x80;
	if( !is_dim ){
		
		// Most fsck'up so far... The bits aren't in order or even in the same BYTE!
		uint8_t dc = (( message[4] >> 4 ) & 0x01) | (( message[4] >> 2 ) & 0x02) | (( message[4] >> 4 ) & 0x04) | (( message[2] << 1 ) & 0x08);
		event->rf.addr.bits.devicecode = to_x10_code[dc];
		
		int flag = message[4] & 0x20;
	}

}
