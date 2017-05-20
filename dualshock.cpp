#include "dualshock.h"
#include "hidapi.h"

#include <array>
#include <algorithm>
#include <functional>

#define VENDOR_PRODUCT_FMT "%04hx %04hx"
#define HID_ARRAY std::array<uint8_t, HID_DATA_LEN>

static const unsigned int HID_DATA_LEN = 64;
static const unsigned int META_STR_LEN = 64;
static const unsigned int DELAY = 4;

static const unsigned short SONY_VENDOR_ID = 1356;
// dualshock 4 second edition, dualshock 4, dualshock 3
static const std::array<unsigned short, 3> SONY_PRODUCT_IDS = {{
    2508, 1476, 616}};

class Event {
    public:
        const char* name;
        const std::function<uint8_t(const HID_ARRAY)> get_value;
        Event(const char* name,
            std::function<uint8_t(const HID_ARRAY)> get_value):
            name(name), get_value(get_value) {}
};

static const std::array<const Event, 22> EVENTS = {{
    Event("LX", [](const HID_ARRAY d) { return d[1]; }),

    Event("LY", [](const HID_ARRAY d) { return d[2]; }),

    Event("RX", [](const HID_ARRAY d) { return d[3]; }),

    Event("RY", [](const HID_ARRAY d) { return d[4]; }),

    Event("DN", [](const HID_ARRAY d) {
        const auto v = d[5] & 0xf; return v == 0x0 || v == 0x1 || v == 0x7; }),
    Event("DE", [](const HID_ARRAY d) {
        const auto v = d[5] & 0xf; return v == 0x1 || v == 0x2 || v == 0x3; }),
    Event("DS", [](const HID_ARRAY d) {
        const auto v = d[5] & 0xf; return v == 0x3 || v == 0x4 || v == 0x5; }),
    Event("DW", [](const HID_ARRAY d) {
        const auto v = d[5] & 0xf; return v == 0x5 || v == 0x6 || v == 0x7; }),
    Event("square", [](const HID_ARRAY d) { return (d[5] & 0x10) != 0; }),
    Event("cross", [](const HID_ARRAY d) { return (d[5] & 0x20) != 0; }),
    Event("circle", [](const HID_ARRAY d) { return (d[5] & 0x40) != 0; }),
    Event("triangle", [](const HID_ARRAY d) { return (d[5] & 0x80) != 0; }),

    Event("L1", [](const HID_ARRAY d) { return (d[6] & 0x1) != 0; }),
    Event("R1", [](const HID_ARRAY d) { return (d[6] & 0x2) != 0; }),
    Event("share", [](const HID_ARRAY d) { return (d[6] & 0x10) != 0; }),
    Event("options", [](const HID_ARRAY d) { return (d[6] & 0x20) != 0; }),
    Event("L3", [](const HID_ARRAY d) { return (d[6] & 0x40) != 0; }),
    Event("R3", [](const HID_ARRAY d) { return (d[6] & 0x80) != 0; }),

    Event("PS", [](const HID_ARRAY d) { return (d[7] & 0x1) != 0; }),
    Event("touchpad", [](const HID_ARRAY d) { return (d[7] & 0x2) != 0; }),

    Event("L2", [](const HID_ARRAY d) { return d[8]; }),

    Event("R2", [](const HID_ARRAY d) { return d[9]; }),
}};

class DualShock {
	public:
		t_object pd_object;
        t_outlet* outlet;
        struct hid_device_info* devs;
        hid_device* dev;
        t_clock* clock;
        HID_ARRAY hid_data;
        HID_ARRAY hid_data_prev;
        bool is_polling;

        void initialize(const char* vendor, const char* product);
        void free_device();
        void set_device(const char* vendor, const char* product);
        void set_device(const unsigned short vendor,
                const unsigned short product);
		void list_devices();
        void read();
        void emit_event(const char* name, uint8_t prev,
                uint8_t current);
        void start();
        void stop();
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
                post("autodetected device " VENDOR_PRODUCT_FMT
                     "; attempting to set",
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

    clock = clock_new(this, reinterpret_cast<t_method>(dualshock_read));
    clock_setunit(clock, DELAY, 0);
    is_polling = false;

    outlet = outlet_new(&pd_object, 0);
}

void DualShock::list_devices() {
    for (auto d = devs; d != nullptr; d = d->next) {
		post("%ls %ls", d->manufacturer_string, d->product_string);
		post("\t" VENDOR_PRODUCT_FMT " (%ls)", d->vendor_id, d->product_id,
                d->serial_number);
	}
}

void DualShock::read() {
    if (dev == nullptr) {
        return;
    }

    int result = 0, block_count = -1;
    // post("read start");
    do {
        if (result == -1) {
            error("couldn't read from device");
            return;
        }
        /*
        for(int i = 0; i < result; i++) {
            startpost("%02hx ", hid_data[i]);
        }
        endpost();
        */
        for (auto e : EVENTS) {
            emit_event(e.name, e.get_value(hid_data_prev),
                    e.get_value(hid_data));
        }
        hid_data_prev = hid_data;
        result = hid_read(dev, hid_data.data(), HID_DATA_LEN);
        block_count += 1;
    } while(result != 0); 
    // post("read end, %i blocks", block_count);
    if (is_polling) {
        clock_delay(clock, 1);
    }
}

void DualShock::emit_event(const char* name, uint8_t prev,
        uint8_t current) {
    if (prev == current) {
        return;
    }
    t_atom event;
    SETFLOAT(&event, current);
    outlet_anything(outlet, gensym(name), 1, &event);
}

void DualShock::free_device() {
    if (dev != nullptr) {
        hid_close(dev);
        dev = nullptr;
    }
}

void DualShock::start() {
    if (!is_polling) {
        is_polling = true;
        clock_delay(clock, 1);
    }
}

void DualShock::stop() {
    if (is_polling) {
        is_polling = false;
        clock_unset(clock);
    }
}

void DualShock::set_device(const char* vendor, const char* product) {
    unsigned short v, p;

    sscanf(vendor, "%hx", &v);
    sscanf(product, "%hx", &p);
    set_device(v, p);
}

void DualShock::set_device(unsigned short vendor, unsigned short product) {
    wchar_t manufacturer[META_STR_LEN], product_string[META_STR_LEN];

    free_device();
    dev = hid_open(vendor, product, nullptr);

    if (dev == nullptr) {
        error("failed to open device " VENDOR_PRODUCT_FMT, vendor, product);
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
    stop();
    clock_free(clock);
    hid_free_enumeration(devs);
    free_device();
    hid_exit();
    outlet_free(outlet);
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

void dualshock_read(DualShock *h) {
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

void dualshock_start(DualShock *h) {
    h->start();
}

void dualshock_stop(DualShock *h) {
    h->stop();
}

void dualshock_setup() {
    dualshock_class = class_new(
        gensym("dualshock"), reinterpret_cast<t_newmethod>(dualshock_new),
        reinterpret_cast<t_method>(dualshock_free), sizeof(DualShock),
        CLASS_DEFAULT, A_DEFSYMBOL, A_DEFSYMBOL, static_cast<t_atomtype>(0));

    class_addbang(dualshock_class, dualshock_read);
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_read), gensym("read"),
            static_cast<t_atomtype>(0));
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_list), gensym("ls"),
            static_cast<t_atomtype>(0));
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_set_device), gensym("open"),
            A_GIMME, 0);
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_start), gensym("start"),
            static_cast<t_atomtype>(0));
    class_addmethod(dualshock_class,
            reinterpret_cast<t_method>(dualshock_stop), gensym("stop"),
            static_cast<t_atomtype>(0));
}
