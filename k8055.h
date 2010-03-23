#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <usb.h>
#include <assert.h>

/* prototypes */
int OpenDevice(long board_address);
int CloseDevice();
long ReadAnalogChannel(long Channelno);
int ReadAllAnalog(long* data1, long* data2);
int OutputAnalogChannel(long channel, long data);
int OutputAllAnalog(long data1,long data2);
int ClearAllAnalog();
int ClearAnalogChannel(long channel);
int SetAnalogChannel(long channel);
int SetAllAnalog();
int WriteAllDigital(long data);
int ClearDigitalChannel(long channel);
int ClearAllDigital();
int SetDigitalChannel(long channel);
int SetAllDigital();
int ReadDigitalChannel(long channel);
long ReadAllDigital();
int ResetCounter(long counternr);
long ReadCounter(long counterno);
int SetCounterDebounceTime(long counterno, long debouncetime);
