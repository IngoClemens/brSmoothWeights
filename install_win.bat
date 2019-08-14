@echo off

:: ---------------------------------------------------------------------
:
: Installation script to install a module-based script or plug-in for
: Autodesk Maya.
:
: ----------------------------------------------------------------------

set copyright=Copyright (c) 2019 Ingo Clemens, brave rabbit
set installerVersion=0.9.0-190814

:: The name automatically gets defined throught the name of the module.
set name=

:: ---------------------------------------------------------------------
:  values overriden by module.cfg (if present and active)
:  ---------------------------------------------------------------------

set mayaVersion=2019
set pluginVersion=any
set iconPath=
set pluginPath=
set scriptPath=
set customPath=


:: ---------------------------------------------------------------------
:  system specific variables
:  ---------------------------------------------------------------------

set modulePath=C:\Users\$USER\Documents\maya\modules
set factoryPath=C:\Program_Files\Autodesk\Maya$VERSION\modules;C:\Users\$USER\Documents\maya\$VERSION\modules;C:\Users\$USER\Documents\maya\modules;C:\Program_Files\Common_Files\Alias_Shared\Modules\maya\$VERSION;C:\Program_Files\Common_Files\Alias_Shared\Modules\maya;C:\Program_Files\Common_Files\Autodesk_Shared\Modules\maya\$VERSION
set installPathFile=%TEMP%\moduleInstaller

set installPathList=%modulePath%
set "installPathList=%installPathList:$USER=!USERNAME!%"


:: ---------------------------------------------------------------------
:  general
:  ---------------------------------------------------------------------

:: Stores the user input.
set input=
set includePlugin=0
set logfile=

set installOption=1
set deleteOption=0

set doDelete=0
set deleteFile=
set deleteDir=

set doBackup=0
set backupPath=
set backupFile=
set backupDir=


:: ---------------------------------------------------------------------
:  begin
:  ---------------------------------------------------------------------

:: Delayed expansion is important to be able to retrieve variables from
:  within a for loop.
SETLOCAL ENABLEDELAYEDEXPANSION

:: Switch to the current directory and get it's path.
set currentDir=%cd%
cd %currentDir%

:: Setup the log file.
set logfile=%currentDir%\install.log
break > "%logfile%"
call :logStatus "Begin installation"

:: ---------------------------------------------------------------------
:  module configuration
:  ---------------------------------------------------------------------

:: Read any custom settings from the module.cfg file.
call :readConfig

call :logStatus "Maya version : %mayaVersion%"
call :logStatus "Module version : %pluginVersion%"
call :logStatus "Module icons : %iconPath%"
call :logStatus "Module plug-ins : %pluginPath%"
call :logStatus "Module scripts : %scriptPath%"
for /l %%n in (0,1,!customCount!) do call :logStatus "Module custom path : !customPath[%%n]!"

:: Get the global MAYA_APP_DIR environment variable.
:  If this is not defined use the default modules path.
if not "%MAYA_APP_DIR%" == "" (
    set modulePath=%MAYA_APP_DIR%\modules
    :: Add the MAYA_APP_DIR path to the factory paths when searching for
    :  existing modules.
    set factoryPath=%factoryPath%;%modulePath%
)

:: Check if the file exists which contains all previously used custom
:  install paths. Add all previous paths to the factory path array to
:  search for previous installations.
:  Also, create a list of all custom install paths to be able to give
:  the user an option to choose from previously used paths during the
:  custom installation.
set installPathFileExists=0
set installPathListCount=0
set savedCustomPath=
if exist %installPathFile% (
    for /f "tokens=* delims=" %%l in (%installPathFile%) do (
        set pathItem=%%l\
        set "pathItem=!pathItem: \=!"
        set "pathItem=!pathItem: =§!"
        set factoryPath=!factoryPath!;!pathItem!
        set installPathList=!installPathList!;!pathItem!
        if !installPathListCount! == 0 (
            set savedCustomPath=%%l
        )
        set /a installPathListCount+=1
    )
    set installPathFileExists=1
)

call :logStatus "MAYA_APP_DIR : %MAYA_APP_DIR%"
call :logStatus "Module path : %modulePath%"

:: ---------------------------------------------------------------------
:  header
:  ---------------------------------------------------------------------

echo  ---------------------------------------------------------------------
echo  Module installer for Autodesk Maya.
echo.
echo  Version: %installerVersion%
echo  moduleInstaller is under the terms of the MIT License
echo  %copyright%
echo  ---------------------------------------------------------------------
echo.

:: ---------------------------------------------------------------------
:  checks
:  ---------------------------------------------------------------------

:: Check if the source files for the installation are present.
:  The installer must be located in the same folder as the modules
:  folder which has to contain the folder with the name of the module.
set "sourceDir=%currentDir%\modules"
if exist "%sourceDir%"\* (
    :: The modules folder has to contain one folder with the name of
    :  the module.
    set count=0
    for /d %%d in ("%sourceDir%"\*) do (
        :: Get just the name of the folder.
        set name=%%~nd
        set /a count+=1
        call :logStatus "Source folder content : %%~nd"
    )
    if not !count! == 1 (
        echo Error: The modules folder must only contain one folder with the name of the module.
        call :logStatus "Error: The modules folder must only contain one folder with the name of the module."

        call :cancelInstall
    )
) else (
    echo.
    echo Error: The source files for the installation cannot be found.
    echo The module files must be located in:
    echo %sourceDir%
    call :logStatus "Error: The source files must be located in: %sourceDir%"

    call :cancelInstall
)

:: Get the name from the found folder in the modules folder. Because of
:  the delayed expansion the variable name is enclosed in '!'.
set name=!name!
call :logStatus "Module Name : %name%"

set "moduleBase=%sourceDir%\%name%"
call :logStatus "Module base path : %moduleBase%"

:: Check if the module contains a plug-in folder.
if exist "%moduleBase%"\plug-ins\* set includePlugin=1
call :logStatus "Module has plug-in : %includePlugin%"

:: ---------------------------------------------------------------------
:  intro
:  ---------------------------------------------------------------------

:: Inform about the installation and ask to continue.
:  Display the license, if any and cancel the installation if the
:  license doesn't get accepted.

echo This program installs %name% for Autodesk Maya.
echo.

:checkInputStart
set /p input="Do you want to continue? [y/n]: "
if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
    call :wrongInput
    goto :checkInputStart
)

set confirm=0
for %%i in (y Y) do if %%i == %input% set confirm=1

if !confirm! == 1 (
    :: -----------------------------------------------------------------
    :  display license
    :  -----------------------------------------------------------------
    if exist LICENSE (
        call :logStatus "Displaying license"
        echo.
        echo.

        type LICENSE

        echo.
        echo.

        :checkLicenseInput
        set /p input="Do you agree with the terms of the license? [y/n]: "
        if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
            call :wrongInput
            goto :checkLicenseInput
        )

        set confirm=0
        for %%i in (y Y) do if %%i == !input! set confirm=1

        if !confirm! == 0 (
            call :cancelInstall
        )

        echo.
    )

) else (
    call :cancelInstall
)

:: ---------------------------------------------------------------------
:  install option
:  ---------------------------------------------------------------------

:: The path the module file is saved to.
set moduleFilePath=""

echo.
echo Installation type:
echo    1. Simple
echo    2. Custom (Additional options)
echo    3. Create module file only (Manual setup)
echo    4. Uninstall
echo    5. Exit
echo.
set /p input="Please enter choice [1-5]: "
if !input! == 1 (
    set installOption=1
) else if !input! == 2 (
    set installOption=2
) else if !input! == 3 (
    set moduleFilePath=%currentDir%\%name%.mod
    set moduleFilePath=!moduleFilePath: \=\!
    set moduleFilePath=!moduleFilePath: =§!
    call :writeModuleFile !moduleFilePath!
    :: Replace the section subsitution for the output.
    set moduleFilePath=!moduleFilePath:§= !
    echo.
    echo Module file saved to:
    echo !moduleFilePath!
    call :finishInstall
) else if !input! == 4 (
    set installOption=2
    set deleteOption=1
) else if !input! == 5 (
    call :cancelInstall
) else (
    call :cancelInstall
)
call :logStatus "Install option : %installOption%"

:: ---------------------------------------------------------------------
:  cleanup existing modules
:  ---------------------------------------------------------------------

if %installOption% == 2 (
    echo.
    echo Checking for existing modules...
    call :logStatus "Checking for existing modules"
)

set visitedPaths=

:: Go through each Maya version and default module paths and check for
:  existing modules.
for %%v in (!mayaVersion!) do (
    for %%p in (%factoryPath%) do (
        :: Replace the version placeholder.
        set modPath=%%p
        set "modPath=!modPath:$VERSION=%%v!"
        set "modPath=!modPath:$USER=%USERNAME%!"

        set "modPath=!modPath:§= !"

        call :logStatus "Searching default path : !modPath!"

        :: Check if the current path already has been processed in case
        :  of non version specific paths.
        set processed=0
        for /f "tokens=* delims=;" %%i in ("!visitedPaths!") do (
            set visited=%%i
            set "visited=!visited:§= !"
            if !visited! == !modPath! (
                set processed=1
            )
        )

        :: Check if the module exists in one of the default locations.
        set modFile=!modPath!\%name%.mod
        if exist !modFile! if !processed! == 0 (
            call :logStatus "Found existing module file : !modFile!"
            :: Check where the module file points to.
            :  Get the content of the .mod file and parse the first
            :  line. The last item is the location of the module folder.
            set line=
            set lineSet=0
            for /f "USEBACKQtokens=* delims=\n" %%l in ("!modFile!") do (
                :: There doesn't seem to be an easy way to break from a
                :  loop. Therefore lineSet is used to mark that the
                :  first line has been processed.
                if !lineSet! == 0 (
                    set line=%%l
                    set lineSet=1
                )
            )
            call :logStatus "First line in module : !line!"

            :: Simple modules without a plugin only need a module
            :  description built from four elements:
            :  + name version path
            : When plugins are included these are usually version
            : specific so the Maya version needs to be defined as well:
            : + mayaVersion name version path
            set itemNum=4
            if !includePlugin! == 1 (
                set itemNum=5
            ) else (
                if /i "!line:~2,11!" == "MAYAVERSION" (
                    set itemNum=5
                )
            )
            call :logStatus "Required line items : !itemNum!"

            :: Get the path of the module content from the first line.
            :  Since splitting lines is a rather difficult task and
            :  arrays cannot be easily accessed loop through the items
            :  if the space-separated list and get the item at the index
            :  which matches the desired position.
            set modDir=
            set numItems=0
            :: Define the index for accessing the list of line items.
            :  Usually this would be items[itemNum-1] but it doesn't
            :  seem possible to decrement a number in place.
            set /a index=!itemNum!-1
            for %%i in (!line!) do (
                if !numItems! == !index! (
                    set modDir=%%i
                )
                set /a numItems+=1
            )

            :: Some folder names can contain spaces. If the line is
            :  split by spaces the path to the module gets split up as
            :  well. The path items then need to get joined.
            if !numItems! gtr !itemNum! (
                set count=!itemNum!
                set id=0
                for %%i in (!line!) do (
                    if !id! lss !numItems! (
                        if !id! gtr !index! (
                            set modDir=!modDir! %%i
                        )
                    )
                    set /a id+=1
                )
            ) else (
                if !numItems! lss !itemNum! (
                    echo.
                    echo Error: The following module file doesn't seem to be properly formatted.
                    echo The module description needs to have !itemNum! space-separated items.
                    echo !modFile!
                    call :logStatus "Error: Wrong formatting in module file."

                    call :cancelInstall
                )
            )
            call :logStatus "Existing module content : !modDir!"

            :: Only display existing modules and files when the custom
            :  install option has been selected.
            if %installOption% == 2 (
                echo.
                echo Found existing module:
                echo Module path:
                echo !modFile!
                echo Module content:
                echo !modDir!
                echo.

                :checkFileDelete
                set /p input="Do you want to delete the files? [y/n]: "
                if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
                    call :wrongInput
                    goto :checkFileDelete
                )
            ) else (
                set input=y
            )

            set confirm=0
            for %%i in (y Y) do if %%i == !input! set confirm=1

            if !confirm! == 1 (
                :: -----------------------------------------------------
                :  Delete old files
                :  -----------------------------------------------------
                set doDelete=1
                set deleteFile=!modFile!
                set deleteDir=!modDir!
                call :logStatus "Marked for deletion : !modFile!"
                call :logStatus "Marked for deletion : !modDir!"
            ) else (
                :: -----------------------------------------------------
                :  Backup old files
                :  -----------------------------------------------------
                echo.

                :checkFileBackup
                set /p input="Do you want to backup the files? [y/n]: "
                if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
                    call :wrongInput
                    goto :checkFileBackup
                )

                set confirm=0
                for %%i in (y Y) do if %%i == !input! set confirm=1

                if !confirm! == 1 (
                    set doBackup=1
                    call :getTimeStamp

                    set pathString=!modDir!
                    set "pathString=!pathString: =$!"
                    call :getBackupPath !pathString!


                    set backupPath=!backupPath!\%name%_!timestamp!
                    set backupFile=!modFile!
                    set backupDir=!modDir!
                    call :logStatus "Marked for backup : !modFile!"
                    call :logStatus "Marked for backup : !modDir!"
                    call :logStatus "Backup folder : !backupPath!"
                ) else (
                    echo.
                    echo Error: The previous files must be removed to continue with the installation.
                    call :logStatus "Error: The previous files must be removed to continue with the installation."
                    call :cancelInstall
                )
            )

            set replacePath=!modPath!
            set "replacePath=!replacePath: =§!"
            set visitedPaths=!visitedPaths!;!replacePath!
        )
    )
)

echo.

if %installOption% == 2 (
    echo ... Check completed.
    echo.
    call :logStatus "Check completed"
)

:: ---------------------------------------------------------------------
:  setup the module content path
:  ---------------------------------------------------------------------

:  If the custom install option has been chosen check if previous user
:  defined paths have been saved. In this case use the first stored path
:  as the default path for the installation.
if %installPathFileExists% == 1 if %installPathListCount% gtr 0 if %installOption% == 2 (
    set modulePath=%savedCustomPath%
    call :logStatus "Module path using stored custom path : !modulePath!"
)

set "modulePath=!modulePath:$USER=%USERNAME%!"
if %deleteOption% == 0 (
    echo The module will be installed in:
    echo %modulePath%
)

set moduleFilePath=%modulePath%

if %installOption% == 1 (
    if %doDelete% == 1 (
        echo.
        echo A previous installation has been detected and will be deleted.
    )

    echo.

    :checkContinue
    set /p input="Do you want to continue? [y/n]: "
    if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
        call :wrongInput
        goto :checkContinue
    )

    set confirm=0
    for %%i in (y Y) do if %%i == !input! set confirm=1

    if !confirm! == 0 (
        call :cancelInstall
    )
) else (
    :: Prompt for the installation path.
    if %installOption% == 2 if %deleteOption% == 0 (
        echo.

        :checkUsePath
        set /p input="Do you want to use this location? [y/n]: "
        if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
            call :wrongInput
            goto :checkUsePath
        )

        set confirm=0
        for %%i in (y Y) do if %%i == !input! set confirm=1

        if !confirm! == 0 (
            set needsPathInput=1

            :: In case the file exists which contains the previous install
            :  paths display these as a list the user can choose from.
            if %installPathFileExists% == 1 (
                echo.
                echo Previous install paths:
                set count=1
                for %%i in (!installPathList!) do (
                    set listItem=%%i
                    set "listItem=!listItem:§= !"
                    echo     !count!. !listItem!
                    set /a count+=1
                )
                echo     !count!. New path
                echo.
                set /p input="Please enter choice [1-!count!]: "

                if !input! lss !count! (
                    set count=1
                    for %%i in (!installPathList!) do (
                        if !count! == !input! (
                            set listItem=%%i
                            set "listItem=!listItem:§= !"
                            set modulePath=!listItem!
                            call :logStatus "User defined install path : !listItem!"
                            set needsPathInput=0
                        )
                        set /a count+=1
                    )
                ) else (
                    if !input! == !count! (
                        set needsPathInput=1
                    ) else (
                        call :cancelInstall
                    )
                )
            )

            if !needsPathInput! == 1 (
                echo.
                set /p input="Please enter a custom install path: "
                set userDefinedPath=!input!
                :: It's not possible to query if the input was empty when
                :  the return key was pressed because it doesn't qualify as
                :  being empty. Therefore the check is performed the other
                :  way around.
                if "!input!" == "" (
                    echo.
                    echo Error: No path provided for the installation.
                    call :logStatus "Error: No path provided for the installation."
                    call :cancelInstall
                ) else (
                    call :logStatus "User defined install path : !userDefinedPath!"
                    :: Check if the entered path is valid
                    if not exist "!input!\*" (
                        echo.
                        echo The path does not exist.
                        echo.

                        :checkCreatPath
                        set /p input="Do you want to create it? [y/n]: "
                        if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
                            call :wrongInput
                            goto :checkCreatPath
                        )

                        set confirm=0
                        for %%i in (y Y) do if %%i == !input! set confirm=1

                        if !confirm! == 1 (
                            md "!userDefinedPath!"
                            call :logStatus "Created folder : !userDefinedPath!"
                        ) else (
                            echo.
                            echo Error: No valid path provided for the installation.
                            call :logStatus "Error: No valid path provided for the installation."
                            call :cancelInstall
                        )
                    )
                    set modulePath=!userDefinedPath!

                    if %installPathFileExists% == 1 (
                        echo !userDefinedPath! >> %installPathFile%
                    ) else (
                        echo !userDefinedPath! > %installPathFile%
                    )
                    call :logStatus "Added user defined path !userDefinedPath! to %installPathFile%"
                )
            )
        )
    )
)

:: ---------------------------------------------------------------------
:  setup the module file path
:  ---------------------------------------------------------------------

set "modulePath=!modulePath:$USER=%USERNAME%!"
if %deleteOption% == 0 (
    echo.
    echo The module file %name%.mod will be created in:
    echo %moduleFilePath%
)

:: Prompt for the installation path.
if %installOption% == 2 if %deleteOption% == 0 (
    echo.

    :checkUseModuleFilePath
    set /p input="Do you want to use this location? [y/n]: "
    if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
        call :wrongInput
        goto :checkUseModuleFilePath
    )

    set confirm=0
    for %%i in (y Y) do if %%i == !input! set confirm=1

    if !confirm! == 0 (
        set needsPathInput=1

        :: In case the file exists which contains the previous install
        :  paths display these as a list the user can choose from.
        if %installPathFileExists% == 1 (
            echo.
            echo Previous install paths:
            set count=1
            for %%i in (!installPathList!) do (
                set listItem=%%i
                set "listItem=!listItem:§= !"
                echo     !count!. !listItem!
                set /a count+=1
            )
            echo     !count!. New path
            echo.
            set /p input="Please enter choice [1-!count!]: "

            if !input! lss !count! (
                set count=1
                for %%i in (!installPathList!) do (
                    if !count! == !input! (
                        set listItem=%%i
                        set "listItem=!listItem:§= !"
                        set moduleFilePath=!listItem!
                        call :logStatus "User defined module file path : !listItem!"
                        set needsPathInput=0
                    )
                    set /a count+=1
                )
            ) else (
                if !input! == !count! (
                    set needsPathInput=1
                ) else (
                    call :cancelInstall
                )
            )
        )

        if !needsPathInput! == 1 (
            echo.
            set /p input="Please enter a custom path for the module file: "
            set userDefinedModuleFilePath=!input!
            :: It's not possible to query if the input was empty when
            :  the return key was pressed because it doesn't qualify as
            :  being empty. Therefore the check is performed the other
            :  way around.
            if "!input!" == "" (
                echo.
                echo Error: No module file path provided for the installation.
                call :logStatus "Error: No module file path provided for the installation."
                call :cancelInstall
            ) else (
                call :logStatus "User defined module file path : !userDefinedModuleFilePath!"
                :: Check if the entered path is valid
                if not exist "!input!\*" (
                    echo.
                    echo The path does not exist.
                    echo.

                    :checkCreatPath
                    set /p input="Do you want to create it? [y/n]: "
                    if not !input! == y if not !input! == Y if not !input! == n if not !input! == N (
                        call :wrongInput
                        goto :checkCreatPath
                    )

                    set confirm=0
                    for %%i in (y Y) do if %%i == !input! set confirm=1

                    if !confirm! == 1 (
                        md "!userDefinedModuleFilePath!"
                        call :logStatus "Created folder : !userDefinedModuleFilePath!"
                    ) else (
                        echo.
                        echo Error: No valid module file path provided for the installation.
                        call :logStatus "Error: No valid module file path provided for the installation."
                        call :cancelInstall
                    )
                )
                set moduleFilePath=!userDefinedModuleFilePath!

                if !modulePath! neq !moduleFilePath! (
                    if %installPathFileExists% == 1 (
                        echo !userDefinedModuleFilePath! >> %installPathFile%
                    ) else (
                        echo !userDefinedModuleFilePath! > %installPathFile%
                    )
                    call :logStatus "Added user defined path !userDefinedModuleFilePath! to %installPathFile%"
                )
            )
        )
    )
)

:: ---------------------------------------------------------------------
:  Delete old files
:  ---------------------------------------------------------------------

if %doDelete% == 1 (
    del /q "!deleteFile!"
    rmdir /s /q "!deleteDir!"

    echo.
    echo Deleted previous module:
    echo !deleteFile!
    echo Deleted previous module content:
    echo !deleteDir!
    call :logStatus "Deleted previous module : !deleteFile!"
    call :logStatus "Deleted previous module content : !deleteDir!"
) else (
    :: -----------------------------------------------------------------
    :  Backup old files
    :  -----------------------------------------------------------------
    if %doBackup% == 1 (
        md "!backupPath!"
        xcopy /s "!backupDir!" "!backupPath!\%name%\" >NUL
        move "!backupFile!" "!backupPath!" >NUL

        rmdir /s /q "!backupDir!"

        echo.
        echo Moved old files to:
        echo !backupPath!
        call :logStatus "Moved old files to : !backupPath!"
    )
)

if %deleteOption% == 1 (
    call :finishInstall
)

:: ---------------------------------------------------------------------
:  Create the module
:  ---------------------------------------------------------------------

if not exist !moduleFilePath! (
    md "!moduleFilePath!"
    call :logStatus "Created module path : !moduleFilePath!"
) else (
    call :logStatus "Using existing module path : !moduleFilePath!"
)

echo.
echo Writing module file...

set moduleFilePath=!moduleFilePath!\%name%.mod
set moduleFilePath=!moduleFilePath: \=\!
set moduleFilePath=!moduleFilePath: =§!
call :writeModuleFile !moduleFilePath!

echo ... Done

echo.
echo Copying files...

set modulePath=!modulePath!\%name%\
set modulePath=!modulePath: \=\!
xcopy /s "!moduleBase!" "!modulePath!" >NUL

echo ... Done

echo.
echo Installed module to:
echo !modulePath!
call :logStatus "Installed module to : !modulePath!"

call :finishInstall


:: ---------------------------------------------------------------------
:  helper functions
:  ---------------------------------------------------------------------

:finishInstall
echo.
echo Installation log saved to:
echo %logfile%
echo.
echo ----------------------- Installation complete ------------------------
echo.
echo.
call :logStatus "Installation complete"
pause
exit


:cancelInstall
echo.
echo Installation log saved to:
echo %logfile%
echo.
echo ----------------------- Installation cancelled ------------------------
echo.
echo.
call :logStatus "Installation cancelled"
pause
exit


:wrongInput
echo Please enter [y]es or [n]o.
goto :eof


:readConfig
:: Custom count stores the number of custom paths for the module. Since
:  there is no build-in way to query the size of an array this number
:  gets defined when reading the config file.
set customCount=-1
if exist module.cfg (
    call :logStatus "Using module.cfg"
    for /f "tokens=* delims=" %%l in (module.cfg) do (
        set line=%%l
        :: Skip comment line.
        if /i "!line:~0,1!" neq "#" (
            :: Since there is no nice way of splitting lines the key
            :  in each line gets replaced with nothing so that only the
            :  value for the key remains.
            rem

            :: module path
            echo !line!|find "MODULE_DEFAULT_WIN" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_DEFAULT_WIN =!"
                if "!replace!" neq "" (
                    set modulePath=!replace!
                )
            )

            :: maya versions
            echo !line!|find "MAYA_VERSIONS" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MAYA_VERSIONS =!"
                if "!replace!" neq "" (
                    :: Versions are separated by a semicolon.
                    set "replace=!replace:;= !"
                    set mayaVersion=!replace!
                )
            )

            :: plug-in version
            echo !line!|find "MODULE_VERSION" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_VERSION =!"
                if "!replace!" neq "" (
                    set pluginVersion=!replace!
                )
            )

            :: icons path
            echo !line!|find "MODULE_ICONS" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_ICONS =!"
                if "!replace!" neq "" (
                    set iconPath=!replace!
                )
            )

            :: plug-ins path
            echo !line!|find "MODULE_PLUGINS" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_PLUGINS =!"
                if "!replace!" neq "" (
                    set pluginPath=!replace!
                )
            )

            :: scripts path
            echo !line!|find "MODULE_SCRIPTS" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_SCRIPTS =!"
                if "!replace!" neq "" (
                    set scriptPath=!replace!
                )
            )

            :: custom paths
            echo !line!|find "MODULE_CUSTOM_PATHS_WIN" >nul
            if not errorlevel 1 (
                set replace=!line!
                set "replace=!replace:MODULE_CUSTOM_PATHS_WIN =!"
                if "!replace!" neq "" (
                    :: A module can be defined with any number of custom
                    :  paths. For each occurrance in the config file
                    :  increase the count and store the path in a list.
                    set /a customCount+=1
                    set customPath[!customCount!]=!replace!
                )
            )
        )
    )
)
goto :eof


:writeModuleFile
set fileName=%1
set fileName=!fileName:§= !
call :logStatus "Begin writing module file : %fileName%"
:: Clear the module file if it exists.
break > "%fileName%"
set "pathString=!modulePath:$USER=%USERNAME%!\!name!"
set "pathString=!pathString: \=\!"
if !includePlugin! == 1 (
    for %%v in (!mayaVersion!) do (
        echo + MAYAVERSION:%%v !name! !pluginVersion! !pathString! >> "%fileName%"
        if "!iconPath!" neq "" (
            echo icons: !iconPath! >> "%fileName%"
        )
        if "!pluginPath!" neq "" (
            set replace=!pluginPath!
            set "replace=!replace:VERSION=%%v!"
            set "replace=!replace:PLATFORM=win64!"
            echo plug-ins: !replace! >> "%fileName%"
        )
        if "!scriptPath!" neq "" (
            echo scripts: !scriptPath! >> "%fileName%"
        )
        for /l %%n in (0,1,!customCount!) do (
            set replace=!customPath[%%n]!
            set "replace=!replace:VERSION=%%v!"
            echo !replace! >> "%fileName%"
        )
        echo. >> "%fileName%"
    )
) else (
    echo + !name! !pluginVersion! !pathString! >> "%fileName%"
    if "!iconPath!" neq "" (
        echo icons: !iconPath! >> "%fileName%"
    )
    if "!pluginPath!" neq "" (
        set replace=!pluginPath!
        set "replace=!replace:VERSION=%%v!"
        set "replace=!replace:PLATFORM=win64!"
        echo plug-ins: !replace! >> "%fileName%"
    )
    if "!scriptPath!" neq "" (
        echo scripts: !scriptPath! >> "%fileName%"
    )
    for /l %%n in (0,1,!customCount!) do (
        set replace=!customPath[%%n]!
        set "replace=!replace:VERSION=%%v!"
        echo !replace! >> "%fileName%"
    )
)
call :logStatus "Finished writing module file : %fileName%"
goto :eof


:getBackupPath
set fullPath=%1
set fullPath=%fullPath::=§%
set fullPath=%fullPath: =$%
set fullPath=%fullPath:\= %
set count=0
for %%i in (%fullPath%) do (
    set /a count+=1
)
set /a count-=1
set add=%count%
set /a add-=1
set backupPath=
set index=0
for %%i in (%fullPath%) do (
    if !index! lss %count% (
        set backupPath=!backupPath!%%i
    )
    if !index! lss %add% (
        set backupPath=!backupPath!\
    )
    set /a index+=1
)
set backupPath=!backupPath:$= !
set backupPath=!backupPath:§=:!
goto :eof


:getTimeStamp
set theDate=%DATE%
set theDate=%theDate:/=%
set theDate=%theDate:.=%
set theDate=%theDate::=%
set theDate=%theDate: =%
set theDate=%theDate:,=%
set theTime=%TIME%
set theTime=%theTime:/=%
set theTime=%theTime:.=%
set theTime=%theTime::=%
set theTime=%theTime: =%
set theTime=%theTime:,=%
set timestamp=%theDate%_%theTime%
goto :eof


:logStatus
echo %TIME% : %~1 >> "%logfile%"
goto :eof

ENDLOCAL

:: ---------------------------------------------------------------------
:  MIT License
:
:  Copyright (c) 2019 Ingo Clemens, brave rabbit
:  moduleInstaller under the terms of the MIT License
:
:  Permission is hereby granted, free of charge, to any person obtaining
:  a copy of this software and associated documentation files (the
:  "Software"), to deal in the Software without restriction, including
:  without limitation the rights to use, copy, modify, merge, publish,
:  distribute, sublicense, and/or sell copies of the Software, and to
:  permit persons to whom the Software is furnished to do so, subject to
:  the following conditions:
:
:  The above copyright notice and this permission notice shall be
:  included in all copies or substantial portions of the Software.
:
:  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
:  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
:  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
:  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
:  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
:  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
:  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
:
:  Author: Ingo Clemens    www.braverabbit.com
:  ---------------------------------------------------------------------
