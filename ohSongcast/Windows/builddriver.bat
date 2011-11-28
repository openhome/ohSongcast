@echo off
md Build >nul 2>nul
md Build\Driver32 >nul 2>nul
md Build\Driver64 >nul 2>nul
md Build\Obj >nul 2>nul
echo.
echo Making 32 bit driver
echo ====================
echo.
call cmd.exe /k "setenv.bat \WinDDK\7600.16385.1 chk x86 WIN7 && cd Driver && build && cd .. && pkgdriver32 && exit"
echo.
echo Making 64 bit driver
echo ====================
echo.
call cmd.exe /k "setenv.bat \WinDDK\7600.16385.1 chk x64 WIN7 && cd Driver && build && cd .. && pkgdriver64 && exit"
