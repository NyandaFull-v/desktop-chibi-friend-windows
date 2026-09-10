$ErrorActionPreference='Stop'
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs=& $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(-not $vs){throw 'Visual Studio C++ desktop tools were not found.'}
$vcvars=Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
$root=Split-Path -Parent $MyInvocation.MyCommand.Path
$cmd='call "'+$vcvars+'" && cd /d "'+$root+'" && cl /nologo /utf-8 /std:c++20 /O1 /EHsc /DUNICODE /D_UNICODE /DNOMINMAX /DWIN32_LEAN_AND_MEAN qa.cpp image.cpp storage.cpp /link /SUBSYSTEM:CONSOLE /OUT:qa.exe user32.lib gdi32.lib ole32.lib windowscodecs.lib'
cmd /d /s /c $cmd
if($LASTEXITCODE-ne 0){throw "QA build failed: $LASTEXITCODE"}
& (Join-Path $root 'qa.exe') (Join-Path $root 'dist')
if($LASTEXITCODE-ne 0){throw "QA failed: $LASTEXITCODE"}
