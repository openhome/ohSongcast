@echo off
call builddriver.bat
call buildinstall.bat
copy ..\..\Build\Obj\Windows\Release\ohSongcaster.dll Build\ohSongcaster.dll
copy ..\..\Build\Obj\Windows\Release\ohSongcaster.net.dll Build\ohSongcaster.net.dll
copy Driver\ohSongcaster.inf Build\ohSongcaster.inf
msbuild Wpf\ohSongcaster\ohSongcaster.sln /target:ohSongcaster /p:Configuration=Release
"%ProgramFiles(x86)%\NSIS\makensis.exe" Installer\Installer.nsi
Signtool sign /v /a /t http://timestamp.verisign.com/scripts/timestamp.dll Build\ohSongcasterInstaller.exe