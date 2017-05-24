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


## Usage

`start` starts polling, `stop` stops it; the outlet produces `<key> <value>` messages.

Try one of the sample dualshock patches included, or print the output to see what the messages are like (caution, the joysticks can get pretty busy), or consult the source for all possible messages.

NOTE: The DualShock 4 controller sample rate is 4 milliseconds, but because PD operates at sampling rate speed that is not divisible by 4 ms, the external samples every 1 millisecond, so you might get up to around 1 millsecond jitter.  This probably shouldn't affect things, but if it does it should be easy to change in the source file.


## TODO

Maybe add gyros/accelerometer support.
