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

#include <stdio.h>
#include <unistd.h>
#include <libx10.h>

int cm15a_send_data(x10_controller_t* controller, int8_t* data, unsigned int size);

int main(int argc, const char* argv[]){
	x10_controller_t* controller = x10_controller_open(X10_CM15A);
	
	if ( !controller ){
		fprintf(stderr, "Error: %s\n", x10_get_error());
		return 0;
	}
	
	x10_device_t* relay = x10_device_create(controller, x10_addr(X10_HOUSE_A, 3), X10_RELAY);
	x10_device_t* dim = x10_device_create(controller, x10_addr_from_string("A2"), X10_DIMMER);
	
	int n = argc-1;
	
	if ( n == 0 ){
		printf("need args\n");
		return 1;
	}
	
	int8_t data[n];
	for ( int i = 1; i < argc; i++ ){
		sscanf(argv[i], "%x", (int*)&data[i-1]);
	}
	
	int pos = 0;
	do {
		int size = data[pos];
		cm15a_send_data(controller, &data[pos + 1], size);
		pos += size + 1;
	} while ( pos < n );
	
	x10_device_release(relay);
	x10_device_release(dim);
	
	x10_controller_close(controller);
	
	return 0;
}
