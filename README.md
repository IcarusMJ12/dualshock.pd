# DualShock

A Pure Data extension that captures Sony DualShock input and translates it into
human-usable form.  Currently Mac-only, though it should be fairly easy to
tweak the Makefile, and patches are welcome.


## Install

1. Install [Pure Data](https://puredata.info/)
2. Install [clang](https://clang.llvm.org/)
3. Install [hidapi](http://www.signal11.us/oss/hidapi/)
4. Do `PD_SRC_PATH=<path_to_pd_src> PREFIX=<lib_and_include_prefix> make`
    * optionally also `make remote` if you are not using PD Extended or otherwise have [Maxlib](https://puredata.info/downloads/maxlib)
5. Add this path to your Pure Data Preferences -> Path.
6. Make a `dualshock` object in your patch and enjoy!


## TODO

Better touchpad support.  Maybe gyros/accelerometer also.
