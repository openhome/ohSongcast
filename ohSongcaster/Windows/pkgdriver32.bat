@echo off
copy Driver\ohSongcaster.inf Build\Driver32\ohSongcaster.inf >nul
copy Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\ohSongcaster32.sys >nul
copy Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\Driver32\ohSongcaster.sys >nul
Signtool sign /v /a /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSongcaster.sys >nul
inf2cat.exe /driver:Build\Driver32 /os:Vista_X86,7_X86 /v >nul
Signtool sign /v /a /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSongcaster.cat >nul
Signtool verify /pa /c Build\Driver32\ohSongcaster.cat Build\Driver32\ohSongcaster.sys
