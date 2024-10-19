if exist PostBuildScript.bat (
	call "%DevEnvDir%..\Tools\vsdevcmd.bat" -no_logo
	PostBuildScript.bat
)