;--------------------------------
;Include Modern UI


  !include "MUI.nsh"
  !include "LogicLib.nsh"
  !include "WordFunc.nsh"
  !include "FileFunc.nsh"
  !include "nsDialogs.nsh"
  !include "x64.nsh"

  !insertmacro VersionCompare

  !define DOTNETURI "http://download.microsoft.com/download/7/0/3/703455ee-a747-4cc8-bd3e-98a615c3aedb/dotNetFx35setup.exe"
  !define PRODUCT "Songcaster"
  !define VERSION "0.0.0"
  !define LAUNCHFILE "ohSongcaster.exe"
  !define LAUNCHFILEDIR ""
  !define OUTFILE "..\Build\ohSongcasterInstaller.exe"
  !define ICON "ohLogo.ico"

  !define PATH "OpenHome"

  

;--------------------------------
;General

  ;Name and file
  Name ${PRODUCT}
  OutFile "${OUTFILE}"
  BrandingText " "
  XPStyle on

  ; next line required to allow removal of startmenu items on VISTA
  ;  but it doesn't work on our Linux version of makensis.exe
  ;RequestExecutionLevel admin

;--------------------------------
;Variables

  Var InstallDotNET

;--------------------------------
;Default installation folder

  InstallDir "$PROGRAMFILES\OpenHome\${PRODUCT}"


;--------------------------------
;Interface Configuration

  !define MUI_ICON ohLogo.ico
  !define MUI_UNICON ohLogo.ico

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP Header.bmp
  !define MUI_WELCOMEFINISHPAGE_BITMAP WelcomeFinish.bmp
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP UnwelcomeFinish.bmp

  !define MUI_ABORTWARNING


;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE License.txt
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE PageDirectoryLeave
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES  

  ;FINISH page settings
  !define MUI_FINISHPAGE_TITLE "Completing the Setup Wizard for ${PRODUCT}"
  !define MUI_FINISHPAGE_TEXT "${PRODUCT} has been installed on your computer.\n\nClick Finish to close this wizard."
  
  !define MUI_FINISHPAGE_SHOWREADME
  !define MUI_FINISHPAGE_SHOWREADME_TEXT "Launch ${PRODUCT}"
  !define MUI_FINISHPAGE_SHOWREADME_FUNCTION LaunchLink

  !define MUI_FINISHPAGE_RUN
  !define MUI_FINISHPAGE_RUN_TEXT "Create desktop shortcut"
  !define MUI_FINISHPAGE_RUN_FUNCTION CreateDesktopShortcut

  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  
  !define MUI_FINISHPAGE_TITLE "Uninstall Application Settings"
  !define MUI_FINISHPAGE_TEXT "Do you wish to uninstall all ${PRODUCT} application settings (e.g. User option files, plugins, etc)"
  
  !define MUI_FINISHPAGE_RUN
  !define MUI_FINISHPAGE_RUN_TEXT "Uninstall application settings"
  !define MUI_FINISHPAGE_RUN_FUNCTION un.RemoveApplicationData
  !define MUI_FINISHPAGE_RUN_NOTCHECKED
  !define MUI_FINISHPAGE_NOREBOOTSUPPORT
  
  !insertmacro MUI_UNPAGE_FINISH
  !insertmacro MUI_UNPAGE_FINISH

Function LaunchLink
    ExecShell "open" "$INSTDIR\${LAUNCHFILE}"
FunctionEnd

Function CreateDesktopShortcut
    CreateShortCut "$DESKTOP\${PRODUCT}.lnk" "$INSTDIR\${LAUNCHFILE}" ""
FunctionEnd

Function un.RemoveApplicationData
    RMDir /r "$LOCALAPPDATA\${PRODUCT}"
FunctionEnd

Function IsDotNETInstalled
  Push $0
  Push $1
  
  ReadRegDWORD $1 HKLM 'SOFTWARE\Microsoft\NET Framework Setup\NDP\v2.0.50727' Install
  
  ${If} $1 == ''
    StrCpy $0 "not installed"
  ${Else}
    StrCpy $0 "installed"
  ${EndIf}

  Pop $1
  Exch $0
FunctionEnd

Function AddToFirewallExceptions
    SimpleFC::AllowDisallowExceptionsNotAllowed 0 ; enable exceptions
    Pop $0
    SimpleFC::AddApplication "${PRODUCT}" "$INSTDIR\${LAUNCHFILE}" 0 2 "" 1 ; add to exceptions list
    Pop $0
    SimpleFC::EnableDisableApplication "$INSTDIR\${LAUNCHFILE}"  1 ; enable in exceptions list
    Pop $0
FunctionEnd

Function un.RemoveFromFirewallExceptions
    SimpleFC::RemoveApplication "$INSTDIR\${LAUNCHFILE}"
    Pop $0
FunctionEnd

;------------------------------------------------------------------------------
; NextVersionString
;
; returns substring of $1 contents - from 1st char upto (but not inc) delimiter
; returns substring in $1
; $2 and $3 are rserved
;
!macro SHARINGMACRO un
Function ${un}NextVersionString
    Pop $1
    Pop $0
    Push $2
    Push $3

    StrCpy $1 ""
    StrCpy $3 0             ;reset char count

    ${If} $0 == ""          ;if supplied buffer is empty...
        ;MessageBox MB_OK|MB_ICONSTOP "GFS: suplied buffer is empty - exiting"
        Goto GFSexit        ;exit
    ${Endif}

GFSstartloop:
    ;MessageBox MB_OK|MB_ICONSTOP "GFS:$0"
    StrCpy $2 $0 1 $3       ;copy next char from $0 into $2
    ;MessageBox MB_OK|MB_ICONSTOP "GFS:read 1 char:$2"
    ${If} $2 == ","         ;if the char is delimiter
        ;MessageBox MB_OK|MB_ICONSTOP "GFS:read delimiter - about to copy $3 chars then exit"
        StrCpy $1 $0 $3 ""  ;copy all chars upto (but not inc delimiter)
        ;MessageBox MB_OK|MB_ICONSTOP "GFS:read delimiter - $1"
        IntOp $3 $3 + 1     ;increment value in $3 (to discard the delimiter)
        StrCpy $0 $0 "" $3  ;copy into $0 contents of $0 (no max length) offset by count ($3) from start of string
        ;MessageBox MB_OK|MB_ICONSTOP "GFS:read delimiter - $0"
        Goto GFSexit
    ${Endif}

    IntOp $3 $3 + 1         ;increase char count...
    ;MessageBox MB_OK|MB_ICONSTOP "GFS:increased char count to $3 - now looping"
    Goto GFSstartloop       ;and loop back

GFSexit:
    ;MessageBox MB_OK|MB_ICONSTOP "GFS:exiting"
    
    Pop $3
    Pop $2
    Push $0
    Push $1
FunctionEnd
!macroend
;------------------------------------------------------------------------------


;------------------------------------------------------------------------------
; UninstallPreviousVersions
;
; uninstall all previously installed versions
;
; The "InstalledVersions" reg entry contains a list of all installed versions in the form
; "${PRODUCT}-w.x-y.z,${PRODUCT}-a.b-c.d,..." etc (comma separated)
;
; There are various "MessageBox..." lines in this function, which were used for debugging - They been commented out.
;
; "InstalledVersions" string is read into $1
; GetFirstSubstring is called. This reads string in $1 and returns (in $1) the first substring
; This substring consists of a version for a single installation in the form "App-w.x-y.z"
; This is used to identify and locate a specific "UninstallString" reg entry.
; This reg entry contains the path to the location of the uninstall.exe for that installed version
; The uninstall.exe is then called to remove this version of the application.
; This process is repeated until GetFirstSubstring returns an empty string.
; Each time round the loop we read "InstalledVersions" with an increasing offset stored in $2
; The offset defines how many chars to discrd from the beginning of the string.
; At the end of each loop $2 is increased by length of substring +1 (+1 is for the delimiter)
;
Function UninstallPreviousVersions
    Push $0
    Push $1
    Push $2
    Push $3

    StrCpy $2 0 ; reset counter

    ReadRegStr $0 HKLM "Software\OpenHome\${PRODUCT}" "InstalledVersions"
Loop:
    ;MessageBox MB_OK|MB_ICONSTOP "About to read registry list $0"
    
    Push $0
    Push $1
    Call NextVersionString
    Pop $1
    Pop $0

    ;MessageBox MB_OK|MB_ICONSTOP "Parser returned:$1"
    ;MessageBox MB_OK|MB_ICONSTOP "Parser returned list:$0"

    StrCmp $1 "" Exit 0 ; if previous versions list is empty exit

    ;MessageBox MB_OK|MB_ICONSTOP "Checking reg key for $1"
    ReadRegStr $3 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-$1" "UninstallString"
    StrCmp $3 "" +3 0 ; jump 3 lines if string is empty, jump 0 lines if it's not
    ExecWait '"$3" /S _?=$INSTDIR'
    ;MessageBox MB_OK|MB_ICONSTOP "Uninstalled $3"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-$1"
    Goto Loop

Exit:
    ;MessageBox MB_OK|MB_ICONSTOP "<UninstallPreviousVersions"
    
    Pop $3
    Pop $2
    Pop $1
    Pop $0
    ;Sleep 5000
FunctionEnd
;------------------------------------------------------------------------------


Function un.CleanInstalledVersions
    Push $0
    Push $1
    Push $2
    
    ReadRegStr $0 HKLM "Software\OpenHome\${PRODUCT}" "InstalledVersions"
    
Loop:
    Push $0
    Push $1
    Call un.NextVersionString
    Pop $1
    Pop $0
    
    StrCmp $1 "" Exit 0 ; if previous versions list is empty exit
    
    StrCmp $1 ${VERSION} Loop 0 ; remove uninstalled version string
    
    StrCpy $2 "$2$1," "" 0 ; concatenate versions that are still installed
    
Exit:
    ;MessageBox MB_OK|MB_ICONSTOP "New InstalledVersion string is $2"
    WriteRegStr HKLM "Software\OpenHome\${PRODUCT}" "InstalledVersions" '$2'

    Pop $0
    Pop $1
    Pop $2
FunctionEnd

Function .onInit
  ; check version of .Net installed
  ;
  Call IsDotNetInstalled
  Pop $0
  ${If} $0 == "not installed"
    StrCpy $InstallDotNET "Yes"
  ${Else}
    StrCpy $InstallDotNET "No"
  ${EndIf}
  
retry:
  ; The next code block tries to find if the application is currently running and alerts the user to close the app before retrying.
  ; FindWindow : 2nd param=window "class name" for the app, 3rd param="window caption" for the app 
  ; Any releases made in the past with a non generic title (window caption) need a dedicated 2 line code block of the format shown below

  System::Call 'kernel32::OpenMutex(i 0x100000, b 0, t "ohSongcasterMutex") i .R0'
    IntCmp $R0 0 continueInstall
        System::Call 'kernel32::CloseHandle(i $R0)'
        MessageBox MB_ICONSTOP|MB_RETRYCANCEL "${PRODUCT} is currently running. Please close the application and retry." IDRETRY retry
        Abort
continueInstall:
FunctionEnd

Function PageDirectoryLeave
    Call UninstallPreviousVersions
FunctionEnd

!insertmacro SHARINGMACRO ""
!insertmacro SHARINGMACRO "un."

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"


;--------------------------------
;Installer Sections

Section "OpenHome" SecOpenHome 
  SectionIn RO
  
  ${If} $InstallDotNet == "Yes"
     SetDetailsView hide
     inetc::get /caption "Downloading .NET Framework 3.5" /canceltext "Cancel" ${DOTNETURI} "$TEMP\dotNetFx35setup.exe" /end
     Pop $1
 
     ${If} $1 != "OK"
        Delete "$TEMP\dotNetFx35setup.exe"
        Abort "Installation cancelled."
     ${EndIf}
 
     ExecWait "$TEMP\dotNetFx35setup.exe"
     Delete "$TEMP\dotNetFx35setup.exe"
 
     SetDetailsView show
  ${EndIf}
  
  ; next line required to allow removal of startmenu items on VISTA
  SetShellVarContext all

  SetOutPath "$INSTDIR\"
  File ..\Wpf\ohSongcaster\bin\Release\ohSongcaster.exe
  SetOutPath "$INSTDIR\"
  File ..\Wpf\ohSongcaster\bin\Release\ohSongcaster.net.dll
  SetOutPath "$INSTDIR\"
  File ..\Wpf\ohSongcaster\bin\Release\ohSongcaster.dll
  SetOutPath "$INSTDIR\Driver64"
  File ..\Build\Driver64\ohSongcaster.cat
  SetOutPath "$INSTDIR\Driver64"
  File ..\Build\Driver64\ohSongcaster.inf
  SetOutPath "$INSTDIR\Driver64"
  File ..\Build\Driver64\ohSongcaster.sys
  SetOutPath "$INSTDIR\Driver64"
  File ..\Build\Driver64\Install64.exe
  SetOutPath "$INSTDIR\Driver32"
  File ..\Build\Driver32\ohSongcaster.cat
  SetOutPath "$INSTDIR\Driver32"
  File ..\Build\Driver32\ohSongcaster.inf
  SetOutPath "$INSTDIR\Driver32"
  File ..\Build\Driver32\ohSongcaster.sys
  SetOutPath "$INSTDIR\Driver32"
  File ..\Build\Driver32\Install32.exe
  
  ${If} ${RunningX64}
     SetOutPath "$INSTDIR\Driver64"
     ExecWait '"$INSTDIR\Driver64\Install64.exe" -i av.openhome.org ohSongcaster.inf'
  ${Else}
     SetOutPath "$INSTDIR\Driver32"
     ExecWait '"$INSTDIR\Driver32\Install32.exe" -i av.openhome.org ohSongcaster.inf'
  ${EndIf}

  ;Store installation folder
  WriteRegStr HKCU "${PATH}" "" "$INSTDIR"

  ;Create uninstaller
  SetOutPath "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Create shortcuts

  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\${PATH}"
  CreateShortCut "$SMPROGRAMS\${PATH}\${PRODUCT}.lnk" "$INSTDIR\${LAUNCHFILE}" ""
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ControlPanel\NameSpace\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "(Default)" "${PRODUCT}"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "(Default)" "${PRODUCT}"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "System.ApplicationName" "OpenHome.${PRODUCT}"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "System.ControlPanel.Category" "2,8"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "LocalizedString" "${PRODUCT}"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "InfoTip" "${PRODUCT}"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}" "System.Software.TasksFileUrl" "$INSTDIR\Tasks.xml"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}\DefaultIcon" "(Default)" "$INSTDIR\SongcasterPreferences.exe"
  WriteRegStr HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}\Shell\Open\Command" "(Default)" "$INSTDIR\SongcasterPreferences.exe"

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}" "DisplayName" "${PRODUCT} ${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}" "Publisher" "OpenHome"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}" "NoRepair" 1

  Call AddToFirewallExceptions
  
  ; add this version to the list of installed versions
  ReadRegStr $0 HKLM "Software\OpenHome\${PRODUCT}" "InstalledVersions"
  StrCpy $0 "$0${VERSION}," "" "" ; copy string, no max len, no offset
  WriteRegStr HKLM "Software\OpenHome\${PRODUCT}" "InstalledVersions" '$0'

  ;MessageBox MB_OK|MB_ICONSTOP "The following versions are installed : $0"

SectionEnd


;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecOpenHome ${LANG_ENGLISH} "OpenHome"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecOpenHome} $(DESC_SecOpenHome)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Call un.RemoveFromFirewallExceptions

  ; next line required to allow removal of startmenu items on VISTA
  SetShellVarContext all
  
  ${If} ${RunningX64}
     SetOutPath "$INSTDIR\Driver64"
     ExecWait '"$INSTDIR\Driver64\Install64.exe" -d av.openhome.org ohSongcaster.inf'
  ${Else}
     SetOutPath "$INSTDIR\Driver32"
     ExecWait '"$INSTDIR\Driver32\Install32.exe" -d av.openhome.org ohSongcaster.inf'
  ${EndIf}
  
  SetOutPath $TEMP

  ;delete the installed files
  Delete "$INSTDIR\\Songcaster.exe"
  Delete "$INSTDIR\\ohSongcaster.net.dll"
  Delete "$INSTDIR\\ohSongcaster.dll"
  Delete "$INSTDIR\Driver64\Songcaster.cat"
  Delete "$INSTDIR\Driver64\Songcaster.inf"
  Delete "$INSTDIR\Driver64\Songcaster.sys"
  Delete "$INSTDIR\Driver64\Install64.exe"
  Delete "$INSTDIR\Driver32\Songcaster.cat"
  Delete "$INSTDIR\Driver32\Songcaster.inf"
  Delete "$INSTDIR\Driver32\Songcaster.sys"
  Delete "$INSTDIR\Driver32\Install32.exe"
  RMDir "$INSTDIR\Driver32"
  RMDir "$INSTDIR\Driver64"
  RMDir "$INSTDIR\."

  Delete "$INSTDIR\*.log"
  Delete "$INSTDIR\*.crash"
  Delete "$INSTDIR\Uninstall.exe"

  ;delete the install folder if it's empty
  RMDir "$INSTDIR"

  ; now delete the Start Menu files and folders
  Delete "$SMPROGRAMS\${PATH}\${PRODUCT}.lnk"
  RMDir "$SMPROGRAMS\${PRODUCT}"
  RMDir "$SMPROGRAMS\${PATH}"
  
  ;delete the desktop shortcut
  Delete "$DESKTOP\${PRODUCT}.lnk"
  
  Call un.CleanInstalledVersions

  DeleteRegKey HKCU "${PATH}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Explorer\ControlPanel\NameSpace\{3CD797B4-05E4-4032-9759-6384EF20A7DC}"
  DeleteRegKey HKCR "CLSID\{3CD797B4-05E4-4032-9759-6384EF20A7DC}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT}-${VERSION}"

SectionEnd
