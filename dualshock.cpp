#include "dualshock.h"

#include <array>
#include <algorithm>

static const unsigned int HID_DATA_LEN = 64;
static const unsigned int META_STR_LEN = 64;

static const unsigned short SONY_VENDOR_ID = 1356;
// dualshock 4 second edition, dualshock 4, dualshock 3
static const std::array<unsigned short, 3> SONY_PRODUCT_IDS = {{
    2508, 1476, 616}};


class DualShock {
	public:
		t_object x_obj;
        struct hid_device_info* devs;
        hid_device* dev;
        unsigned char hid_data[HID_DATA_LEN];

        void initialize(const char* vendor, const char* product);
        void free_device();
        void set_device(const char* vendor, const char* product);
        void set_device(const unsigned short vendor,
                const unsigned short product);
		void list_devices();
        void read();
        void teardown();
};

void DualShock::initialize(const char* vendor, const char* product) {
    devs = hid_enumerate(0x0, 0x0);
    // try to find the first available one as a default
    if (strlen(vendor) == 0 || strlen(product) == 0) {
        for (auto d = devs; d != nullptr; d = d->next) {
            if (d->vendor_id != SONY_VENDOR_ID)
                continue;
            auto p = std::find(SONY_PRODUCT_IDS.begin(),
                    SONY_PRODUCT_IDS.end(), d->product_id);
            if (p != SONY_PRODUCT_IDS.end()) {
                post("autodetected device %04hx %04hx; attempting to set",
                        SONY_VENDOR_ID, *p);
                set_device(SONY_VENDOR_ID, *p);
                break;
            }
        }
    } else {
        post("trying device %s %s", vendor, product);
        set_device(vendor, product);
    }
    if (dev == nullptr) {
        list_devices();
    }
}

void DualShock::list_devices() {
    for (auto d = devs; d != nullptr; d = d->next) {
		post("%ls %ls", d->manufacturer_string, d->product_string);
		post("\t%04hx %04hx (%ls)", d->vendor_id, d->product_id,
                d->serial_number);
	}
}

void DualShock::read() {
    if (dev == nullptr) {
        return;
    }

    int result = 0, block_count = -1;
    post("read start");
    do {
        if (result == -1) {
            error("couldn't read from device");
            return;
        }
        for(int i = 0; i < result; i++) {
            startpost("%02hx ", hid_data[i]);
        }
        endpost();
        result = hid_read(dev, hid_data, HID_DATA_LEN);
        block_count += 1;
    } while(result != 0); 
    post("read end, %i blocks", block_count);
}

void DualShock::free_device() {
    if (dev != nullptr) {
        hid_close(dev);
        dev = nullptr;
    }
}

void DualShock::set_device(const char* vendor, const char* product) {
    unsigned short v, p;

    if (vendor == nullptr || product == nullptr) {
        return;
    }

    sscanf(vendor, "%hx", &v);
    sscanf(product, "%hx", &p);
    set_device(v, p);
}

void DualShock::set_device(unsigned short vendor, unsigned short product) {
    wchar_t manufacturer[META_STR_LEN], product_string[META_STR_LEN];

    free_device();
    dev = hid_open(vendor, product, nullptr);

    if (dev == nullptr) {
        error("failed to open device %04hx %04hx", vendor, product);
        return;
    }

    if (hid_set_nonblocking(dev, 1)) {
        error("couldn't set nonblocking mode on device");
    }

    if (hid_get_manufacturer_string(dev, manufacturer, META_STR_LEN)) {
        error("couldn't read opened device's manufacturer string");
        return;
    }
    if (hid_get_product_string(dev, product_string, META_STR_LEN)) {
        error("couldn't read opened device's product string");
        return;
    }
    post("using '%ls %ls' as a controller", manufacturer, product_string);
}

void DualShock::teardown() {
    hid_free_enumeration(devs);
    free_device();
    hid_exit();
}

void dualshock_list(DualShock *h) {
    h->list_devices();
}

void dualshock_set_device(DualShock *h, t_symbol *_ignored, int argc,
        t_atom *argv) {
    if (argc != 2 || argv[0].a_type != A_SYMBOL ||
            argv[1].a_type != A_SYMBOL) {
        error("hid takes vendor_id and product_id strings as arguments");
        error("argc: %i, type1: %i, type2: %i", argc, argv[0].a_type,
                argv[1].a_type);
        return;
    }
    h->set_device(argv[0].a_w.w_symbol->s_name, argv[1].a_w.w_symbol->s_name);
}

void dualshock_read_data(DualShock *h) {
    h->read();
}

void* dualshock_new(t_symbol *vendor, t_symbol *product) {
    auto h = reinterpret_cast<DualShock*>(pd_new(dualshock_class));
    h->initialize(vendor->s_name, product->s_name);
    return reinterpret_cast<void*>(h);
}

void dualshock_free(DualShock *h) {
    h->teardown();
}

void dualshock_setup() {
    dualshock_class = class_new(
        gensym("dualshock"), reinterpret_cast<t_newmethod>(dualshock_new),
        reinterpret_cast<t_method>(dualshock_free), sizeof(DualShock),
        CLASS_DEFAULT, A_DEFSYMBOL, A_DEFSYMBOL, static_cast<t_atomtype>(0));

    class_addbang(dualshock_class, dualshock_read_data);
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_read_data), gensym("read"),
            static_cast<t_atomtype>(0));
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_list), gensym("ls"),
            static_cast<t_atomtype>(0));
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_set_device), gensym("open"),
            A_GIMME, 0);
}
