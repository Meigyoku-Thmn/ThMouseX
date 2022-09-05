ThMouseX
=======

Introduction
------------
ThMouseX is a fork from ThMouse made by hwei.

This is a tool that enables mouse control for Shoot 'em ups games, intended for Touhou Project series, allowing player character to move towards wherever the cursor points.

Differences of the fork
-----------------------
* Support any game's resolutions
* Support DirectX8 and DirectX9 games

Drawbacks
--------
* Doesn't work with games that use [JIT compilation](https://en.wikipedia.org/wiki/Just-in-time_compilation), such as .NET or JVM. I would port this tool to .NET someday.
* Only works with 32-bit games (all games in Touhou Project are 32-bit).
* Require the game to display a winform dialog first for the tool to be fully functional.
* Configuration is difficult and requires knowledge about computer science. If you are familiar with Cheat Engine then maybe you can do it.

Compatibility
-------------
This tool should be compatible with any Touhou games from 6 to the latest. Here is the list of preconfigured games in games.txt:
* Touhou 6 ~ Embodiment of Scarlet Devil (ver v1.02h)
* Touhou 7 ~ Perfect Cherry Blossom (ver v1.00b)
* Touhou 8 ~ Imperishable Night (ver v1.00d)
* Touhou 9 ~ Phantasmagoria of Flower View (ver v1.50a)
* Touhou 9.5 ~ Shoot the Bullet (ver v1.02a)
* Touhou 10 ~ Mountain of Faith 
* Touhou 11 ~ Subterranean Animism 
* Touhou 12 ~ Undefined Fantastic Object (ver v1.00b)
* Touhou 12.5 ~ Shoot the Bullet DS
* Touhou 12.8 ~ Fairy Wars 
* Touhou 13 ~ Ten Desires (ver 1.00c)
* Touhou 14 ~ Double Dealing Character (ver 1.00b)
* Touhou 14.3 ~ Impossible Spell Card
* Touhou 15 ~ Legacy of Lunatic Kingdom (ver 1.00b)
* Touhou 16 ~ Hidden Star in Four Seasons
* Touhou 18 ~ Unconnected Marketeers
* Touhou 18.5 ~ 100th Black Market
You can edit the games.txt file to add more games, even PC-98 emulator to play Touhou 1 to Touhou 5 (cursor will not show up).

FAQ
---
### Download link
(TODO)

### How to compile?
This project can be compiled via Visual Studio, or just Visual Studio Build Tools:
* [Visual Studio](https://visualstudio.microsoft.com/), select "Desktop development with C++" during installation.
* [Visual Studio Build Tools](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools), select "Visual C++ build tools" during installation.
Remember to put cursor.png, ThDxHook.dll, DX8Hook.dll, ThMouse.exe, ThMouse.ini in the same folder.

### How to use ThMouse?
1. Run ThMouse.exe before entering a game.
2. Enable the option to select resolutions first when lauching the game (if the game doesn't show a dialog initially, the cursor cannot show up).
3. If the game is supported, a cursor will show upon entering the game.

### How to control?
* The character will move towards where the cursor points. Please note that it will NOT move immidiately with the cursor, because ThMouse doesn't change the moving speed.
* You still have to use left hand to focus and shoot.
* Left click can initiate bomb.
* Right click can turn off mouse control temporally.
* Press A to hide Windows' mouse, press D to unhide it.

### How to close it?
1. Exit the game first.
2. Double-click the ThMouse icon on the taskbar and press the quit button.

Additional Instructions
-----------------------
### Bomb for TH11
In order to bomb in TH11 (Subterranean Animism), you may need to modify ThMouse.ini:
1. Run TH11.exe, go to OPTIONS, then KEY CONFIG.
2. Find the number xx that corresponds to the button "SPECIAL".
3. Exit the game and open up ThMouse.ini.
4. change the number after "LeftButton = " to the number you find through step 1-2.

### Cursor sprite
The crosshair/cursor sprite may be changed:
1. Find your preferred crosshair in .png format and place it into the same folder as ThMouse.exe.
2. Open up ThMouse.ini.
3. Change the file name after "CursorTexture = " to the preferred crosshair file's name. (Don't forget the file extension name ".png".)

### "games.txt" file
You can extend ThMouse to support more Shoot 'em ups games by modifying the "games.txt" file.
Open the file with any text editor and try yourself.
(To document more)
