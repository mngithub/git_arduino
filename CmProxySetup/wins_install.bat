:CheckOS
IF EXIST "%PROGRAMFILES(X86)%" (GOTO 64BIT) ELSE (GOTO 32BIT)

:64BIT
echo 64-bit...
GOTO CMD

:32BIT
echo 32-bit...
GOTO CMD

:CMD
md "C:\cmproxy2"
md "C:\cmproxy2\bin"
md "C:\cmproxy2\client"
md "C:\cmproxy2\service"
md "C:\cmproxy2\log"
xcopy "%~dp0bin" "C:\cmproxy2\bin" /s /e
xcopy "%~dp0client" "C:\cmproxy2\client" /s /e
xcopy "%~dp0service" "C:\cmproxy2\service" /s /e
copy "%~dp0wins_uninstall.bat" "C:\cmproxy2\wins_uninstall.bat"
GOTO SERVICE

:SERVICE
cd "C:\cmproxy2\service"
"C:\Windows\Microsoft.NET\Framework\v4.0.30319\InstallUtil.exe" CmProxy.exe
pause