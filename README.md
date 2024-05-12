# 225

It's a 15-puzzle, made out of 15-puzzles.
Play it on your 8-bit handhelds!
Currently, the Game Boy and Game Gear are supported.

## Building

To build the game, you'll need [GBDK][1] and a [BSD-compatible make][2].
You will also need my ["NES Tools"][3] for generating the graphics data.
After all of these are installed and working,
make sure they are available on your PATH
and you can proceed with the build.

First, check out the sources:

    git clone --recurse-submodules https://github.com/pineberryfox/225.git

The build system selects which variant to build
based on the chosen object directory.

```
$ mkdir obj.gb
$ bmake MAKEOBJDIR=obj.gb LCC=/path/to/gbdk/bin/lcc
$ mkdir obj.gg
$ bmake MAKEOBJDIR=obj.gg LCC=/path/to/gbdk/bin/lcc
$ mkdir obj.sms
$ bmake MAKEOBJDIR=obj.sms LCC=/path/to/gbdk/bin/lcc
```

The web-playable version builds upon the Game Boy version.
It also currently probably only builds on macOS,
as `MKZIP` is set to an invocation of Apple's `ditto`.

```
$ bmake MAKEOBJDIR=obj.gb LCC=/path/to/gbdk/bin/lcc web.zip
```

## Third-party tools

The Game Gear (and SMS) version uses a custom sound driver, but
the Game Boy version uses the sound driver from [VGM2GBSFX][4].
The web version embeds the Game Boy version in [binjgb][5].
Each of these third-party tools is used under the LICENSE specified
in their respective subdirectories.

[1]: https://gbdk-2020.github.io/gbdk-2020/
[2]: https://www.crufty.net/help/sjg/bmake.html
[3]: https://github.com/vvulpes0/nes-tools.git
[4]: https://github.com/untoxa/VGM2GBSFX.git
[5]: https://github.com/binji/binjgb.git
