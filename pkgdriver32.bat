@echo off
md Build\Driver32 >nul 2>nul
md Build\DriverRaw >nul 2>nul
copy ohSoundcard\Windows\Driver\ohSoundcard.inf Build\Driver32\ohSoundcard.inf >nul
copy ohSoundcard\Windows\Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\Driver32\ohSoundcard.sys >nul
copy ohSoundcard\Windows\Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\DriverRaw\ohSoundcard32.sys >nul
Signtool sign /v /a /ac ohSoundcard\Windows\Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSoundcard.sys >nul
inf2cat.exe /driver:Build\Driver32 /os:Vista_X86,7_X86 /v >nul
Signtool sign /v /a /ac ohSoundcard\Windows\Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSoundcard.cat >nul
Signtool verify /pa /c Build\Driver32\ohSoundcard.cat Build\Driver32\ohSoundcard.sys
