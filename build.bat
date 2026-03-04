@echo off
setlocal

set "VCINSTALLDIR=C:\Program Files\Microsoft Visual Studio\18\Community\VC\"
set "VCToolsInstallDir=C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717\"
set "VCToolsVersion=14.50.35717"
set "WindowsSdkDir=C:\Program Files (x86)\Windows Kits\10\"
set "WindowsSdkVersion=10.0.26100.0"

set "PATH=%VCToolsInstallDir%bin\Hostx64\x64;%WindowsSdkDir%bin\%WindowsSdkVersion%\x64;%PATH%"
set "INCLUDE=%VCINSTALLDIR%Include;%WindowsSdkDir%Include\%WindowsSdkVersion%\ucrt;%WindowsSdkDir%Include\%WindowsSdkVersion%\shared;%WindowsSdkDir%Include\%WindowsSdkVersion%\um;%WindowsSdkDir%Include\%WindowsSdkVersion%\winrt"
set "LIB=%VCINSTALLDIR%lib\atlmfc\x64;%VCINSTALLDIR%lib\x64;%WindowsSdkDir%lib\%WindowsSdkVersion%\ucrt\x64;%WindowsSdkDir%lib\%WindowsSdkVersion%\um\x64"

echo Building NNetScan...

cl.exe /EHsc /W4 /O2 /MT /D_WIN32_WINNT=0x0601 /DWINVER=0x0601 /I"%WindowsSdkDir%Include\%WindowsSdkVersion%\ucrt" /I"%WindowsSdkDir%Include\%WindowsSdkVersion%\shared" /I"%WindowsSdkDir%Include\%WindowsSdkVersion%\um" /I"%WindowsSdkDir%Include%\10.0.26100.0\winrt" ^
    /Fe:nnetscan.exe ^
    src/main.cpp src/network.cpp src/arp.cpp src/portscan.cpp src/oui_lookup.cpp src/gui.cpp ^
    /link /LIBPATH:"%VCToolsInstallDir%lib\x64" /LIBPATH:"%WindowsSdkDir%lib\%WindowsSdkVersion%\um\x64" ^
    iphlpapi.lib ws2_32.lib comctl32.lib /SUBSYSTEM:WINDOWS

if %ERRORLEVEL% NEQ 0 (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

echo.
echo Build successful! Output: nnetscan.exe
