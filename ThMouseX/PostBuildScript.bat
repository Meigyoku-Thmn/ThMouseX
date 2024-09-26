call "%DevEnvDir%..\Tools\vsdevcmd.bat" -no_logo
set ClientManifestPath="%SolutionDir%ThMouseXServer\client.manifest"
set ComClientPath="%OutDir%ThMouseX.dll;#2"
mt -nologo -manifest %ClientManifestPath% -outputresource:%ComClientPath%