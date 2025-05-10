# NesEmulator
<pre>
  _   _                _____                       _           _                  
 | \ | |   ___   ___  | ____|  _ __ ___    _   _  | |   __ _  | |_    ___    _ __ 
 |  \| |  / _ \ / __| |  _|   | '_ ` _ \  | | | | | |  / _` | | __|  / _ \  | '__|
 | |\  | |  __/ \__ \ | |___  | | | | | | | |_| | | | | (_| | | |_  | (_) | | |   
 |_| \_|  \___| |___/ |_____| |_| |_| |_|  \__,_| |_|  \__,_|  \__|  \___/  |_|   
</pre>

[![Linux Build](https://img.shields.io/github/actions/workflow/status/FerociousMagikarp/NesEmulator/ubuntu.yml?label=Linux&logo=linux)](https://github.com/FerociousMagikarp/NesEmulator/actions/workflows/ubuntu.yml)
[![macOS Build](https://img.shields.io/github/actions/workflow/status/FerociousMagikarp/NesEmulator/macos.yml?label=macOS&logo=apple)](https://github.com/FerociousMagikarp/NesEmulator/actions/workflows/macos.yml)
[![Windows Build](https://img.shields.io/github/actions/workflow/status/FerociousMagikarp/NesEmulator/windows.yml?label=Windows&logo=Windows")](https://github.com/FerociousMagikarp/NesEmulator/actions/workflows/windows.yml)
[![License](https://img.shields.io/badge/license-MIT-blue)](https://opensource.org/licenses/MIT)
[![Dependency](https://img.shields.io/badge/dependency-SDL2-0175c2)](https://www.libsdl.org/)
[![C++20](https://img.shields.io/badge/C++-20-blue?logo=c%2B%2B)](https://shields.io/)

## Summary
A simple NES emulator written in C++.

This project began as a hobbyist experiment, evolving into a hands-on journey through modern C++ practices.(I coundn't even write proper C++98 code before this. :clown_face: )

Reference : [WIKI](https://www.nesdev.org/wiki/). Occasionally referenced existing implementations when tackling complex components.

## Screenshots
<img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/SuperMario.png" width="256" >  <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/F1Race.png" width="256" >  <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/Chip&Dale1.png" width="256" >

<img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/Contra.png" width="256" >   <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/AdventureIsland.png" width="256" >   <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/DoubleDragon3.png" width="256" >

## Dependencies
* SDL2
* C++20 compiler

## Usage
To run a .nes image,
```
NesEmulator <rom_file>
```
You could get a help information by using:
```
NesEmulator -?
```

## Controls

You can change the default configuration in `./config.ini`.

|Player1||Player2||
|:---|:---|:---|:---|
|**Button**|**Key**|**Button**|**Key**|
|A|K|A|NumpadPeriod|
|B|J|B|Numpad0|
|Turbo A|I|Turbo A|Numpad2|
|Turbo B|U|Turbo B|Numpad1|
|Select|;|Select||
|Start|Enter|Start||
|Up|W|Up|Up|
|Down|S|Down|Down|
|Left|A|Left|Left|
|Right|D|Right|Right|

## Future
* Support more mappers
* Fix bugs(For example, PPU timing.)
* ...
