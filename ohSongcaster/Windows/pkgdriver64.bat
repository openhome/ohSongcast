@echo off
copy Driver\ohSongcaster.inf Build\Driver64\ohSongcaster.inf >nul
copy Driver\drmsimp\objchk_win7_amd64\amd64\vaddrms.sys Build\ohSongcaster64.sys >nul
copy Driver\drmsimp\objchk_win7_amd64\amd64\vaddrms.sys Build\Driver64\ohSongcaster.sys >nul
Signtool sign /v /a /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver64\ohSongcaster.sys >nul
inf2cat.exe /driver:Build\Driver64 /os:Vista_X64,7_X64 /v >nul
Signtool sign /v /a /ac Driver\gsrootr1.crt /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver64\ohSongcaster.cat >nul
Signtool verify /pa /c Build\Driver64\ohSongcaster.cat Build\Driver64\ohSongcaster.sys
