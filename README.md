# MEGADRIVER

Eurorack sound source that uses the mythical YM2612 sound chip as his heart.

The objective of this project is to create an HW platform that can generate the sound of the '80s and have a polyphonic module that sounds great at live-play.

# FEATURES

- Six full-featured voices that can operate in two configurable modes:
    - polyphony mode, up to six voices.
    - mono mode, with six independent voices.

- Two selectable memory banks:
    - Bank_0, preset bank with four fixed instruments.
    - Bank_1, user bank with room to store up to eight different presets.

- Full control of all FM parameters present at the YM2612 chip:
    - Four operators by voice, eight different algorithms.
    - Each operator comes with twelve parameters to tweak.

- Four analog channels that can be configured for:
    - V/Oct tracking, five octaves tracking (0-5V).
    - Gate input.
    - Parameter mapping (-5V, 5V).

- Megadrive instrument cloning!!!
    - With this FW comes a Python tool that allows you to interface the module using Midi SysEx messages. With this tool, you can use vgi files to load directly the register values to clone instruments used in Sega Megadrive games.

# BUILDING

To compile the project you should provide the path to your GNU toolchain path to the makefile.

You can do this in two ways:

1. Passing this as an argument in command line:

```
make all GCC_PATH=<path_toolchain_bin>
```

2. Modify GCC_PATH variable with your path.

Is up to you.

# HARDWARE

Coming soon...

# LICENSE

This project is licensed under the MIT License.

> Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

> The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

# ACKNOWLEDGMENTS

I would like to mention to [Aidan Lawrence](https://www.aidanlawrence.com), who's web page help me to kick off the project.

# CONTACT

Any feedback is welcome!

Sebastián Del Moral - sebmorgal@gmail.com