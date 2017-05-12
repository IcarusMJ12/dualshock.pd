#include "hid.h"

#define HID_DATA_LEN 256
#define META_STR_LEN 64


class HID {
	public:
		t_object x_obj;
        struct hid_device_info* devs;
        hid_device* dev;
        unsigned char hid_data[HID_DATA_LEN];

        void initialize(char* vendor, char* product);
        void free_device();
        void set_device(char* vendor, char* product);
		void list_devices();
        void read();
        void teardown();
};

void HID::initialize(char* vendor, char* product) {
    devs = hid_enumerate(0x0, 0x0);
    set_device(vendor, product);
    if (dev == nullptr) {
        list_devices();
    }
}

void HID::list_devices() {
    for(auto d = devs; d != nullptr; d = d->next) {
		post("%ls %ls", d->manufacturer_string, d->product_string);
		post("\t%04hx %04hx (%ls)", d->vendor_id, d->product_id,
                d->serial_number);
	}
}

void HID::read() {
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

void HID::free_device() {
    if (dev != nullptr) {
        hid_close(dev);
        dev = nullptr;
    }
}

void HID::set_device(char* vendor, char* product) {
    unsigned short v, p;
    wchar_t manufacturer[META_STR_LEN], product_string[META_STR_LEN];

    if (vendor == nullptr || product == nullptr) {
        return;
    }

    free_device();
    sscanf(vendor, "%hx", &v);
    sscanf(product, "%hx", &p);
    dev = hid_open(v, p, nullptr);

    if (dev == nullptr) {
        error("failed to open device %s %s", vendor, product);
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

void HID::teardown() {
    hid_free_enumeration(devs);
    free_device();
    hid_exit();
}

void hid_bang(HID *h) {
    h->list_devices();
}

void hid_set_device(HID *h, t_symbol *_ignored, int argc, t_atom *argv) {
    if (argc != 2 || argv[0].a_type != A_SYMBOL ||
            argv[1].a_type != A_SYMBOL) {
        error("hid takes vendor_id and product_id strings as arguments");
        error("argc: %i, type1: %i, type2: %i", argc, argv[0].a_type,
                argv[1].a_type);
        return;
    }
    h->set_device(argv[0].a_w.w_symbol->s_name, argv[1].a_w.w_symbol->s_name);
}

void hid_read_data(HID *h) {
    h->read();
}

void* hid_new(t_symbol *vendor, t_symbol *product) {
    auto h = reinterpret_cast<HID*>(pd_new(hid_class));
    h->initialize(vendor->s_name, product->s_name);
    return reinterpret_cast<void*>(h);
}

void hid_free(HID *h) {
    h->teardown();
}

void hid_setup() {
    hid_class = class_new(
        gensym("hid"), reinterpret_cast<t_newmethod>(hid_new),
        reinterpret_cast<t_method>(hid_free), sizeof(HID), CLASS_DEFAULT,
        A_DEFSYMBOL, A_DEFSYMBOL, static_cast<t_atomtype>(0));

    class_addbang(hid_class, hid_bang);
    class_addmethod(hid_class, reinterpret_cast<t_method>(hid_bang),
            gensym("ls"), static_cast<t_atomtype>(0));
    class_addmethod(hid_class, reinterpret_cast<t_method>(hid_set_device),
            gensym("open"), A_GIMME, 0);
    class_addmethod(hid_class, reinterpret_cast<t_method>(hid_read_data),
            gensym("read"), static_cast<t_atomtype>(0));
}
