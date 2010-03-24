#include "ruby.h"
#include "k8055.h"

#include <string.h>
#include <stdio.h>
#include <usb.h>
#include <assert.h>
#include <sys/time.h>

#define STR_BUFF 256
#define false 0
#define true 1

// Set default values.

int delay = 0;

// Defining a space for information and references about the module to be stored internally
VALUE RubyK8055 = Qnil;

// Prototype for the initialization method - Ruby calls this, not you
void Init_rubyk8055();

// ------------------- Validations ---------------------

static int check_connection(VALUE self) {
    if (rb_iv_get(self, "@connected") == true) {
        return true;
    } else {
        printf("Not connected to K8055!\n");
        return false;
    }
}

static int valid_analog_channel(long channel) {
    if (channel == 1 || channel == 2) {
        return true;
    } else {
        printf("Invalid analog channel! Must be 1 or 2 : (%ld)\n", channel);
        return false;
    }
}

static int valid_analog_value(long value) {
    if (value >= 0 && value <= 255) {
        return true;
    } else {
        printf("Invalid analog value! Must be between 0-255 : (%ld)\n", value);
        return false;
    }
}

static int valid_digital_channel(long channel) {
    if (channel >= 1 && channel <= 8) {
        return true;
    } else {
        printf("Invalid digital channel! Must be between 1-8 : (%ld)\n", channel);
        return false;
    }
}

static int valid_counter(long counter) {
    if (counter == 1 || counter == 2) {
        return true;
    } else {
        printf("Invalid counter! Must be 1 or 2 : (%ld)\n", counter);
        return false;
    }
}

// ----------------------------- K8055 Methods ---------------------------

static VALUE method_connect(int argc, VALUE *argv, VALUE self) {
    long board_address = NUM2INT(rb_iv_get(self, "@board_address"));
    // if no args given, connect to board '0'
    if (argc > 0) {
        // Converts ruby fixnum to C data
        board_address = NUM2INT(argv[0]);
    }
    if (rb_iv_get(self, "@connected") == false) {
        if (OpenDevice(board_address) != -1) {
            printf("Connected to K8055 with address: %ld\n", board_address);
            rb_iv_set(self, "@connected", Qtrue);
            // Sets the board address to the connected board.
            rb_iv_set(self, "@board_address", board_address);
            return Qtrue;
        } else {
            printf("Could not connect to K8055 with address: %ld\n", board_address);
            rb_iv_set(self, "@connected", false);
        }
    } else {
        printf("Already connected to K8055!\n");
    }
    return false;
}

static VALUE method_disconnect(VALUE self) {
    if (check_connection(self)) {
        if (CloseDevice() != -1) {
            printf("Closed connection to K8055.\n");
            rb_iv_set(self, "@connected", false);
            return 0;
        } else {
            printf("Could not close connection to K8055.\n");
        }
    }
    return false;
}

static VALUE method_address(VALUE self) {
    if (check_connection(self)) {
        return INT2NUM(rb_iv_get(self, "@board_address")); // int2num returns 0 instead of false.
    }
    return false;
}

static VALUE method_get_analog(VALUE self, long channel) {
    if (check_connection(self)) {
        channel = NUM2INT(channel);
        if (valid_analog_channel(channel)) {
            long data = ReadAnalogChannel(channel);
            if (data != -1)
                return data;
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_analog(VALUE self, long channel, long value) {
    if (check_connection(self)) {
        channel = NUM2INT(channel);
        value = NUM2INT(value);
        if (valid_analog_value(value)) {
            if (valid_analog_channel(channel)) {
                if (OutputAnalogChannel(channel, value) != -1)
                    return true;
            }
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_analog_max(VALUE self, long channel) {
    return method_set_analog(self, channel, 255);
}

static VALUE method_set_analog_min(VALUE self, long channel) {
    return method_set_analog(self, channel, 0);
}

static VALUE method_get_digital(VALUE self, long channel) {
    if (check_connection(self)) {
        channel = NUM2INT(channel);
        if (valid_digital_channel(channel)) {
            long data = ReadDigitalChannel(channel);
            if (data != -1)
                return data;
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_digital(VALUE self, long channel, int value) {
    if (check_connection(self)) {
        channel = NUM2INT(channel);
        if (value != 0) {   // simple boolean conversion from non-zero to true
            value = true;
        }
        if (valid_digital_channel(channel)) {
            if (value == true) {
                if (SetDigitalChannel(channel) != -1)
                    return true;
            } else {
                if (ClearDigitalChannel(channel) != -1)
                    return true;
            }
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_write_all_digital(VALUE self, int value) {
    if (check_connection(self)) {
        value = NUM2INT(value);
        if (WriteAllDigital(value) != -1)
            return true;
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_all_digital(VALUE self) {
    if (check_connection(self)) {
        if (SetAllDigital() != -1)
            return true;
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_clear_all_digital(VALUE self) {
    if (check_connection(self)) {
        if (ClearAllDigital() != -1)
            return true;
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_all_analog(VALUE self) {
    if (check_connection(self)) {
        if (SetAllAnalog() != -1)
            return true;
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_clear_all_analog(VALUE self) {
    if (check_connection(self)) {
        if (ClearAllAnalog() != -1)
            return true;
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_read_counter(VALUE self, long counter) {
    if (check_connection(self)) {
        counter = NUM2INT(counter);
        if (valid_counter(counter)) {
            long data = ReadCounter(counter);
            if (data != -1)
                return data;
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_reset_counter(VALUE self, long counter) {
    if (check_connection(self)) {
        counter = NUM2INT(counter);
        if (valid_counter(counter)) {
            if (ResetCounter(counter) != -1)
                return true;
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_set_debounce(VALUE self, long counter, long time) {
    if (check_connection(self)) {
        counter = NUM2INT(counter);
        time = NUM2INT(time);
        if (valid_counter(counter)) {
            if (SetCounterDebounceTime(counter, time) != -1)
                return true;
        }
        printf("K8055 returned an error.\n");
        return false;
    }
}

static VALUE method_to_s(VALUE self) {
    if (check_connection(self)) {
        return ReadAllValues();
    }
}


static VALUE rubyk8055Init(VALUE self) {
  rb_iv_set(self, "@connected", false);
  rb_iv_set(self, "@board_address", INT2NUM(0));
}

// ----------------------- RubyK8055 class initialization ----------------------

void Init_rubyk8055() {

    VALUE USB = rb_define_module("USB");
    VALUE RubyK8055 = rb_define_class_under(USB, "RubyK8055", rb_cObject);

    rb_define_method(RubyK8055, "initialize", rubyk8055Init, 0);

    rb_define_method(RubyK8055, "connect", method_connect, -1);
    rb_define_method(RubyK8055, "disconnect", method_disconnect, 0);
    rb_define_method(RubyK8055, "address", method_address, 0);

    rb_define_method(RubyK8055, "get_analog", method_get_analog, 1);
    rb_define_method(RubyK8055, "set_analog", method_set_analog, 2);
    rb_define_method(RubyK8055, "set_analog_max", method_set_analog_max, 1);
    rb_define_method(RubyK8055, "set_analog_min", method_set_analog_min, 1);

    rb_define_method(RubyK8055, "get_digital", method_get_digital, 1);
    rb_define_method(RubyK8055, "set_digital", method_set_digital, 2);
    rb_define_method(RubyK8055, "write_all_digital", method_write_all_digital, 1);

    rb_define_method(RubyK8055, "set_all_digital", method_set_all_digital, 0);
    rb_define_method(RubyK8055, "clear_all_digital", method_clear_all_digital, 0);
    rb_define_method(RubyK8055, "set_all_analog", method_set_all_analog, 0);
    rb_define_method(RubyK8055, "clear_all_analog", method_clear_all_analog, 0);

    rb_define_method(RubyK8055, "to_s", method_to_s, 0);

    rb_define_method(RubyK8055, "read_counter", method_read_counter, 1);
    rb_define_method(RubyK8055, "reset_counter", method_reset_counter, 1);
    rb_define_method(RubyK8055, "set_debounce", method_set_debounce, 2);

}


/*

-------- Implemented: --------

int OpenDevice(long board_address);
int CloseDevice();

long ReadAnalogChannel(long Channelno);

int OutputAnalogChannel(long channel, long data);

int ReadDigitalChannel(long channel);
int SetDigitalChannel(long channel);
int ClearDigitalChannel(long channel);

int SetAllDigital();
int ClearAllDigital();
int SetAllAnalog();
int ClearAllAnalog();

long ReadCounter(long counterno);
int ResetCounter(long counternr);
int SetCounterDebounceTime(long counterno, long debouncetime);

*/

