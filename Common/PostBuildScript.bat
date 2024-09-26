call "%DevEnvDir%..\Tools\vsdevcmd.bat" -no_logo
set PostBuildTool="%SolutionDir%PostBuildTool\bin\x86\%Configuration%\net48\PostBuildTool"
cl PreparationScript.h /P /EP /u /nologo
    if %errorlevel% neq 0 exit /b %errorlevel%
copy PreparationScript.i PreparationScript.lua /Y
    if %errorlevel% neq 0 exit /b %errorlevel%
%PostBuildTool% FormatLuaScript PreparationScript.lua