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

static const int TIMEOUT = 15 * 1000;
static const int WRITE_ENDPOINT = 0x02;
static const int READ_ENDPOINT = 0x81;

int cm15a_send_command(x10_device_t* device, x10_command_t* command);
int cm15a_set_addr(x10_controller_t* device, x10_addr_t addr);
int cm15a_send_data(x10_controller_t* device, uint8_t* data, unsigned int size);

int cm15a_create_backend(x10_backend_t* backend){
	backend->send_command = cm15a_send_command;
	
	return x10_OK;
}

int cm15a_send_command(x10_device_t* device, x10_command_t* command){
	if ( !cm15a_set_addr(device->controller, device->addr) ){
		return x10_set_error("cm15a: failed to set address.");
	};
	if ( !cm15a_send_data(device->controller, &command->header, command->size) ){
		return x10_set_error("cm15a: failed to send function.");
	}
	
	sleep(2);
	
	return x10_OK;
}

int cm15a_set_addr(x10_controller_t* controller, x10_addr_t addr){
	uint8_t data[] = {0x04, addr.value};
	return cm15a_send_data(controller, data, 2);
}

int cm15a_send_data(x10_controller_t* controller, uint8_t* data, unsigned int size){
	char reply[10];
	char checksum = 0x55;
	
	char buf[size];
	printf(" -> ");
	for ( int i = 0; i < size; i++ ){
		uint8_t v = data[i];
		printf(" %02x", v);
#ifdef WORDS_BIGENDIAN
		buf[i] = (v>>4) | (v<<4);
#else /* WORDS_BIGENDIAN */
		buf[i] = v;
#endif /* WORDS_BIGENDIAN */
	}
	printf("\n");
	
	if ( usb_interrupt_write(controller->usb.handle, WRITE_ENDPOINT, buf, size, TIMEOUT) < 0 ){ printf("write failed\n");}
	int n = usb_interrupt_read(controller->usb.handle, READ_ENDPOINT, reply, 10, TIMEOUT);

	if ( n > 0 ){
		printf(" <- ");
		for ( int i = 0; i < n; i++ ){
			printf(" %02x", reply[i]);
		}
		printf("\n");
		return x10_OK;
	} else {
		printf("read error\n");
		return x10_ERROR;
	}
}
