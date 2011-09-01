@echo off
md Build\Driver64 >nul 2>nul
md Build\DriverRaw >nul 2>nul
copy ohSoundcard\Windows\Driver\ohSoundcard.inf Build\Driver64\ohSoundcard.inf >nul
copy ohSoundcard\Windows\Driver\drmsimp\objchk_win7_amd64\amd64\vaddrms.sys Build\Driver64\ohSoundcard.sys >nul
copy ohSoundcard\Windows\Driver\drmsimp\objchk_win7_amd64\amd64\vaddrms.sys Build\DriverRaw\ohSoundcard64.sys >nul
Signtool sign /v /a /ac ohSoundcard\Windows\Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver64\ohSoundcard.sys >nul
inf2cat.exe /driver:Build\Driver64 /os:Vista_X64,7_X64 /v >nul
Signtool sign /v /a /ac ohSoundcard\Windows\Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver64\ohSoundcard.cat >nul
Signtool verify /pa /c Build\Driver64\ohSoundcard.cat Build\Driver64\ohSoundcard.sys
