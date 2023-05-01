# NesEmulator

## Summary
A simple NES emulator written in C++. It is still under development.

## Screenshots
<img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/SuperMario.png" width="256" >  <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/F1Race.png" width="256" >  <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/Chip&Dale1.png" width="256" >

<img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/Contra.png" width="256" >   <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/AdventureIsland.png" width="256" >   <img src="https://github.com/FerociousMagikarp/NesEmulator/blob/main/screenshots/DoubleDragon3.png" width="256" >

## Dependencies
* SDL2
* C++17 compiler

## Usage
To run a .nes image,
```
NesEmulator [rom_file]
```
You could get a help information by using:
```
NesEmulator -h
```

## Controls
It will support configuring soon.
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
* Add APU support
* Support more mappers
* Fix bugs(For example, PPU timing.)
* ...
