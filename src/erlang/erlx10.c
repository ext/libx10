#include <stdio.h>
#include <string.h>
#include <libx10.h>
#include "defines.h"
#include <erl_driver.h>

typedef struct {
	ErlDrvPort port;
	x10_controller_t* controller;
	unsigned int nr_of_devices;
	unsigned int capacity;
	x10_device_t** devices;
} erlx10_data;

static unsigned int create_device(erlx10_data* d, char type, char addr_str[3]);

static void allocate_array(erlx10_data* d, unsigned int new_capacity){
	d->capacity = new_capacity;
	size_t size = d->capacity * sizeof(x10_device_t*);
	d->devices = realloc(d->devices, size);
}

static ErlDrvData erlx10_start(ErlDrvPort port, char *buff){
	printf("start\n");

	x10_controller_t* controller = x10_controller_open(X10_CM15A);

	//if ( !controller ){
	//	fprintf(stderr, "Error: %s\n", x10_get_error());
	//	return NULL;
	//}

	erlx10_data* d = (erlx10_data*)driver_alloc(sizeof(erlx10_data));

	d->port = port;
	d->controller = controller;
	d->nr_of_devices = 0;
	d->capacity = 0;
	d->devices = NULL;
	allocate_array(d, 10);

	return (ErlDrvData)d;
}

static void erlx10_stop(ErlDrvData handle){
	printf("stop\n");
	erlx10_data* d = (erlx10_data*)handle;

	//x10_device_t* device = d->devices[0];
	//while ( device ){
	//	x10_device_release(device);
	//	device++;
	//}

	x10_controller_close(d->controller);

	driver_free((char*)handle);
}

static int return_ok(erlx10_data* d, char* ret, unsigned int retlen){
	ret[0] = 0;
	return driver_output(d->port, ret, 1 + retlen);
}

static void erlx10_output(ErlDrvData handle, char *buff, int bufflen){
	erlx10_data* d = (erlx10_data*)handle;

	// @todo error handling
	char command = buff[0];
	char ret[10];
	unsigned int retlen = 0;

	switch ( command ){
		case CMD_DEVICE_CREATE:
			printf("creating new device\n");

			// @todo error handling
			char type = buff[1];
			char addr[3] = {buff[2], buff[3], 0};
			unsigned int id = create_device(d, type, addr);
			ret[1] = id;
			retlen = 1;
			break;
		default:
			printf("unhandled command\n");
	}

	return_ok(d, ret, retlen);
}

static unsigned int create_device(erlx10_data* d, char type, char addr_str[3]){
	x10_addr_t addr = x10_addr_from_string(addr_str);
	x10_device_t* device = NULL;

	switch ( type ){
		case DEVICE_TYPE_RELAY:
			device = x10_device_create(d->controller, addr, X10_RELAY);
			break;
		case DEVICE_TYPE_DIMMER:
			device = x10_device_create(d->controller, addr, X10_DIMMER);
			break;
	}

	// @todo error handling
	unsigned int id = d->nr_of_devices++;
	d->devices[id] = device;

	return id;
}

ErlDrvEntry erlx10_entry = {
	NULL,                       /* F_PTR init, N/A */
	erlx10_start,          /* L_PTR start, called when port is opened */
	erlx10_stop,           /* F_PTR stop, called when port is closed */
	erlx10_output,         /* F_PTR output, called when erlang has sent */
	NULL,                       /* F_PTR ready_input, called when input descriptor ready */
	NULL,                       /* F_PTR ready_output, called when output descriptor ready */
	"erlx10",              /* char *driver_name, the argument to open_port */
	NULL,                       /* F_PTR finish, called when unloaded */
	NULL,                       /* F_PTR control, port_command callback */
	NULL,                       /* F_PTR timeout, reserved */
	NULL                        /* F_PTR outputv, reserved */
};

DRIVER_INIT(erlx10) /* must match name in driver_entry */{
	return &erlx10_entry;
}
