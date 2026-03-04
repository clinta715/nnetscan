$env:VCINSTALLDIR = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\"
$env:VCToolsInstallDir = "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Tools\MSVC\14.50.35717\"
$env:WindowsSdkDir = "C:\Program Files (x86)\Windows Kits\10\"
$env:WindowsSdkVersion = "10.0.26100.0"

$vcToolsVersion = "14.50"

$env:PATH = "$env:VCToolsInstallDir\bin\Hostx64\x64;$env:WindowsSdkDir\bin\$env:WindowsSdkVersion\x64;$env:PATH"

$cppStandardLib = "$env:VCToolsInstallDir\include"
$ucrtInclude = "$env:WindowsSdkDir\Include\$env:WindowsSdkVersion\ucrt"
$sharedInclude = "$env:WindowsSdkDir\Include\$env:WindowsSdkVersion\shared"
$umInclude = "$env:WindowsSdkDir\Include\$env:WindowsSdkVersion\um"
$winrtInclude = "$env:WindowsSdkDir\Include\10.0.26100.0\winrt"

$env:INCLUDE = "$cppStandardLib;$ucrtInclude;$sharedInclude;$umInclude;$winrtInclude"

$vcLib = "$env:VCToolsInstallDir\lib\x64"
$umLib = "$env:WindowsSdkDir\lib\$env:WindowsSdkVersion\um\x64"
$ucrtLib = "$env:WindowsSdkDir\lib\$env:WindowsSdkVersion\ucrt\x64"

$env:LIB = "$vcLib;$umLib;$ucrtLib"

Write-Host "Building NNetScan..."
Write-Host "INCLUDE: $env:INCLUDE"
Write-Host "LIB: $env:LIB"

$compileFlags = @(
    "/EHsc",
    "/W3",
    "/O1",
    "/GL",
    "/MT",
    "/D_WIN32_WINNT=0x0601",
    "/DWINVER=0x0601",
    "/D_CRT_SECURE_NO_WARNINGS",
    "/D_WINSOCK_DEPRECATED_NO_WARNINGS",
    "/Fe:nnetscan.exe"
)

$linkFlags = @(
    "/LIBPATH:`"$vcLib`"",
    "/LIBPATH:`"$umLib`"",
    "/LIBPATH:`"$ucrtLib`"",
    "iphlpapi.lib",
    "ws2_32.lib",
    "comctl32.lib",
    "user32.lib",
    "gdi32.lib",
    "/SUBSYSTEM:WINDOWS",
    "/OPT:REF",
    "/OPT:ICF",
    "/LTCG"
)

$sourceFiles = @(
    "src\main.cpp",
    "src\network.cpp",
    "src\arp.cpp",
    "src\portscan.cpp",
    "src\oui_lookup.cpp",
    "src\gui.cpp"
)

$clCmd = "cl.exe " + ($compileFlags -join " ") + " " + ($sourceFiles -join " ") + " /link " + ($linkFlags -join " ")
Write-Host "Running: $clCmd"

Invoke-Expression $clCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!"
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "Build successful! Output: nnetscan.exe"
