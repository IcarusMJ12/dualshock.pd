#ifndef _HID_H
#define _HID_H

#include "hidapi.h"

#include "m_pd.h"

extern "C" {
    static t_class *hid_class;

	class HID;
    void hid_bang(HID *h);
    void* hid_new(t_symbol *vendor, t_symbol *product);
    void hid_set_device(HID *h, t_symbol *_ignored, int argc, t_atom *argv);
    void hid_read_data(HID *h);
    void hid_setup();
}
#endif
