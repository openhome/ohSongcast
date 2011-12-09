@echo off
copy Driver\ohSongcast.inf Build\Driver32\ohSongcast.inf >nul
copy Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\ohSongcast32.sys >nul
copy Driver\drmsimp\objchk_win7_x86\i386\vaddrms.pdb Build\ohSongcast32.pdb >nul
copy Driver\drmsimp\objchk_win7_x86\i386\vaddrms.sys Build\Driver32\ohSongcast.sys >nul
Signtool sign /v /a /sm /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSongcast.sys >nul
inf2cat.exe /driver:Build\Driver32 /os:Vista_X86,7_X86 /v >nul
Signtool sign /v /a /sm /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\ohSongcast.cat >nul
Signtool verify /pa /c Build\Driver32\ohSongcast.cat Build\Driver32\ohSongcast.sys
