call "%DevEnvDir%..\Tools\vsdevcmd.bat" -no_logo
set ClientManifestPath="%SolutionDir%ThMouseXServer\client.manifest"
set ComClientPath="%OutDir%ThMouseX.dll;#2"
rem Embed the manifest file
mt -nologo -manifest %ClientManifestPath% -outputresource:%ComClientPath%