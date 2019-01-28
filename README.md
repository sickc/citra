Citra Framedump
==============
This is a fork of Citra emulator with experimental framedumping support, based on JSelby's yuzu "ffmpeg hack".

Requirements for framedumping: OGL as renderer and DSP HLE. SWR and DSP LLE support is comming later(or never). You must also use the QT frontend, SDL2/CLI won't work. FFMPEG executable must be in the directory with citra-qt.exe and named "ffmpeg.exe"(if you are using a shared build of ffmpeg, then you need to copy all of it's dll here too). To start check "Enable Framedumping" in Tools->Movie.
The encoded video resolution is hardcoded to 1800x1080 in renderer_opengl.cpp, but the resolution of the rendered image depends on the settings.

Visit Citra emulator's [website](https://citra-emu.org/)
Visit Citra's [central GitHub repo](https://github.com/citra-emu/citra)

### How do I build this?

* __Windows__: [Windows Build](https://github.com/citra-emu/citra/wiki/Building-For-Windows)
* __Linux__: [Linux Build](https://github.com/citra-emu/citra/wiki/Building-For-Linux)
* __macOS__: [macOS Build](https://github.com/citra-emu/citra/wiki/Building-for-macOS)

I'll also provide builds of my own in the Buils section(some may be based on this merged with Citra-Canary to get more features and performance)

### TODO/Plans List

* Implement lower screen recording by writing it to another video stream(and maybe stereoscopic recording)
* Find a way to encode video without the use of extrnal executables and get rid of platfor specific code(aka completely redo the whole framedump.cpp)
* Make a nice GUI for altering encoding settings(and unhardcode the encoding resolution) with options for different layout and such
* Code formatting and compliance with Citra's contribution guidelines
* PR it to the main Citra repo(and hopefully get it merged)
