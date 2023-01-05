# IndeX360

Uses your Valve Index Controllers as an xbox 360 gamepad. Forked from XJoy and ported over to use OpenVR's Input system.
In practice any VR controller can be mapped onto the individual 360 buttons using SteamVR.



## Dual mode

This version has an option for a split dual controller, where each index controller simulates a xbox 360 gamempad. This is usefull for simple local multiplayer games where not many control buttons are needed. The program registers two seperate controllers for games that don't support a single controller for two players. Some ingame configuration might be needed to configure the buttons properly.

The following buttons are available and mirrored for both controllers

| Xbox Button    |
|----------------|
| B              |
| A              |
| Right Trigger  |
| Right Shoulder |
| Left Analog    |
| Left Thumb     |
| Start          |
| Back           |
| Guide          |

To switch to dual mode, press [2] in the console window after starting the program.



## Note
Bumpers are hardcoded to the to middle fingers, since those kind of inputs aren't directly available through SteamVR's input binding interface.

## Installation

1. [Install the ViGEm Bus Driver](https://github.com/ViGEm/ViGEmBus/releases/tag/setup-v1.16.116) (install all requirements as well)
2. Download and extract the latest release.

## Usage

1. Run SteamVR and make sure both controllers are active.
2. Run IndeX360.exe.
3. (optional) Enable dual mode by pressing 2 in console.
4. (optional) Test or calibrate the controllers in Windows USB Game Controller control panel.
4. Leave the console window open in the background while running the game.


## Customization

Right now all buttons are hard-coded to their "default" xbox equivalents. If you wish to
customize these mappings, feel free to modify the `process_button` method in XJoy.cpp and
recompile yourself. I plan to add support for a configuration file and maybe a GUI in later
versions. The default mappings are shown below:


| Joy-Con Button     | Xbox Button    |
|--------------------|----------------|
| Right B            | B              |
| Right A            | A              |
| Left B             | Y              |
| Left A             | X              |
| Left Trigger       | Left Trigger   |
| Right Trigger      | Right Trigger  |
| Left Middle Finger | Left Shoulder  |
| Right Middle Finger| Right Shoulder |
| Left Touchpad      | D-PAD          |
| Left Analog        | Left Analog    |
| Right Analog       | Right Analog   |
| Left Stick         | Left Thumb     |
| Right Stick        | Right Thumb    |
| Right Touchpad D   | Start          |
| Right Touchpad U   | Back           |
| Right Touchpad R   | Guide          |


## Building

If you wish to build XJoy yourself, simply install the ViGEm Bus Driver as outlined in the
installation steps, open the XJoy.sln file in Visual Studio 2017, and build. Everything
should work out of the box but if it does not feel free to submit an issue. Note that at
least on my end it _seems_ to be working in Visual Studio 2019 as well, which is good.
Update: Needs OpenVR.dll
