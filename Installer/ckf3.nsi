!define PRODUCT_NAME "Chicken Fortress 3"
!define PRODUCT_VERSION "(Alpha 5)"
!define PRODUCT_PUBLISHER "hzqst"
!define PRODUCT_WEB_SITE "http://www.moddb.com/mods/chicken-fortress-3"

!include "MUI.nsh"

!define MUI_ABORTWARNING
!define MUI_ICON "ckf3.ico"

;!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
;!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "TradChinese"

!insertmacro MUI_RESERVEFILE_LANGDLL
!insertmacro MUI_RESERVEFILE_INSTALLOPTIONS

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "CKF3_Alpha5_Setup.exe"
InstallDir "$PROGRAMFILES\CKF3"
ShowInstDetails show
ShowUnInstDetails show
BrandingText " "

Section "MainSection" SEC01
  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  File /r "Files\*.*"
  CreateShortCut "$DESKTOP.lnk" "$INSTDIR\ckf3.exe"
SectionEnd

Function .onInit
  !insertmacro MUI_LANGDLL_DISPLAY

  Push $INSTDIR
  ReadRegStr $INSTDIR HKCU "SOFTWARE\Valve\Steam" "ModInstallPath"
  StrCmp $INSTDIR "" PopDir
  Goto Done

  PopDir:
    Pop $INSTDIR
  Done:

FunctionEnd