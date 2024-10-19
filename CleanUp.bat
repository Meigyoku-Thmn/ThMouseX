if exist bin (
	rmdir /Q /S bin
) else if exist "%OutDir%" (
	rmdir /Q /S "%OutDir%"
)