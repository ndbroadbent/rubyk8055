/*
   This file is part of the libk8055 Library.

   The libk8055 Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The libk8055 Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   http://opensource.org/licenses/

   Copyleft (C) 2005 by Sven Lindberg
     k8055@k8055.mine.nu

   Copyright (C) 2007 by Pjetur G. Hjaltason
       pjetur@pjetur.net
       Commenting, general rearrangement of code, bugfixes,
       python interface with swig and simple k8055 python class


	Input packet format

	+---+---+---+---+---+---+---+---+
	|DIn|Sta|A1 |A2 |   C1  |   C2  |
	+---+---+---+---+---+---+---+---+
	DIn = Digital input in high nibble, except for input 3 in 0x01
	Sta = Status,x01 = OK ?
	A1  = Analog input 1, 0-255
	A2  = Analog input 2, 0-255
	C1  = Counter 1, 16 bits (lsb)
	C2  = Counter 2, 16 bits (lsb)

	Output packet format

	+---+---+---+---+---+---+---+---+
	|CMD|DIG|An1|An2|Rs1|Rs2|Dbv|Dbv|
	+---+---+---+---+---+---+---+---+
	CMD = Command
	DIG = Digital output bitmask
	An1 = Analog output 1 value, 0-255
	An2 = Analog output 2 value, 0-255
	Rs1 = Reset counter 1, command 3
	Rs2 = Reset counter 3, command 4
	Dbv = Debounce value for counter 1 and 2, command 1 and 2

	Or split by commands

	Cmd 0, Reset ??
	Cmd 1, Set debounce Counter 1
	+---+---+---+---+---+---+---+---+
	|CMD|   |   |   |   |   |Dbv|   |
	+---+---+---+---+---+---+---+---+
	Cmd 2, Set debounce Counter 2
	+---+---+---+---+---+---+---+---+
	|CMD|   |   |   |   |   |   |Dbv|
	+---+---+---+---+---+---+---+---+
	Cmd 3, Reset counter 1
	+---+---+---+---+---+---+---+---+
	| 3 |   |   |   | 00|   |   |   |
	+---+---+---+---+---+---+---+---+
	Cmd 4, Reset counter 2
	+---+---+---+---+---+---+---+---+
	| 4 |   |   |   |   | 00|   |   |
	+---+---+---+---+---+---+---+---+
	cmd 5, Set analog/digital
	+---+---+---+---+---+---+---+---+
	| 5 |DIG|An1|An2|   |   |   |   |
	+---+---+---+---+---+---+---+---+

**/


#include "k8055.h"
#include <math.h>

#define STR_BUFF 256
#define PACKET_LEN 8

#define K8055_IPID 0x5500
#define VELLEMAN_VENDOR_ID 0x10cf

#define USB_OUT_EP 0x01	/* USB output endpoint */
#define USB_INP_EP 0x81 /* USB Input endpoint */

#define USB_TIMEOUT 20
#define K8055_ERROR -1

#define DIGITAL_INP_OFFSET 0
#define DIGITAL_OUT_OFFSET 1
#define ANALOG_1_OFFSET 2
#define ANALOG_2_OFFSET 3
#define COUNTER_1_OFFSET 4
#define COUNTER_2_OFFSET 6

#define CMD_RESET 0x00
#define CMD_SET_DEBOUNCE_1 0x01
#define CMD_SET_DEBOUNCE_2 0x01
#define CMD_RESET_COUNTER_1 0x03
#define CMD_RESET_COUNTER_2 0x04
#define CMD_SET_ANALOG_DIGITAL 0x05

/* set debug to 0 to not print excess info */
int DEBUG = 1;

/* variables for usb */
static struct usb_bus *bus, *busses;
static struct usb_device *dev;
static usb_dev_handle *device_handle = '\0';

/* globals for datatransfer */
unsigned char data_in[PACKET_LEN+1], data_out[PACKET_LEN+1];

/* char* device_id[]; */

static int ReadK8055Data(void)
{
    int read_status = 0, i = 0;

    for(i=0; i < 3; i++)
        {
        read_status = usb_interrupt_read(device_handle, USB_INP_EP, (char *)data_in, PACKET_LEN, USB_TIMEOUT);
        if ((read_status == PACKET_LEN) && (data_in[1] & 0x01)) return 0;
        if (DEBUG)
            fprintf(stderr, "Read retry\n");
        }
    return K8055_ERROR;
}

static int WriteK8055Data(unsigned char cmd)
{
    int write_status = 0, i = 0;

    data_out[0] = cmd;
    for(i=0; i < 3; i++)
        {
	/* usb_interrupt_write requires 16-bit output, USB1.1 uses 8-bit. a small "feature" gained with USB2.0 */
        write_status = usb_interrupt_write(device_handle, USB_OUT_EP, (int *)data_out, PACKET_LEN, USB_TIMEOUT);
        if((write_status == PACKET_LEN) && (ReadK8055Data() == 0)) return 0;
        if (DEBUG)
            fprintf(stderr, "Write retry\n");
        }
    return K8055_ERROR;
}

static int takeover_device(usb_dev_handle * udev, int interface)
{
    char driver_name[STR_BUFF];

    memset(driver_name, 0, STR_BUFF);
    int ret = K8055_ERROR;

    assert(udev != NULL);
    ret = usb_get_driver_np(udev, interface, driver_name, sizeof(driver_name));
    if (ret == 0)
    {
        if (DEBUG)
            fprintf(stderr, "Got driver name: %s\n", driver_name);
        if (0 > usb_detach_kernel_driver_np(udev, interface))
        {
            if (DEBUG)
                fprintf(stderr, "Disconnect OS driver: %s\n", usb_strerror());
        }
        else if (DEBUG)
            fprintf(stderr, "Disconnected OS driver: %s\n", usb_strerror());
    }
    else if (DEBUG)
        fprintf(stderr, "get driver name: %s\n", usb_strerror());

    /* claim interface */
    if (usb_claim_interface(udev, interface) < 0)
    {
        if (DEBUG)
            fprintf(stderr, "Claim interface error: %s\n", usb_strerror());
        return K8055_ERROR;
    }
    else
        usb_set_altinterface(udev, interface);
    usb_set_configuration(udev, 1);

    if (DEBUG)
        {
        fprintf(stderr, "Found interface %d\n", interface);
        fprintf(stderr, "Took over the device\n");
        }

    return 0;
}

int OpenDevice(long board_address)
{
    unsigned char located = 0;
    int ipid;

    /* init USB and find all of the devices on all busses */
    usb_init();
    usb_find_busses();
    usb_find_devices();
    busses = usb_get_busses();

    /* ID of the welleman board is 5500h + address config */
    if (board_address >= 0 && board_address < 4) {
        ipid = K8055_IPID + (int)board_address;
    } else {
        fprintf(stderr, "Invalid board address: %ld. Must be between 0-3.\n", board_address);
        return K8055_ERROR;              /* throw error instead of being nice */
    }
    /* start looping through the devices to find the correct one */
    for (bus = busses; bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if ((dev->descriptor.idVendor == VELLEMAN_VENDOR_ID) &&
                (dev->descriptor.idProduct == ipid))
            {
                located++;
                device_handle = usb_open(dev);
                if (DEBUG)
                    fprintf(stderr,
                            "Velleman Device Found @ Address %s Vendor 0x0%x Product ID 0x0%x\n",
                            dev->filename, dev->descriptor.idVendor,
                            dev->descriptor.idProduct);
                if (takeover_device(device_handle, 0) < 0)
                {
                    if (DEBUG)
                        fprintf(stderr,
                                "Can not take over the device from the OS driver\n");
                    usb_close(device_handle);   /* close usb if we fail */
                    return K8055_ERROR;  /* throw K8055_ERROR to show that OpenDevice failed */
                }
                else
                {
                    memset(data_out,0,8);	/* Write cmd 0, read data */
                    return WriteK8055Data(CMD_RESET);
                }
            }
        }
    }
    if (DEBUG)
        fprintf(stderr, "Could not find velleman k8055 with address %d\n",
                (int)board_address);
    return K8055_ERROR;
}

int CloseDevice()
{
    return usb_close(device_handle);
}

long ReadAnalogChannel(long channel)
{
    if (channel == 1 || channel == 2)
    {
        if ( ReadK8055Data() == 0)
        {
            if (channel == 2)
                return data_in[ANALOG_2_OFFSET];
            else
                return data_in[ANALOG_1_OFFSET];
        }
        else
            return K8055_ERROR;
    }
    else
        return K8055_ERROR;
}

int ReadAllAnalog(long *data1, long *data2)
{
    if (ReadK8055Data() == 0)
    {
        *data1 = data_in[ANALOG_1_OFFSET];
        *data2 = data_in[ANALOG_2_OFFSET];
        return 0;
    }
    else
        return K8055_ERROR;
}

int OutputAnalogChannel(long channel, long data)
{
    if (channel == 1 || channel == 2)
    {
        if (channel == 2)
            data_out[ANALOG_2_OFFSET] = (unsigned char)data;
        else
            data_out[ANALOG_1_OFFSET] = (unsigned char)data;

        return WriteK8055Data(CMD_SET_ANALOG_DIGITAL);
    }
    else
        return K8055_ERROR;
}

int OutputAllAnalog(long data1, long data2)
{
    data_out[2] = (unsigned char)data1;
    data_out[3] = (unsigned char)data2;

    return WriteK8055Data(CMD_SET_ANALOG_DIGITAL);
}

int ClearAllAnalog()
{
    return OutputAllAnalog(0, 0);
}

int ClearAnalogChannel(long channel)
{
    if (channel == 1 || channel == 2)
    {
        if (channel == 2)
            return OutputAnalogChannel(2, 0);
        else
            return OutputAnalogChannel(1, 0);
    }
    else
        return K8055_ERROR;
}

int SetAnalogChannel(long channel)
{
    if (channel == 1 || channel == 2)
    {
        if (channel == 2)
            return OutputAnalogChannel(2, 0xff);
        else
            return OutputAnalogChannel(1, 0xff);
    }
    else
        return K8055_ERROR;

}

int SetAllAnalog()
{
    return OutputAllAnalog(0xff, 0xff);
}

int WriteAllDigital(long data)
{
    data_out[1] = (unsigned char)data;
    return WriteK8055Data(CMD_SET_ANALOG_DIGITAL);
}

int ClearDigitalChannel(long channel)
{
    unsigned char data;

    if (channel > 0 && channel < 9)
    {
        data = data_out[1] ^ (1 << (channel-1));
        return WriteAllDigital(data);
    }
    else
        return K8055_ERROR;
}

int ClearAllDigital()
{
    return WriteAllDigital(0x00);
}

int SetDigitalChannel(long channel)
{
    unsigned char data;

    if (channel > 0 && channel < 9)
    {
        data = data_out[1] | (1 << (channel-1));
        return WriteAllDigital(data);
    }
    else
        return K8055_ERROR;
}

int SetAllDigital()
{
    return WriteAllDigital(0xff);
}

int ReadDigitalChannel(long channel)
{
    int rval;
    if (channel > 0 && channel < 9)
    {
        if ((rval = ReadAllDigital()) == K8055_ERROR) return K8055_ERROR;
        return ((rval & (1 << (channel-1))) > 0);
    }
    else
        return K8055_ERROR;
}

long ReadAllDigital()
{
    int return_data = 0;

    if (ReadK8055Data() == 0)
    {
	return_data = (
		((data_in[0] >> 4) & 0x03) |  /* Input 1 and 2 */
		((data_in[0] << 2) & 0x04) |  /* Input 3 */
		((data_in[0] >> 3) & 0x18) ); /* Input 4 and 5 */

        return return_data;
    }
    else
        return K8055_ERROR;
}

int ReadAllValues(long int *data1, long int * data2, long int * data3, long int * data4, long int * data5)
{
    if (ReadK8055Data() == 0)
    {
	*data1 = (
		((data_in[0] >> 4) & 0x03) |  /* Input 1 and 2 */
		((data_in[0] << 2) & 0x04) |  /* Input 3 */
		((data_in[0] >> 3) & 0x18) ); /* Input 4 and 5 */
        *data2 = data_in[ANALOG_1_OFFSET];
        *data3 = data_in[ANALOG_2_OFFSET];
        *data4 = *((short int *)(&data_in[COUNTER_2_OFFSET]));
        *data5 = *((short int *)(&data_in[COUNTER_1_OFFSET]));
 	return 0;
    }
    else
        return K8055_ERROR;
}

int ResetCounter(long counterno)
{
    if (counterno == 1 || counterno == 2)
    {
        data_out[0] = 0x02 + (unsigned char)counterno;  /* counter selection */
        data_out[3 + counterno] = 0x00;
        return WriteK8055Data(data_out[0]);
    }
    else
        return K8055_ERROR;
}

long ReadCounter(long counterno)
{
    if (counterno == 1 || counterno == 2)
    {
        if (ReadK8055Data() == 0)
        {
            if (counterno == 2)
                return *((short int *)(&data_in[COUNTER_2_OFFSET]));
            else
                return *((short int *)(&data_in[COUNTER_1_OFFSET]));
        }
        else
            return K8055_ERROR;
    }
    else
        return K8055_ERROR;
}

int SetCounterDebounceTime(long counterno, long debouncetime)
{
    float value;

    if (counterno == 1 || counterno == 2)
    {
        data_out[0] = (unsigned char)counterno;
        /* the velleman k8055 use a exponetial formula to split up the
           debouncetime 0-7450 over value 1-255. I've tested every value and
           found that the formula dbt=0,338*value^1,8017 is closest to
           vellemans dll. By testing and measuring times on the other hand I
           found the formula dbt=0,115*x^2 quite near the actual values, a
           little below at really low values and a little above at really
           high values. But the time set with this formula is within +-4% */
        if (debouncetime > 7450)
            debouncetime = 7450;
        value = sqrtf(debouncetime / 0.115);
        if (value > ((int)value + 0.49999999))  /* simple round() function) */
            value += 1;
        data_out[5 + counterno] = (unsigned char)value;
        if (DEBUG)
            fprintf(stderr, "Debouncetime%d value for k8055:%d\n",
                    (int)counterno, data_out[5 + counterno]);
        return WriteK8055Data(data_out[0]);
    }
    else
        return K8055_ERROR;
}

