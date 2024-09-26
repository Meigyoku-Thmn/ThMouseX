call "%DevEnvDir%..\Tools\vsdevcmd.bat" -no_logo
xcopy "%SolutionDir%%Configuration%\*" "%ProjectDir%%OutDir%" /y /s /d
    if %errorlevel% neq 0 exit /b %errorlevel%
xcopy "%SolutionDir%NeoLuaBootstrap\bin\x86\%Configuration%\net48\*" "%ProjectDir%%OutDir%" /y /s /d
    if %errorlevel% neq 0 exit /b %errorlevel%
set ServerManifestPath="%SolutionDir%ThMouseXServer\server.manifest"
set ComServerPath="%ProjectDir%%OutDir%ThMouseXGUI.exe;#1"
mt -nologo -manifest %ServerManifestPath% -outputresource:%ComServerPath%