set ServerManifestPath="%SolutionDir%ThMouseXServer\server.manifest"
set ComServerPath="%OutDir%ThMouseXGUI.exe;#1"
set TargetPath="%OutDir%ThMouseXGUI.exe"
set TlbPath="%SolutionDir%ThMouseXServer\server.tlb"
rem Embed the Typelib file into the Server dll file
"%PostBuildTool%" ImportResource %TargetPath% %TlbPath% typelib #1
if %errorlevel% neq 0 exit /b %errorlevel%
rem Embed the manifest file
mt -nologo -manifest %ServerManifestPath% -outputresource:%ComServerPath%