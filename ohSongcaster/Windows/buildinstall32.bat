cl /nologo /FoBuild/Obj/Install32.obj -c /W4 /WX /EHsc /Gd -DDEFINE_LITTLE_ENDIAN -DDEFINE_TRACE -D_CRT_SECURE_NO_WARNINGS -DDllExport=__declspec(dllexport) -DDllExportClass= Install.cpp -I..\..\..\ohNet\Build\Include -I..\..\..\ohNet\Build\Include\Cpp
link /nologo /SUBSYSTEM:CONSOLE /map Iphlpapi.lib /incremental:no /out:Build\Obj\Install32.exe Build\Obj\Install32.obj setupapi.lib
copy Build\Obj\Install32.exe Build\Driver32\Install32.exe
signtool sign /v /a /t http://timestamp.verisign.com/scripts/timestamp.dll Build\Driver32\Install32.exe
copy Build\Obj\Install32.exe Build\Install32.exe
