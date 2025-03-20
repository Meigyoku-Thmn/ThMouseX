set ClientManifestPath="%SolutionDir%ThMouseXServer\client.manifest"
set ComClientPath="%OutDir%%TargetName%.dll;#2"
rem Embed the manifest file
mt -nologo -manifest %ClientManifestPath% -outputresource:%ComClientPath%