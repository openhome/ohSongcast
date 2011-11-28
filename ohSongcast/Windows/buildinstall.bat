@echo off
md Build >nul 2>nul
md Build\Driver32 >nul 2>nul
md Build\Driver64 >nul 2>nul
md Build\Obj >nul 2>nul
echo.
echo Making Install32.exe
echo ====================
echo.
call cmd.exe /k ""%VCINSTALLDIR%\bin\vcvars32.bat" && buildinstall32 && exit"
echo.
echo Making Install64.exe
echo =====================
echo.
call cmd.exe /k ""%VCINSTALLDIR%\bin\amd64\vcvars64.bat" && buildinstall64 && exit"



