Name "SpecEd 2011"
OutFile "SpecEd2011_Full.exe"
InstallDir $PROGRAMFILES\SpecEd2011

InstallDirRegKey HKLM "Software\SpecEd2011" "Install_Dir"

page directory
page instfiles


uninstpage uninstConfirm
uninstpage instfiles


BrandingText "SpecEd 2011/2012 PSU ARL"
Caption "SpecEd 2011/2012"

Function .onInit
	InitPluginsDir
	File /oname=$PLUGINSDIR\splash.bmp ".\splash.bmp"
	splash::show 2000 $PLUGINSDIR\splash

  Pop $0
FunctionEnd


section "SpecEd2011 (required)"
  SetOutPath $INSTDIR
  WriteRegStr HKLM "SOFTWARE\SpecEd2011" "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpecEd2011" "DisplayName" "SpecEd 2011/2012"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpecEd2011" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpecEd2011" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpecEd2011" "NoRepair" 1
  
  file /r "..\..\*.*"
  

  ClearErrors
  FileOpen $0 $INSTDIR\SpecEdAudit\SpecEd2011.bat w
  IfErrors done1
  FileWrite $0 "@set SPECEDHOME=$INSTDIR"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set EDITOR=%SystemRoot%\system32\notepad.exe"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set PATH=%SPECEDHOME%\perl\bin;%SPECEDHOME%\instantclient_11_2;%SPECEDHOME%\PostgreSQL\8.3\bin;%PATH%"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set TNS_ADMIN=%SPECEDHOME%\instantclient_11_2"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set NLS_LANG=AMERICAN"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set TZ=EST5EDT"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@set perl5lib="
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  FileWrite $0 "@"
  FileWriteByte $0 "34"
  FileWrite $0 "%SPECEDHOME%\perl\bin\perl.exe"
  FileWriteByte $0 "34"
  FileWrite $0 " SpecEd2011.pl --gui --level1 > console.txt"
  FileWriteByte $0 "13"
  FileWriteByte $0 "10"
  
  FileClose $0
  
done1:
 
  Strcpy $OUTDIR "$INSTDIR\perl\installscripts"
  ExecWait '"$INSTDIR\perl\bin\perl.exe" configscript.pl'
  
exitok:
    
  
  CreateDirectory "$SMPROGRAMS\SpecEd2011"
  Strcpy $OUTDIR "$INSTDIR\SpecEdAudit"
  CreateShortCut "$SMPROGRAMS\SPECED2011\SpecEd2011.lnk" "$INSTDIR\SpecEdAudit\SpecEd2011.bat" SW_SHOWMINIMIZED
  CreateShortCut "$SMPROGRAMS\SpecEd2011\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
  WriteUninstaller $INSTDIR\uninstall.exe
  
 
sectionend


section "Uninstall"
  deleteregkey HKLM "Software\SpecEd2011"
  deleteregkey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\SpecEd2011"
  
  rmdir /r "$INSTDIR"
  rmdir /r "$SMPROGRAMS\SpecEd2011"
sectionend
