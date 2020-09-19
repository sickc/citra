# Merge log

Scroll down for the original README.md!

Base revision: a576eb633fbe55619f8728dab15df143b451143a

|Pull Request|Commit|Title|Author|Merged?|
|----|----|----|----|----|
|[6](https://github.com/citra-emu/citra-canary/pull/6)|[d9c3e53d4](https://github.com/citra-emu/citra-canary/pull/6/files/)|Canary Base (MinGW Test)|[liushuyu](https://github.com/liushuyu)|Yes|
|[5533](https://github.com/citra-emu/citra/pull/5533)|[50db4a669](https://github.com/citra-emu/citra/pull/5533/files/)|UDS: implement GetApplicationData|[mnml](https://github.com/mnml)|Yes|
|[5509](https://github.com/citra-emu/citra/pull/5509)|[6b92a3373](https://github.com/citra-emu/citra/pull/5509/files/)|Look at direction of analog axis travel instead of instantaneous sample|[xperia64](https://github.com/xperia64)|Yes|
|[5448](https://github.com/citra-emu/citra/pull/5448)|[c0f5fde82](https://github.com/citra-emu/citra/pull/5448/files/)|Implement basic rerecording features|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5446](https://github.com/citra-emu/citra/pull/5446)|[81bcd29c0](https://github.com/citra-emu/citra/pull/5446/files/)|Bump kernel version for 3DSX|[wwylele](https://github.com/wwylele)|Yes|
|[5411](https://github.com/citra-emu/citra/pull/5411)|[e8c9328b9](https://github.com/citra-emu/citra/pull/5411/files/)|dumping/ffmpeg_backend: Various fixes|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5382](https://github.com/citra-emu/citra/pull/5382)|[b34ceb89c](https://github.com/citra-emu/citra/pull/5382/files/)|service/nwm_uds: Various improvements/corrections|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5331](https://github.com/citra-emu/citra/pull/5331)|[7c6898fdd](https://github.com/citra-emu/citra/pull/5331/files/)|NWM_UDS: implement disconnect_reason and EjectClient|[B3n30](https://github.com/B3n30)|Yes|
|[5328](https://github.com/citra-emu/citra/pull/5328)|[0af8f1f9c](https://github.com/citra-emu/citra/pull/5328/files/)|APT: implement Set and GetWirelessRebootInfo|[B3n30](https://github.com/B3n30)|Yes|
|[5325](https://github.com/citra-emu/citra/pull/5325)|[0363c5f84](https://github.com/citra-emu/citra/pull/5325/files/)|kernel/thread: Change owner_process to std::weak_ptr|[zhaowenlan1779](https://github.com/zhaowenlan1779)|Yes|
|[5278](https://github.com/citra-emu/citra/pull/5278)|[2c0cf5106](https://github.com/citra-emu/citra/pull/5278/files/)|Port yuzu-emu/yuzu#3791: "configuration: Add Restore Default and Clear options to hotkeys"|[FearlessTobi](https://github.com/FearlessTobi)|Yes|
|[5255](https://github.com/citra-emu/citra/pull/5255)|[8b6f7eb4f](https://github.com/citra-emu/citra/pull/5255/files/)|Keep keyboard state independent of acquired buttons|[hamish-milne](https://github.com/hamish-milne)|Yes|


End of merge log. You can find the original README.md below the break.

------

**BEFORE FILING AN ISSUE, READ THE RELEVANT SECTION IN THE [CONTRIBUTING](https://github.com/citra-emu/citra/wiki/Contributing#reporting-issues) FILE!!!**

Citra
==============
[![Travis CI Build Status](https://travis-ci.com/citra-emu/citra.svg?branch=master)](https://travis-ci.com/citra-emu/citra)
[![AppVeyor CI Build Status](https://ci.appveyor.com/api/projects/status/sdf1o4kh3g1e68m9?svg=true)](https://ci.appveyor.com/project/bunnei/citra)
[![Bitrise CI Build Status](https://app.bitrise.io/app/4ccd8e5720f0d13b/status.svg?token=H32TmbCwxb3OQ-M66KbAyw&branch=master)](https://app.bitrise.io/app/4ccd8e5720f0d13b)
[![Discord](https://img.shields.io/discord/220740965957107713?color=%237289DA&label=Citra&logo=discord&logoColor=white)](https://discord.gg/FAXfZV9)

Citra is an experimental open-source Nintendo 3DS emulator/debugger written in C++. It is written with portability in mind, with builds actively maintained for Windows, Linux and macOS.

Citra emulates a subset of 3DS hardware and therefore is useful for running/debugging homebrew applications, and it is also able to run many commercial games! Some of these do not run at a playable state, but we are working every day to advance the project forward. (Playable here means compatibility of at least "Okay" on our [game compatibility list](https://citra-emu.org/game).)

Citra is licensed under the GPLv2 (or any later version). Refer to the license.txt file included. Please read the [FAQ](https://citra-emu.org/wiki/faq/) before getting started with the project.

Check out our [website](https://citra-emu.org/)!

Need help? Check out our [asking for help](https://citra-emu.org/help/reference/asking/) guide.

For development discussion, please join us on our [Discord server](https://citra-emu.org/discord/) or at #citra-dev on freenode.

### Development

Most of the development happens on GitHub. It's also where [our central repository](https://github.com/citra-emu/citra) is hosted.

If you want to contribute please take a look at the [Contributor's Guide](https://github.com/citra-emu/citra/wiki/Contributing) and [Developer Information](https://github.com/citra-emu/citra/wiki/Developer-Information). You should also contact any of the developers in the forum in order to know about the current state of the emulator because the [TODO list](https://docs.google.com/document/d/1SWIop0uBI9IW8VGg97TAtoT_CHNoP42FzYmvG1F4QDA) isn't maintained anymore.

If you want to contribute to the user interface translation, please check out the [citra project on transifex](https://www.transifex.com/citra/citra). We centralize the translation work there, and periodically upstream translations.

### Building

* __Windows__: [Windows Build](https://github.com/citra-emu/citra/wiki/Building-For-Windows)
* __Linux__: [Linux Build](https://github.com/citra-emu/citra/wiki/Building-For-Linux)
* __macOS__: [macOS Build](https://github.com/citra-emu/citra/wiki/Building-for-macOS)


### Support
We happily accept monetary donations or donated games and hardware. Please see our [donations page](https://citra-emu.org/donate/) for more information on how you can contribute to Citra. Any donations received will go towards things like:
* 3DS consoles for developers to explore the hardware
* 3DS games for testing
* Any equipment required for homebrew
* Infrastructure setup

We also more than gladly accept used 3DS consoles! If you would like to give yours away, don't hesitate to join our [Discord server](https://citra-emu.org/discord/) and talk to bunnei.
