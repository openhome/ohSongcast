@echo off
call builddriver.bat
call buildinstall.bat
copy ..\..\Build\Obj\Windows\Release\ohSongcast.dll Build\ohSongcast.dll
copy ..\..\Build\Obj\Windows\Release\ohSongcast.net.dll Build\ohSongcast.net.dll
copy Driver\ohSongcast.inf Build\ohSongcast.inf
msbuild Wpf\ohSongcast\ohSongcast.sln /target:ohSongcast /p:Configuration=Release
"%ProgramFiles(x86)%\NSIS\makensis.exe" Installer\Installer.nsi
Signtool sign /v /a /t http://timestamp.verisign.com/scripts/timestamp.dll Build\ohSongcastInstaller.exe