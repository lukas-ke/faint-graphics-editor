!include "MUI2.nsh" ; Use the modern user interface
!define MUI_ICON "installer.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "instsplash.bmp"

Name "Faint Graphics Editor"
OutFile "Install Faint v$$VERSION.exe"

InstallDir "$PROGRAMFILES\Faint"
RequestExecutionLevel admin

!define MUI_COMPONENTSPAGE_NODESC ; No description on hover for components
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Section "-Faint Application"
$$FILES
WriteUninstaller "$INSTDIR\Uninstall.exe"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "DisplayName" "Faint Graphics Editor"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "DisplayIcon" "$\"$INSTDIR\graphics\faint-icon.ico$\""
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "Publisher" "Lukas Kemmer"
WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "DisplayVersion" "$$VERSION"
WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint" \
                 "EstimatedSize" "$$SIZE"
SectionEnd

Section /o "Desktop shortcut"
CreateShortCut "$DESKTOP\Faint.lnk" "$INSTDIR\Faint.exe" ""
SectionEnd

Section "Start menu entry"
CreateDirectory "$SMPROGRAMS\Faint graphics editor"
CreateShortCut "$SMPROGRAMS\Faint graphics editor\Faint.lnk" "$INSTDIR\Faint.exe"
SectionEnd

Section "Uninstall"
$$UNINSTALLFILES
DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Faint"
Delete "$SMPROGRAMS\Faint graphics editor\Faint.lnk"
RMDir "$SMPROGRAMS\Faint graphics editor"
Delete "$DESKTOP\Faint.lnk"
SectionEnd
