@echo off
call builddriver.bat
call buildinstall.bat
msbuild Wpf\ohSoundcard\ohSoundcard.sln /target:ohSoundcard /p:Configuration=Release

