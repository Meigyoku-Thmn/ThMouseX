ThMouseX
=======

Introduction
------------
ThMouseX is a fork from [ThMouse](https://github.com/hwei/ThMouse) made by hwei.

This is a tool that enables mouse control for Shoot 'em ups games, intended for Touhou Project series, allowing player character to move towards wherever the cursor points.

Demo clip: https://www.youtube.com/watch?v=uMkzmM13qpU

Download link
---
https://github.com/Meigyoku-Thmn/ThMouseX/releases

Differences of the fork
-----------------------
* Support any game's resolutions
* Support DirectX8 and DirectX9 games
* Can be opened and closed at any time, it will automatically detect configurated games.

Drawbacks
--------
* Only works with 32-bit games (all games in Touhou Project are 32-bit).
* Configuration is difficult.

Preconfigured games
-------------
This tool should be compatible with any Touhou games from 6 to the latest. Here is the list of preconfigured games in [games.txt](https://github.com/Meigyoku-Thmn/ThMouseX/blob/master/ThMouseX/Games.txt):
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
* Touhou 16.5 ~ Violet Detector
* Touhou 17 ~ Wily Beast and Weakest Creature
* Touhou 18 ~ Unconnected Marketeers
* Touhou 18.5 ~ 100th Black Market
* Touhou 19 ~ Unfinished Dream of All Living Ghost (ver demo) (not included in release yet)

It also have preconfiguration of some other games:
* DANMAKAI: Red Forbidden Fruit
* Fantastic Danmaku Festival I & II

You can edit the games.txt file to add more games.

FAQ
---
### How to compile on your computer (not recommended for non-tech savvy)
This project can be compiled via Visual Studio (I use Visual Studio 2022), or just Visual Studio Build Tools:
* [Visual Studio](https://visualstudio.microsoft.com/), select "Desktop development with C++" during installation.
* [Visual Studio Build Tools](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools), select "Visual C++ build tools" during installation.

Also you need .NET Framework 4.8 SDK, it should be selectable in one of the aboves.

You also need to setup [vcpkg](https://vcpkg.io/en/getting-started.html) and put vcpkg folder path into the PATH of environment variables.

Remember to compile for 32-bit, toolset v143, .NET Framework 4.8, and make sure these files and folder are in the same folder:
* ConfigScripts
* 0Harmony.dll
* Common.dll
* Cursor.png
* DX8Hook.dll
* Games.txt
* Neo.Lua.dll
* NeoLuaBootstrap.dll
* Sigil.dll
* ThMCore.dll
* THMouseX.exe
* ThMouseX.txt
* VirtualKeyCodes.txt

Due to the limitation of Visual Studio's Compiler, you might have to compile to the second time for it to work.

### Another way to compile without installing anything on your computer (recommended way)
- Fork the THMouseX repository
- Go to the "Actions" tab
- Select the "Deployment" workflow
- Run the workflow (refresh the page if nothing shows up)
- Wait for it, then download the produced zipped artifact (shown up in the page with url form `https://github.com/<username>/ThMouseX/actions/runs/<flowid>`).

### How to use ThMouseX?
1. Run ThMouseX.exe.
2. Run your game, or you can run your game first and then run ThMouseX.exe.
3. If the game is supported, a cursor will show upon entering the game.

### How to control?
* The character will move towards where the cursor points. Please note that it will NOT move immidiately with the cursor, because ThMouseX doesn't modifier any game's behavior.
* You still have to use left hand to focus and shoot.
* Left click to use bomb/spell.
* Right click to toggle mouse control.
* Press M to toggle Windows mouse cursor visibility.

### How to close it?
1. You can close your game first or ThMouseX first, it doesn't matter.
2. Double-click the ThMouseX icon on the taskbar and press the Quit button, or right click it and press Exit.

Additional Instructions
-----------------------
### Cursor sprite
The crosshair/cursor sprite may be changed:
1. Find your preferred crosshair in .png format and place it into the same folder as ThMouseX.exe.
2. Open up ThMouseX.txt.
3. Change the file name after "CursorTexture = " to the preferred crosshair file's name. (Don't forget the file extension name ".png".)

### Add your favorite games
You can extend ThMouseX to support more Shoot 'em ups games by modifying the "games.txt" file. You can also use Lua script for advanced cases.

Open the file with any text editor and try yourself.

(To document more)

### Antivirus programs' detection
This tool uses various code injection techniques, so it's normal that some antivirus programs don't like it. You can verify the source code and build it on your own using the instructions above.

### ThMouseX doesn't work with old versions of Fantastic Danmaku Festival I (or games that use .NET Framework 2/3)
Old (pre-Steam) versions of Fantastic Danmaku Festival use .NET Framework 2, so normally this tool will not work because it uses .NET Framework 4. But you can force the game to use .NET Framework 4 by putting this file to the same place with THMHJ.exe:

https://gist.github.com/Meigyoku-Thmn/d94b41b4381867c6622e15a617cea8f3

(Ensure the name of this manifest file is `THMHJ.exe.config`)
