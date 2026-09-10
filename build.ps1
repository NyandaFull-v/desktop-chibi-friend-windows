$ErrorActionPreference = 'Stop'
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'Visual Studio C++ desktop tools were not found.' }
$vcvars = Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
$root = Split-Path -Parent $MyInvocation.MyCommand.Path
$out = Join-Path $root 'dist'
if (Test-Path -LiteralPath $out) {
    $resolvedRoot = [IO.Path]::GetFullPath($root).TrimEnd('\')
    $resolvedOut = [IO.Path]::GetFullPath($out).TrimEnd('\')
    if (-not $resolvedOut.StartsWith($resolvedRoot + '\', [StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to clean an output folder outside the project: $resolvedOut"
    }
    Remove-Item -LiteralPath $resolvedOut -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $out | Out-Null
$cmd = 'call "' + $vcvars + '" && cd /d "' + $root + '" && rc /nologo app.rc && cl /nologo /utf-8 /std:c++20 /O1 /Os /GL /EHsc /DUNICODE /D_UNICODE /DNOMINMAX /DWIN32_LEAN_AND_MEAN /W4 /permissive- main.cpp image.cpp storage.cpp obs.cpp /link /LTCG /OPT:REF /OPT:ICF /SUBSYSTEM:WINDOWS /MANIFEST:NO /OUT:"' + (Join-Path $out 'DesktopChibiFriend.exe') + '" app.res user32.lib gdi32.lib shell32.lib ole32.lib oleaut32.lib oleacc.lib windowscodecs.lib comctl32.lib shlwapi.lib winmm.lib ws2_32.lib advapi32.lib dwmapi.lib psapi.lib wtsapi32.lib'
cmd /d /s /c $cmd
if ($LASTEXITCODE -ne 0) { throw "C++ build failed: $LASTEXITCODE" }
Copy-Item -LiteralPath (Join-Path $root 'README.txt') -Destination $out -Force
Copy-Item -LiteralPath (Join-Path $root 'characters') -Destination $out -Recurse -Force
Copy-Item -LiteralPath (Join-Path $root 'assets') -Destination $out -Recurse -Force
Copy-Item -LiteralPath (Join-Path $root 'docs') -Destination $out -Recurse -Force
Write-Host "Built: $out"
