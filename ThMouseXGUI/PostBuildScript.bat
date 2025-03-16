set ServerManifestPath="%SolutionDir%ThMouseXServer\server.manifest"
set ComServerPath="%OutDir%ThMouseXGUI.exe;#1"
rem Embed the manifest file
mt -nologo -manifest %ServerManifestPath% -outputresource:%ComServerPath%