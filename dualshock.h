#ifndef _DUALSHOCK_H
#define _DUALSHOCK_H

#include "hidapi.h"

#include "m_pd.h"

extern "C" {
    static t_class *dualshock_class;

	class DualShock;
    void dualshock_bang(DualShock *h);
    void* dualshock_new(t_symbol *vendor, t_symbol *product);
    void dualshock_set_device(DualShock *h, t_symbol *_ignored, int argc, t_atom *argv);
    void dualshock_read_data(DualShock *h);
    void dualshock_setup();
}
#endif //_DUALSHOCK_H
