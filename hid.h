#ifndef _HID_H
#define _HID_H

#include <CoreFoundation/CoreFoundation.h>
#include <Carbon/Carbon.h>
#include <IOKit/hid/IOHIDLib.h>

#include "m_pd.h"

extern "C" {
    static t_class *hid_class;

	class HID;
    void hid_bang(HID *h);
    void* hid_new();
    void hid_setup();
}
#endif
