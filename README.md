ThMouseX
=======

Introduction
------------
ThMouseX is a fork from [ThMouse](https://github.com/hwei/ThMouse) made by hwei.

This is a tool that enables mouse control for Shoot 'em ups games, intended for Touhou Project series, allowing player character to move towards wherever the cursor points.

Demo clip: https://www.youtube.com/watch?v=uMkzmM13qpU

Download link
---
https://github.com/Meigyoku-Thmn/ThMouseX/releases <br>
(Require .NET Framework 4.8 from version 2.1.0)

Differences of the fork
-----------------------
* Support any game's resolutions
* Support DirectX8, DirectX9 and DirectX11 games
* Support .NET Framework games via [NeoLua](https://github.com/neolithos/neolua) and [Lib.Harmony](https://github.com/pardeike/Harmony)
* Can be opened and closed at any time, it will automatically detect configured games
* You can use Lua script ([Lua](https://www.lua.org/), [LuaJIT](https://luajit.org/), or [NeoLua](https://github.com/neolithos/neolua)) to configure further.

Drawbacks
--------
* Only works with 32-bit games, if there is a good 64-bit bullet hell game I will consider adding 64-bit support.
* Doesn't work well with Steam Overlay, although this is minimal.
* Doesn't work well with other mods that inject their own overlay (for example, [thprac](https://github.com/touhouworldcup/thprac)), they can still work but the overlays will override each other.
* Configuration is difficult.

Preconfigured games
-------------
This tool should be compatible with any Touhou games from 6 to the latest. Here is the list of preconfigured games in [Games.txt](https://github.com/Meigyoku-Thmn/ThMouseX/blob/master/ThMouseX/Games.txt):
* Touhou 6&emsp;&emsp;&ensp;東方紅魔郷 ～ Embodiment of Scarlet Devil         (v1.02h)
* Touhou 7&emsp;&emsp;&ensp;東方妖々夢 ～ Perfect Cherry Blossom              (v1.00b)
* Touhou 8&emsp;&emsp;&ensp;東方永夜抄 ～ Imperishable Night                  (v1.00d)
* Touhou 9&emsp;&emsp;&ensp;東方花映塚 ～ Phantasmagoria of Flower View       (v1.50a)
* Touhou 9.5&emsp;&ensp; 東方文花帖 ～ Shoot the Bullet                       (v1.02a)
* Touhou 10&emsp;&emsp;東方風神録 ～ Mountain of Faith                        (v1.00a)
* Touhou 11&emsp;&emsp;東方地霊殿 ～ Subterranean Animism                     (v1.00a)
* Touhou 12&emsp;&emsp;東方星蓮船 ～ Undefined Fantastic Object               (v1.00b)
* Touhou 12.5&emsp; Double Spoiler ～ 東方文花帖                              (v1.00a)
* Touhou 12.8&emsp; 妖精大戦争 ～ 東方三月精 Fairy Wars                        (v1.00a)
* Touhou 13&emsp;&emsp;東方神霊廟 ～ Ten Desires                              (v1.00c)
* Touhou 14&emsp;&emsp;東方輝針城 ～ Double Dealing Character                 (v1.00b)
* Touhou 14.3&emsp; 弾幕アマノジャク ～ Impossible Spell Card                  (v1.00a)
* Touhou 15&emsp;&emsp;東方紺珠伝 ～ Legacy of Lunatic Kingdom                (v1.00b)
* Touhou 16&emsp;&emsp;東方天空璋 ～ Hidden Star in Four Seasons              (v1.00a)
* Touhou 16.5&emsp; 秘封ナイトメアダイアリー 〜 Violet Detector                (v1.00a)
* Touhou 17&emsp;&emsp;東方鬼形獣 ～ Wily Beast and Weakest Creature          (v1.00b)
* Touhou 18&emsp;&emsp;東方虹龍洞 ～ Unconnected Marketeerss                  (v1.00a)
* Touhou 18.5&emsp; バレットフィリア達の闇市場 〜 100th Black Market           (v1.00a)
* Touhou 19&emsp;&emsp;東方獣王園 〜 Unfinished Dream of All Living Ghost     (v1.10c)

It also have preconfiguration of some other games (try the development branch if it doesn't work, see FAQ below): 
* [DANMAKAI: Red Forbidden Fruit](https://store.steampowered.com/app/1388230/DANMAKAI_Red_Forbidden_Fruit/)
* [東方幕華祭 〜 Fantastic Danmaku Festival](https://store.steampowered.com/app/882710/_TouHou_Makuka_Sai__Fantastic_Danmaku_Festival/)
* [東方幕華祭 春雪篇 〜 Fantastic Danmaku Festival Part II](https://store.steampowered.com/app/1031480/TouHou_Makuka_Sai__Fantastic_Danmaku_Festival_Part_II/)
* [東方眠世界 〜 Wonderful Waking World](https://store.steampowered.com/app/1901490/__Wonderful_Waking_World/) ([itch.io link](https://oligarchomp.itch.io/wonderful-waking-world))
* [東方龍隱談 〜 Chaos of Black Loong](https://store.steampowered.com/app/915130/__Touhou_Chaos_of_Black_Loong/)

* [Len'en 1&emsp;連縁无現里　～ Evanescent Existence (1.20a)](https://www.freem.ne.jp/win/game/15994)

* [Len'en 2&emsp;連縁蛇叢釼　～ Earthen Miraculous Sword (1.20a)](https://www.freem.ne.jp/win/game/15995)

* [Len'en 3&emsp;連縁霊烈傳　～ Reactivate Majestical Imperial (1.21a)](https://www.freem.ne.jp/win/game/15996)

* [Len'en 4&emsp;連縁天影戦記　～ Brilliant Pagoda or Haze Castle (1.20f)](https://www.freem.ne.jp/win/game/13429)

You can add more games to [Games2.txt](https://github.com/Meigyoku-Thmn/ThMouseX/blob/master/ThMouseX/Games2.txt) and copy it to ThMouseX's directory, side-by-side with Games.txt.

FAQ
---
### How to compile on your computer (not recommended for non-tech savvy)
This project can be compiled via Visual Studio (I use Visual Studio 2022), or just Visual Studio Build Tools:
* [Visual Studio](https://visualstudio.microsoft.com/), select "Desktop development with C++" during installation.
* [Visual Studio Build Tools](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools), select "Visual C++ build tools" during installation.

Also you need [.NET Framework 4.8 SDK](https://dotnet.microsoft.com/en-us/download/dotnet-framework).

You also need to setup [vcpkg](https://vcpkg.io/en/getting-started.html) and put vcpkg folder path into the PATH of environment variables.

Remember to **compile for 32-bit, toolset v143, .NET Framework 4.8**, and make sure these files and folder are in the same folder:
* ConfigScripts
* 0Harmony.dll
* Common.dll
* DX8Hook.dll
* Neo.Lua.dll
* NeoLuaBootstrap.dll
* Sigil.dll
* ThMCore.dll
* THMouseX.exe
* Cursor.png
* ThMouseX.ini
* VirtualKeyCodes.txt
* Games.txt

The above files should be populated in `ThMouseXGUI\bin` directory.

If you build the development branch, make sure these files and folder are in the same folder:
* ConfigScripts
* 0Harmony.dll
* Neo.Lua.dll
* NeoLuaBootstrap.dll
* THMouseX.dll
* THMouseXGUI.exe
* Cursor.png
* ThMouseX.ini
* VirtualKeyCodes.txt
* Games.txt

The above files should be populated in `Release\x86` directory.

### Another way to compile without installing anything on your computer (recommended way)
- Fork the THMouseX repository to your Github account<br>
![image](https://github.com/Meigyoku-Thmn/ThMouseX/assets/16047808/4b941f22-594d-46c6-be83-818db918e2d4)
- Go to the "Actions" tab<br>
![image](https://github.com/Meigyoku-Thmn/ThMouseX/assets/16047808/34ed8954-c711-4df4-843e-f730d8483394)
- Click "Deployment" on the left sidebar<br>
![chrome_1lYLXTdT9o](https://github.com/Meigyoku-Thmn/ThMouseX/assets/16047808/81c9cd3b-ef98-4092-a172-b48f379dd2c8)
- Run the workflow<br>
![chrome_AxRRzs1MUn](https://github.com/Meigyoku-Thmn/ThMouseX/assets/16047808/cdf4206b-4fa5-4e70-a0ac-a7e325d30ef9)
- Wait for it, then download the produced zipped artifact (shown up in the page with url form `https://github.com/<username>/ThMouseX/actions/runs/<flowid>`)<br>
![chrome_jdw1pXLJl2](https://github.com/Meigyoku-Thmn/ThMouseX/assets/16047808/0bcf952d-4acf-4db7-ba83-8e7d106b0301)

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
You can extend ThMouseX to support more Shoot 'em ups games by modifying the "[Games.txt](https://github.com/Meigyoku-Thmn/ThMouseX/blob/master/ThMouseX/Games.txt)" file. You can also use [Lua script](https://github.com/Meigyoku-Thmn/ThMouseX/tree/master/ThMouseX/ConfigScripts) for advanced cases.

### Sometime ThMouseX doesn't actually work, the game's character just keeps moving to a corner or is not bound exactly to the cursor
Try updating your game to the latest version. ThMouseX currently support only a single version of a game.

If it doesn't solve, then there are really rare bugs that I have yet found a way to fix. <br>
Some workarounds:
* Restart ThMouseX
* Start ThMouseX FIRST, then start the game
* When starting ThMouseX, make sure your game's window is NOT minimized (this should be fixed from version 2.1.0).

### "Delay" movement, lag mouse cursor
This is a normal problem in every games that the in-game cursor is a [little delay](https://github.com/ocornut/imgui/blob/master/docs/EXAMPLES.md#:~:text=About%20mouse%20cursor%20latency) compared to the OS cursor. ThMouseX also doesn't change the game's behavior (it doesn't cheat), so the character still has to follow the game rule, no "teleportation".

### Character cannot stay in one place where the cursor is, even when holding down the SHIFT button, the character just sways crazily around the cursor
The moving step of the character is a fixed value and usually longer than the mouse movement step you are able to make (for example, you can move the cursor 1 px, but the game character can't, if it moves, it has to move 5 px a frame). Holding SHIFT can reduce the moving step but only so much. I think this is an unsolvable issue. <br>
But there is a case that the game character feel really lagged and sways a lot around the cursor however you try, I saw that in Touhou 7. Installing [Vsync Patch](https://en.touhouwiki.net/wiki/Game_Tools_and_Modifications#Vsync_Patches) does fix that.

### Steam Overlay stops functioning, the game control is unusable too!
This is a limitation of how this tool hooks into the game's routine. Please avoid turning off ThMouseX while Steam Overlay is visible.

### Game crashes when exiting or lauching ThMouseX
Please avoid lauching and exiting ThMouseX many times while the game is running.<br>
So far I only see this bug on .NET Framework games and it's a rare bug. It may has something with the library Lib.Harmony which ThMouseX uses.

### Antivirus programs' detection
This tool uses various code injection techniques, so it's normal that some antivirus programs don't like it. You can verify the source code and build it on your own using the instructions above, or set an exception case in the antivirus program for ThMouseX (including the DLLs).

### ThMouseX doesn't work with old versions of Fantastic Danmaku Festival I (or games that use .NET Framework 2/3)
Old (pre-Steam) versions of Fantastic Danmaku Festival use .NET Framework 2, so normally this tool will not work because it uses .NET Framework 4. But you can force the game to use .NET Framework 4 by putting this text file to the same place with THMHJ.exe:

```xml
<?xml version="1.0" encoding="utf-8" ?>
<configuration>
    <startup useLegacyV2RuntimeActivationPolicy="true"> 
        <supportedRuntime version="v4.0"/>
    </startup>
</configuration>
```

Ensure the name of this manifest file is `THMHJ.exe.config`, not `THMHJ.txt` or `THMHJ.exe.config.txt`.

This can also be applied to other applications that use .NET Framework 2/3.
