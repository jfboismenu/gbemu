gbemu, a C++ gameboy emulator
=============================

This emulator is far from complete. While it does pass all blargg's instruction
tests except for one opcode, it doesn't support sound, misses support for many
memory controllers and has some timings screwed up. Still, it reliably plays
some games. There is also no vsync, so the emulator tends to run way too fast.

To launch a game, use the command line. This is a barebones Gameboy emultator,
remember?

> gbemu.exe *path-to-the-rom* *path-to-the-optional-boot-rom*

You don't need a boot rom to use the emulator, but if you have one, the emulator
will run it so you can see the scrolling Nintendo logo. :)
