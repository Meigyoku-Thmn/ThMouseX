if %PlatformTarget% equ x86 (
    CorFlags "%OutDir%%TargetName%.dll" -nologo -32BITREQ-
)