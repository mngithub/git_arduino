:CheckOS
IF EXIST "%PROGRAMFILES(X86)%" (GOTO 64BIT) ELSE (GOTO 32BIT)

:64BIT
echo 64-bit...
GOTO SERVICE

:32BIT
echo 32-bit...
GOTO SERVICE

:SERVICE
cd "C:\cmproxy2\service"
"C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil.exe" /u CmProxy.exe
GOTO CMD

:CMD
del "C:\cmproxy2\bin\CmProxy.exe"
rmdir "C:\cmproxy2\bin" /s /q
rmdir "C:\cmproxy2\service" /s /q
rmdir "C:\cmproxy2\client" /s /q
pause