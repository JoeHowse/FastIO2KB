# FastIO2KB

This repository contains source code for a Windows console application, `fast2kb.exe`, which reads input via `iDmacDrv32.dll` from a Fast I/O Direct Memory Access Controller (DMAC) and remaps this input to DirectInput keyboard events. Any DirectInput-compatible application can receive the remapped input.

For example, on a Windows-based arcade machine with a Fast I/O control system, `fast2kb.exe` can serve input to a DirectInput-compatible emulator such as MAME or PPSSPP.

## System Requirements

* **Operating system:** Windows XP or later. Tested successfully with:
  * Windows XP Embedded SP2
* **Drivers:** `iDmacDrv32.dll`. Depending on how your environment is configured, you might need to copy `iDmacDrv32.dll` into the same directory as `fast2kb.exe`.
* **Fast I/O DMAC board**: Needs to be compatible with your version of `iDmacDrv32.dll`. Tested successfully with:
  * Taito Type X<sup>2</sup> PCIe board (as found in NESiCAxLive and Dariusburst versions of Taito Type X<sup>2</sup>)
* **Fast I/O microcontroller board**: Tested successfully with:
  * Taito K91X1204B board. Supports JAMMA to Fast I/O.

## Troubleshooting

* Copy `iDmacDrv32.dll` (a version that is compatible with your Fast I/O DMAC board) into the same directory as `fast2b.exe`.
* Plug the Fast I/O (Ethernet) cable into another port in your Fast I/O DMAC board. A typical board has two Fast I/O ports, which `fast2b.exe` interprets as one port for Players 1&2 (supported) and another port for Players 3&4 (not yet supported).

## Build Configurations and Keymaps

The project has different build configurations for different keymaps:

* **MAME:** This configuration works well with the default keyboard controls in MAME (an arcade machine emulator).
* **PPSSPP:** This configuration works well with the default keyboard controls in PPSSPP (a PSP emulator). Because the PSP has many inputs and just one player, the keymap combines Fast I/O's Player 1 and 2 inputs to support just one PSP player.

The following tables show the keymaps from Fast I/O inputs to keyboard inputs and from keyboard inputs to emulated controls in MAME or PPSSPP.

### MAME Keymap

| Fast I/O Input                       | Key Received by MAME | Default Mapping in MAME |
| ------------------------------------ | -------------------- | ----------------------- |
| Player 1 Start                       | 1                    | Player 1 Start          |
| Player 1 Up                          | Up                   | Player 1 Up             |
| Player 1 Down                        | Down                 | Player 1 Down           |
| Player 1 Left                        | Left                 | Player 1 Left           |
| Player 1 Right                       | Right                | Player 1 Right          |
| Player 1 Button 1                    | Left Ctrl            | Player 1 Button 1       |
| Player 1 Button 2                    | Left Alt             | Player 1 Button 2       |
| Player 1 Button 3                    | Space                | Player 1 Button 3       |
| Player 1 Button 4                    | Left Shift           | Player 1 Button 4       |
| Player 1 Button 5                    | Z                    | Player 1 Button 5       |
| Player 1 Button 6                    | X                    | Player 1 Button 6       |
| Player 1 Button 7                    | C                    | Player 1 Button 7       |
| Player 1 Button 8                    | V                    | Player 1 Button 8       |
| Player 2 Start                       | 2                    | Player 2 Start          |
| Player 2 Up                          | R                    | Player 2 Up             |
| Player 2 Down                        | F                    | Player 2 Down           |
| Player 2 Left                        | D                    | Player 2 Left           |
| Player 2 Right                       | G                    | Player 2 Right          |
| Player 2 Button 1                    | A                    | Player 2 Button 1       |
| Player 2 Button 2                    | S                    | Player 2 Button 2       |
| Player 2 Button 3                    | Q                    | Player 2 Button 3       |
| Player 2 Button 4                    | W                    | Player 2 Button 4       |
| Player 2 Button 5                    | K                    | *Unmapped*              |
| Player 2 Button 6                    | L                    | *Unmapped*              |
| Player 2 Button 7                    | I                    | *Unmapped*              |
| Player 2 Button 8                    | O                    | *Unmapped*              |
| Coin 1                               | 5                    | Coin 1                  |
| Coin 2                               | 6                    | Coin 2                  |
| Test                                 | F2                   | Test                    |
| Tilt                                 | T                    | Tilt                    |
| Service 1                            | 9                    | Service 1               |
| Service 2                            | 0                    | Service 2               |
| Player 1 Start + Button 1 + Button 3 | Esc                  | Exit                    |

### PPSSPP Keymap

| Fast I/O Input                       | Key Received By PPSSPP | Default Mapping in PPSSPP |
| ------------------------------------ | ---------------------- | ------------------------- |
| Player 1 Start                       | Space                  | Start                     |
| Player 1 Up                          | Up                     | D-pad Up                  |
| Player 1 Down                        | Down                   | D-pad Down                |
| Player 1 Left                        | Left                   | D-pad Left                |
| Player 1 Right                       | Right                  | D-pad Right               |
| Player 1 Button 1                    | Q                      | Left Shoulder             |
| Player 1 Button 2                    | W                      | Right Shoulder            |
| Player 1 Button 3                    | S                      | Triangle                  |
| Player 1 Button 4                    | X                      | Circle                    |
| Player 1 Button 5                    | A                      | Square                    |
| Player 1 Button 6                    | Z                      | X                         |
| Player 1 Button 7                    | D                      | *Unmapped*                |
| Player 1 Button 8                    | C                      | *Unmapped*                |
| Player 2 Start                       | V                      | Select                    |
| Player 2 Up                          | I                      | Analog Up                 |
| Player 2 Down                        | K                      | Analog Down               |
| Player 2 Left                        | J                      | Analog Left               |
| Player 2 Right                       | L                      | Analog Right              |
| Player 2 Button 1                    | Escape                 | Pause                     |
| Player 2 Button 2                    | 1                      | *Unmapped*                |
| Player 2 Button 3                    | 2                      | *Unmapped*                |
| Player 2 Button 4                    | 3                      | *Unmapped*                |
| Player 2 Button 5                    | 4                      | *Unmapped*                |
| Player 2 Button 6                    | 5                      | *Unmapped*                |
| Player 2 Button 7                    | [                      | *Unmapped*                |
| Player 2 Button 8                    | ]                      | *Unmapped*                |
| Coin 1                               | \`                     | Speed Toggle              |
| Coin 2                               | 6                      | *Unmapped*                |
| Test                                 | 7                      | *Unmapped*                |
| Tilt                                 | \                      | *Unmapped*                |
| Service 1                            | 8                      | *Unmapped*                |
| Service 2                            | 9                      | *Unmapped*                |
| Player 1 Start + Button 1 + Button 3 | 0                      | *Unmapped*                |

## To-do

* Add license
* Consider batching input events into fewer SendInput calls
* Add support for Players 3&4
* Test compatibility with other Windows versions
* Test compatibility with other Fast I/O DMAC boards, including:
  * Taito Type X<sup>3</sup> PCIe board
* Test compatibility with other Fast I/O microcontroller boards, including:
  * Taito K91X1204A board. Supports JAMMA to Fast I/O.
  * Taito K91X1243C board
* Consider adding fallback to JVS in case where Fast I/O is unavailable

## Changelog

### 2020-06-07

* Forked from [fatal-bundy/KB2FastIO](https://github.com/fatal-bundy/FastIO2KB)
* Removed unused files
* Configured project to support Windows XP or later
* Configured project to use Warning Level 4 (instead of 3) for greater strictness at compile time
* Replaced keymap by adding separate build configurations for:
  * MAME keymap
  * PPSSPP keymap
* Refactored code
* Added control handler function for cleaner termination
* Fixed bugs that caused keypress (keydown) events to be reported multiple times
* Added support for the following inputs:
  * Player 1 Buttons 7&8
  * Player 2 Start, Up, Down, Left, Right, Buttons 1–8
  * Coins 1&2, Tilt, Service 2
  * Escape sequence: Player 1 Start + Button 1 + Button 3

