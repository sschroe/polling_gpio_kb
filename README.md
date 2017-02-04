# About

This is a simple linux kernel module that allows triggering keyboard events from
gpio inputs. An use case for example might be a small keypad on a Raspberry Pi
with the inputs directly being mapped to the numpad keys or using gpio inputs as
game controls for emulators.


# Requirements

You need to have the linux kernel headers and development tools installed on your
system.

All gpio inputs used with this module must to be low active.


# Building

Before compiling open `polling_gpio_kb.c` and find the gpio map near the top of the file.
Adjust the structure entries for your keys and remove any entries that you will not use.
The second value is the gpio pin number to use. The last two values should be left at `0`.
An example for the Raspberry Pi might look like this:

```
gpio_map[] = { { "A", 5, KEY_A, 0, 0 },
               { "B", 6, KEY_B, 0, 0 },
               { "C", 13, KEY_C, 0, 0 },
               { "D", 19, KEY_D, 0, 0 }
             };
```

You may also modify `POLLING_FREQ_HZ` to change the polling rate. A higher value
increases cpu usage but reduces input latency.

After that simply run `make` to build the module. Kernel headers and the usual
development tools need to be installed on your system for this step.


# Usage

After building run `insmod polling_gpio_kb.ko` to load the module. Assuming
correct setup your gpios should now trigger the expected keys.
