rem Process the header file into a Lua script file
cl PreparationScript.h /P /EP /u /nologo
    if %errorlevel% neq 0 exit /b %errorlevel%
rem Format and commit only if there is any content-change
copy PreparationScript.i _PreparationScript.lua /Y
    if %errorlevel% neq 0 exit /b %errorlevel%
"%PostBuildTool%" FormatLuaScript _PreparationScript.lua PreparationScript.lua