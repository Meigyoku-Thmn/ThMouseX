rem Copy all the C++ output files into this
xcopy "%SolutionDir%%Configuration%\*" "%ProjectDir%%OutDir%" /y /s /d
    if %errorlevel% neq 0 exit /b %errorlevel%
rem Copy all the files of NeoLuaBootstrap into this
xcopy "%SolutionDir%NeoLuaBootstrap\bin\x86\%Configuration%\net48\*" "%ProjectDir%%OutDir%" /y /s /d
    if %errorlevel% neq 0 exit /b %errorlevel%
set ServerManifestPath="%SolutionDir%ThMouseXServer\server.manifest"
set ComServerPath="%ProjectDir%%OutDir%ThMouseXGUI.exe;#1"
rem Embed the manifest file
mt -nologo -manifest %ServerManifestPath% -outputresource:%ComServerPath%