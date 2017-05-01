#include "hid.h"


class CFReleaseGuard {
    CFTypeRef ref;

    public:
        CFReleaseGuard(CFTypeRef ref) {
            this->ref = ref;
        }

        ~CFReleaseGuard() {
            CFRelease(ref);
        }
};


class HID {
	public:
		t_object x_obj;
        IOHIDManagerRef hid_manager;

        void initialize();
		void list_devices();
        void teardown();
};

void HID::initialize() {
    hid_manager = IOHIDManagerCreate(
        kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    if (!hid_manager) {
        error("Could not initialize HID Manager.");
        return;
    }
    IOHIDManagerSetDeviceMatching(hid_manager, nullptr); 
    if (IOHIDManagerOpen(hid_manager, kIOHIDOptionsTypeNone) !=
            kIOReturnSuccess) {
        error("Could not open the HID Manager.");
        return;
    }
}

void HID::list_devices() {
    auto devices = IOHIDManagerCopyDevices(hid_manager);
    if (!devices) {
        error("Couldn't retrieve a list of devices.");
        return;
    }
    auto dev_g = CFReleaseGuard(devices);
    post("%i", CFSetGetCount(devices));
}

void HID::teardown() {
    if (hid_manager) {
        IOHIDManagerClose(hid_manager, kIOHIDOptionsTypeNone);
        CFRelease(hid_manager);
    }
}

void hid_bang(HID *h) {
    h->list_devices();
}

void* hid_new() {
    auto h = reinterpret_cast<HID*>(pd_new(hid_class));
    h->initialize();
    return reinterpret_cast<void*>(h);
}

void hid_free(HID *h) {
    h->teardown();
}

void hid_setup() {
    hid_class = class_new(
        gensym("hid"), reinterpret_cast<t_newmethod>(hid_new),
        reinterpret_cast<t_method>(hid_free), sizeof(HID), CLASS_DEFAULT,
        static_cast<t_atomtype>(0));

    class_addbang(hid_class, hid_bang);
}
