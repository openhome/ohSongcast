@echo off
call builddriver.bat
nmake /f Windows.mak 
call cmd.exe /k ""%VCINSTALLDIR%\bin\amd64\vcvars64.bat" && nmake /f Windows64.mak && exit"
nmake /f Windows.mak release=1
call cmd.exe /k ""%VCINSTALLDIR%\bin\amd64\vcvars64.bat" && nmake /f Windows64.mak release=1 && exit"

msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:ohSoundcard /p:Configuration=Debug
msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:Installer32 /p:Configuration=Debug
msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:Installer64 /p:Configuration=Debug

msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:ohSoundcard /p:Configuration=Release
msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:Installer32 /p:Configuration=Release
msbuild ohSoundcard\Windows\Wpf\ohSoundcard\ohSoundcard.sln /target:Installer64 /p:Configuration=Release

md Build\Installers >nul 2>nul

copy ohSoundcard\Windows\Wpf\Installer32\bin\Release\ohSoundcard32.msi Build\Installers
copy ohSoundcard\Windows\Wpf\Installer64\bin\Release\ohSoundcard64.msi Build\Installers
