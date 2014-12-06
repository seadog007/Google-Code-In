﻿ /****************************************************************************************
 * Copyright (c) 2012 Patrick von Reth <vonreth@kde.org>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/
 
/*
This script requires the Unicode NSIS framework http://www.scratchpaper.com/
You will also need to install http://nsis.sourceforge.net/Nsis7z_plug-in
*/
; registry stuff
!define regkey "Software\${company}\Amarok"
!define uninstkey "Software\Microsoft\Windows\CurrentVersion\Uninstall\Amarok"
 
!define startmenu "$SMPROGRAMS\Amarok"
!define uninstaller "uninstall.exe"
 
 Var StartMenuFolder
 
!define MUI_LANGDLL_ALLLANGUAGES
!define MUI_ICON "amarok.ico"
!define PRODUCT_WEB_SITE http://amarok.kde.org/

;save language
!define MUI_LANGDLL_REGISTRY_ROOT "HKLM" 
!define MUI_LANGDLL_REGISTRY_KEY "${regkey}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "${regkey}" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
;--------------------------------
Unicode true
XPStyle on
ShowInstDetails hide
ShowUninstDetails hide

SetCompressor /SOLID lzma


Name "${productname}"
Caption "${productname}"
 
OutFile "${setupname}"
 
!include "MUI2.nsh"
; all required LangStrings
!include "amarok_translation.nsh"
; a list of all supported language packages
!include "languages.nsh"

SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
 
InstallDir "$PROGRAMFILES\Amarok"
InstallDirRegKey HKLM "${regkey}" ""
 
;--------------------------------
 
AutoCloseWindow false
ShowInstDetails hide


!insertmacro MUI_PAGE_WELCOME
!define MUI_PAGE_HEADER_TEXT $(PAGE_LICENSE_HEADER_TEXT)
!define MUI_PAGE_HEADER_SUBTEXT $(PAGE_LICENSE_SUBTEXT)
!define MUI_LICENSEPAGE_TEXT_BOTTOM " "
!define MUI_LICENSEPAGE_BUTTON $(^NextBtn)
!insertmacro MUI_PAGE_LICENSE "..\..\COPYING"
!insertmacro MUI_PAGE_DIRECTORY 
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_LINK "$(VISIT_PROJECT_HOMEPAGE)"
!define MUI_FINISHPAGE_LINK_LOCATION "${PRODUCT_WEB_SITE}"
!define MUI_FINISHPAGE_RUN "$INSTDIR\bin\amarok.exe"
!insertmacro MUI_PAGE_FINISH


!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
 
 

Section "Amarok" SECTION_AMAROK
    SectionIn RO
    SetOutPath $INSTDIR
    SetShellVarContext all
    
    ExecWait '"$INSTDIR\bin\kdeinit4.exe" "--terminate"'
    WriteRegStr HKLM "${regkey}" "Install_Dir" "$INSTDIR"
    WriteRegStr HKLM "${regkey}" "Version" "${version}"
    WriteRegStr HKLM "${regkey}" "" "$INSTDIR\bin\amarok.exe"
    
    WriteRegStr HKLM "${uninstkey}" "DisplayName" "Amarok (remove only)"
    WriteRegStr HKLM "${uninstkey}" "DisplayIcon" "$INSTDIR\${MUI_ICON}"
    WriteRegStr HKLM "${uninstkey}" "DisplayVersion" "${version}"
    WriteRegStr HKLM "${uninstkey}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegStr HKLM "${uninstkey}" "UninstallString" '"$INSTDIR\${uninstaller}"'
    WriteRegStr HKLM "${uninstkey}" "Publisher" "${company}"

    ; package all files, recursively, preserving attributes
    ; assume files are in the correct places

    File /a /r /x "*.nsi" /x "${setupname}" "${srcdir}\*.*"
    File ${MUI_ICON}
    File "/oname=$INSTDIR\bin\kde.conf" "kde.conf"

    WriteUninstaller "${uninstaller}"

    ;Create shortcuts
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Amarok.lnk" "$INSTDIR\bin\Amarok.exe"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\$(APPEARANCE_SETTINGS).lnk" "$INSTDIR\bin\kcmshell4.exe" "style" "$INSTDIR\bin\systemsettings.exe"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\$(LANGUAGE_SETTINGS).lnk" "$INSTDIR\bin\kcmshell4.exe" "language" "$INSTDIR\bin\systemsettings.exe"
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\uninstall.exe"      
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section /o $(SECTION_SNORE) SECTION_SNORE
    !insertmacro KDE_FETCH_AND_EXTRACT "http://winkde.org/~pvonreth/downloads/Amarok/${version}/" "snorenotify-${architecture}-0.3.7z" "Installing Snore..."
    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
        CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Snorenotify.lnk" "$INSTDIR\bin\snorenotify.exe"
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

!insertmacro KDE_LANGUAGE_PACKAGES

Section
    ExecWait '"$INSTDIR\bin\update-mime-database.exe" "$INSTDIR\share\mime"'
    ExecWait '"$INSTDIR\bin\kbuildsycoca4.exe" "--noincremental"'
SectionEnd

; Uninstaller
; All section names prefixed by "Un" will be in the uninstaller
 
Section "Uninstall"
    SetShellVarContext all
    ExecWait '"$INSTDIR\bin\kdeinit4.exe" "--terminate"'

    DeleteRegKey HKLM "${uninstkey}"
    DeleteRegKey HKLM "${regkey}"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

    RMDir /r "$SMPROGRAMS\$StartMenuFolder"
    RMDir /r "$INSTDIR"
SectionEnd



;initialize the translations
!insertmacro AMAROK_TRANSLATIONS


;installer Fcuntion
Function .onInit

    !insertmacro MUI_LANGDLL_DISPLAY
    !insertmacro INIT_KDE "amarok"

    
    ReadRegStr $R0 HKLM "${uninstkey}" "UninstallString"
    StrCmp $R0 "" done
    ReadRegStr $INSTDIR HKLM "${regkey}" "Install_Dir"
    ;Run the uninstaller
    ;uninst:
    ClearErrors
    ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
    done:
FunctionEnd

; Uninstaller Functions

Function un.onInit

    !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd

  
 



