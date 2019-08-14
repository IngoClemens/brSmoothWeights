#!/bin/bash

# ----------------------------------------------------------------------
#
# Installation script to install a module-based script or plug-in for
# Autodesk Maya.
#
# ----------------------------------------------------------------------

copyright="Copyright (c) 2019 Ingo Clemens, brave rabbit"
installerVersion=0.9.0-190814

# The name automatically gets defined throught the name of the module.
name=""

# ----------------------------------------------------------------------
# values overriden by config.txt (if present and active)
# ----------------------------------------------------------------------

declare -a mayaVersion=(2019)
pluginVersion=any
iconPath=""
pluginPath=""
scriptPath=""
declare -a customPath


# ----------------------------------------------------------------------
# system specific variables
# ----------------------------------------------------------------------

# linux
if [[ "$OSTYPE" == "linux-gnu" ]]
then
    modulePath=/home/$SUDO_USER/maya/modules
    osType=linux64
    configExt=LINUX
    declare -a factoryPath=("/usr/autodesk/mayaVERSION/modules" "/root/maya/VERSION/modules" "/root/maya/modules" "/usr/autodesk/modules/maya/VERSION" "/usr/autodesk/modules/maya" "/home/$SUDO_USER/maya/modules" "/home/$SUDO_USER/maya/VERSION/modules")
    installPathFile=/var/tmp/moduleInstaller
# macOS
elif [[ "$OSTYPE" == "darwin"* ]]
then
    modulePath=/Users/$USER/Library/Preferences/Autodesk/maya/modules
    osType=macOS
    configExt=MACOS
    declare -a factoryPath=("/Applications/Autodesk/mayaVERSION/Maya.app/Contents/modules" "/Users/$USER/Library/Preferences/Autodesk/maya/VERSION/modules" "/Users/$USER/Library/Preferences/Autodesk/maya/modules" "/Users/Shared/Autodesk/modules/maya/VERSION" "/Users/Shared/Autodesk/modules/maya")
    installPathFile=/var/tmp/moduleInstaller
fi

declare -a installPathList=(${modulePath})


# ----------------------------------------------------------------------
# general
# ----------------------------------------------------------------------

# text formatting
BOLD="$(tput bold)"
NORMAL="$(tput sgr0)"
UNDERLINE="$(tput smul)"
BLACK="$(tput setaf 0)"
RED="$(tput setaf 1)"
GREEN="$(tput setaf 2)"
BLUE="$(tput setaf 4)"
MAGENTA="$(tput setaf 5)"

# Stores the user input.
input=""
includePlugin=0
logfile=""

installOption=1

doDelete=0
deleteFile=""
deleteDir=""

doBackup=0
backupPath=""
backupFile=""
backupDir=""


# ----------------------------------------------------------------------
# helper functions
# ----------------------------------------------------------------------

logStatus()
{
    logtime=`date +%Y-%m-%d" "%H:%M:%S`
    echo -e "<$logtime> : $1" >> "$logfile"
}


finishInstall()
{
    echo
    echo "${BOLD}Installation log saved to:${NORMAL}"
    echo "${logfile}"
    echo
    echo "----------------------- ${BOLD}${GREEN}Installation complete${NORMAL} ------------------------"
    echo
    echo
    logStatus "Installation complete"

    if [[ "$OSTYPE" == "linux-gnu" ]]
    then
        read -n 1 -s -r -p "Press any key to exit"
    fi
    exit
}


cancelInstall()
{
    echo
    echo "${BOLD}Installation log saved to:${NORMAL}"
    echo "${logfile}"
    echo
    echo "----------------------- ${BOLD}${RED}Installation cancelled${NORMAL} ------------------------"
    echo
    echo
    logStatus "Installation cancelled"

    if [[ "$OSTYPE" == "linux-gnu" ]]
    then
        read -n 1 -s -r -p "Press any key to exit"
    fi
    exit
}


verifyInput()
{
    if [ "$1" == "y" -o "$1" == "Y" -o "$1" == "n" -o "$1" == "N" ]
    then
        echo 1
    fi
    echo 0
}


wrongInput()
{
    echo "${BOLD}Please enter [${RED}y${BLACK}]es or [${RED}n${BLACK}]o.${NORMAL}"
}


readConfig()
{
    if [ -e module.cfg ]
    then
        logStatus "Using module.cfg"
        while IFS='' read -r line || [[ -n "$line" ]]
        do
            # Skip comment line.
            if [[ ${line:0:1} != "#" ]]
            then
                # module path
                if [[ $line == *"MODULE_DEFAULT_$configExt"* ]]
                then
                    line=${line/MODULE_DEFAULT_$configExt /""}
                    if [[ $line != "" ]]
                    then
                        eval modulePath=${line}
                    fi

                # maya versions
                elif [[ $line == *"MAYA_VERSIONS"* ]]
                then
                    line=${line/MAYA_VERSIONS /""}
                    if [[ $line != "" ]]
                    then
                        IFS=';' read -r -a mayaVersion <<< "${line}"
                    fi

                # plug-in version
                elif [[ $line == *"MODULE_VERSION"* ]]
                then
                    line=${line/MODULE_VERSION /""}
                    if [[ $line != "" ]]
                    then
                        pluginVersion=${line}
                    fi

                # icons path
                elif [[ $line == *"MODULE_ICONS"* ]]
                then
                    line=${line/MODULE_ICONS /""}
                    if [[ $line != "" ]]
                    then
                        iconPath=${line}
                    fi

                # plug-ins path
                elif [[ $line == *"MODULE_PLUGINS"* ]]
                then
                    line=${line/MODULE_PLUGINS /""}
                    if [[ $line != "" ]]
                    then
                        pluginPath=${line}
                    fi

                # scripts path
                elif [[ $line == *"MODULE_SCRIPTS"* ]]
                then
                    line=${line/MODULE_SCRIPTS /""}
                    if [[ $line != "" ]]
                    then
                        scriptPath=${line}
                    fi

                # custom paths
                elif [[ $line == *"MODULE_CUSTOM_PATHS_$configExt"* ]]
                then
                    line=${line/MODULE_CUSTOM_PATHS_$configExt /""}
                    customPath=("${customPath[@]}" "${line}")
                    # Alternative version for adding to the array:
                    #customPath[${#customPath[@]}]="${line}"
                fi
            fi
        done < module.cfg
    fi
}


writeModuleFile()
{
    logStatus "Begin writing module file : $1"
    lines=""
    if [ "$includePlugin" == 1 ]
    then
        for version in "${mayaVersion[@]}"
        do
            lines+="+ MAYAVERSION:${version} ${name} ${pluginVersion} ${modulePath}/${name}\n"
            if [[ $iconPath != "" ]]
            then
                lines+="icons: ${iconPath}\n"
            fi
            if [[ $pluginPath != "" ]]
            then
                writePath=${pluginPath/VERSION/$version}
                writePath=${writePath/PLATFORM/$osType}
                lines+="plug-ins: ${writePath}\n"
            fi
            if [[ $scriptPath != "" ]]
            then
                lines+="scripts: ${scriptPath}\n"
            fi
            for path in "${customPath[@]}"
            do
                writePath=${path/VERSION/$version}
                lines+="${writePath}\n"
            done
            lines+="\n"
        done
    else
        lines="+ ${name} ${pluginVersion} ${modulePath}/${name}\n"
        if [[ $iconPath != "" ]]
        then
            lines+="icons: ${iconPath}\n"
        fi
        if [[ $pluginPath != "" ]]
        then
            writePath=${pluginPath/VERSION/$version}
            writePath=${writePath/PLATFORM/$osType}
            lines+="plug-ins: ${writePath}\n"
        fi
        if [[ $scriptPath != "" ]]
        then
            lines+="scripts: ${scriptPath}\n"
        fi
        for path in "${customPath[@]}"
        do
            lines+="${path}\n"
        done
    fi

    echo -e "$lines" > "$1"

    logStatus "Finished writing module file : $1"
}


# ----------------------------------------------------------------------
# begin
# ----------------------------------------------------------------------

clear

# Switch to the current directory and get it's path.
cd -- "$(dirname "$0")"
currentDir=$(pwd)

# Setup the log file.
logfile=$currentDir/install.log
echo > "$logfile"
logStatus "Begin installation"

# ----------------------------------------------------------------------
# module configuration
# ----------------------------------------------------------------------

# Read any custom settings from the module.cfg file.
readConfig

for version in "${mayaVersion[@]}"
do
    logStatus "Maya version : $version"
done
logStatus "Module version : $pluginVersion"
logStatus "Module icons : $iconPath"
logStatus "Module plug-ins : $pluginPath"
logStatus "Module scripts : $scriptPath"
for path in "${customPath[@]}"
do
    logStatus "Module custom path : $path"
done

# Get the global MAYA_APP_DIR environment variable.
# If this is not defined use the default modules path.
if [[ ! -z "${MAYA_APP_DIR}" ]]
then
    modulePath=${MAYA_APP_DIR}/modules
    # Add the MAYA_APP_DIR path to the factory paths when searching for
    # existing modules.
    factoryPath=("${factoryPath[@]}" "${modulePath}")
fi

# Check if the file exists which contains all previously used custom
# install paths. Add all previous paths to the factory path array to
# search for previous installations.
# Also, create a list of all custom install paths to be able to give
# the user an option to choose from previously used paths during the
# custom installation.
installPathFileExists=0
if [ -e $installPathFile ]
then
    while read line
    do
        factoryPath=("${factoryPath[@]}" "${line}")
        installPathList=("${installPathList[@]}" "${line}")
    done < $installPathFile
    installPathFileExists=1
fi

logStatus "MAYA_APP_DIR : $MAYA_APP_DIR"
logStatus "Module path : $modulePath"

# ----------------------------------------------------------------------
# header
# ----------------------------------------------------------------------

echo "${BOLD}| ---------------------------------------------------------------------${NORMAL}"
echo "${BOLD}| Module installer for Autodesk Maya.${NORMAL}"
echo "${BOLD}| ${NORMAL}"
echo "${BOLD}| Version: $installerVersion${NORMAL}"
echo "${BOLD}| moduleInstaller is under the terms of the MIT License${NORMAL}"
echo "${BOLD}| $copyright${NORMAL}"
echo "${BOLD}| ---------------------------------------------------------------------${NORMAL}"
echo

# ----------------------------------------------------------------------
# checks
# ----------------------------------------------------------------------

# Check if the source files for the installation are present.
# The installer must be located in the same folder as the modules folder
# which has to contain the folder with the name of the module.
sourceDir="$currentDir/modules"
if [ -e "$sourceDir" ]
then
    # The modules folder has to contain one folder with the name of the
    # module.
    count=0
    for entry in "$sourceDir/"*
    do
        name=$(basename "$entry")
        count=$((count+1))
        logStatus "Source folder content : $name"
    done
    if [ $count != 1 ]
    then
        echo "${BOLD}${RED}Error:${BLACK} The modules folder must only contain one folder with the name of the module.${NORMAL}"
        logStatus "Error: The modules folder must only contain one folder with the name of the module."

        cancelInstall
    fi
else
    echo
    echo "${BOLD}${RED}Error:${BLACK} The source files for the installation cannot be found.${NORMAL}"
    echo "${BOLD}The module files must be located in:${NORMAL}"
    echo $sourceDir
    logStatus "Error: The source files must be located in: $sourceDir"

    cancelInstall
fi

logStatus "Module Name : $name"

moduleBase="$sourceDir/$name"
logStatus "Module base path : $moduleBase"

# Check if the module contains a plug-in folder.
if [ -e "$moduleBase/plug-ins" ]
then
    includePlugin=1
fi
logStatus "Module has plug-in : $includePlugin"

# ----------------------------------------------------------------------
# intro
# ----------------------------------------------------------------------

# Inform about the installation and ask to continue.
# Display the license, if any and cancel the installation if the license
# doesn't get accepted.

validEntry=0
echo "${BOLD}This program installs $name for Autodesk Maya.${NORMAL}"
echo
while [[ $validEntry == 0 ]]
do
    read -p "${BOLD}Do you want to continue? [y/n]: ${NORMAL}" input
    validEntry=$(verifyInput $input)
    if [[ $validEntry == 0 ]]
    then
        wrongInput
    fi
done

if [ "${input}" == "y" -o "${input}" == "Y" ]
then
    # ------------------------------------------------------------------
    # display license
    # ------------------------------------------------------------------
    if [ -e LICENSE ]
    then
        logStatus "Displaying license"
        echo
        echo

        licenseContent=$(<LICENSE)
        echo "$licenseContent"

        echo
        echo

        validEntry=0
        while [[ $validEntry == 0 ]]
        do
            read -p "${BOLD}Do you agree with the terms of the license? [y/n]: ${NORMAL}" input
            validEntry=$(verifyInput $input)
            if [[ $validEntry == 0 ]]
            then
                wrongInput
            fi
        done

        if [ "${input}" == "n" -o "${input}" == "N" ]
        then
            cancelInstall
        fi

        echo
    fi
else
    cancelInstall
fi

# ----------------------------------------------------------------------
# install option
# ----------------------------------------------------------------------

# The path the module file is saved to.
moduleFilePath=""

echo
echo "${BOLD}Installation type:${NORMAL}"
echo "${BOLD}    1. Simple${NORMAL}"
echo "${BOLD}    2. Custom (Additional options)${NORMAL}"
echo "${BOLD}    3. Create module file only (Manual setup)${NORMAL}"
echo "${BOLD}    4. Uninstall${NORMAL}"
echo "${BOLD}    5. Exit${NORMAL}"
echo
read -p "${BOLD}Please enter choice [1-5]: ${NORMAL}" input
if [ "${input}" == 1 ]
then
    installOption=1
elif [ "${input}" == 2 ]
then
    installOption=2
elif [ "${input}" == 3 ]
then
    moduleFilePath=$currentDir/$name.mod
    writeModuleFile "$moduleFilePath"
    echo
    echo "${BOLD}Module file saved to:${NORMAL}"
    echo $moduleFilePath
    finishInstall
elif [ "${input}" == 4 ]
then
    installOption=4
elif [ "${input}" == 5 ]
then
    cancelInstall
else
    cancelInstall
fi
logStatus "Install option : $installOption"

# ----------------------------------------------------------------------
# cleanup existing modules
# ----------------------------------------------------------------------

if [ "${installOption}" == 2 -o "${installOption}" == 4 ]
then
    echo
    echo "${BOLD}${BLUE}Checking for existing modules...${NORMAL}"
    logStatus "Checking for existing modules"
fi

visitedPaths=""

# Go through each Maya version and default module paths and check for
# existing modules.
for version in "${mayaVersion[@]}"
do
    for path in "${factoryPath[@]}"
    do
        # Replace the version placeholder.
        modPath="${path/VERSION/$version}"

        logStatus "Searching default path : $modPath"

        # Check if the current path already has been processed in case
        # of non version specific paths.
        processed=0
        IFS=";"
        read -ra visitedElements <<< "$visitedPaths"
        for item in "${visitedElements[@]}"
        do
            if [ "$item" == "$modPath" ]
            then
                processed=1
            fi
        done
        unset IFS

        # Check if the module exists in one of the default locations.
        modFile=${modPath}/$name.mod
        if [ -e "$modFile" -a $processed -eq 0 ]
        then
            logStatus "Found existing module file : $modFile"
            # Check where the module file points to.
            # Get the content of the .mod file and parse the first line.
            # The last item is the location of the module folder.
            while read line
            do
                content=$line
                break
            done < "$modFile"
            items=($content)
            logStatus "First line in module : $line"

            # Simple modules without a plugin only need a module
            # description built from four elements:
            # + name version path
            # When plugins are included these are usually version
            # specific so the Maya version needs to be defined as well:
            # + mayaVersion name version path
            itemNum=4
            if [[ $includePlugin == 1 ]]
            then
                itemNum=5
            elif [[ ${items[1]:0:11} == "MAYAVERSION" ]]
            then
                itemNum=5
            fi
            logStatus "Required line items : $itemNum"

            modDir=${items[${itemNum}-1]}

            # Some folder names can contain spaces. If the line is split
            # by spaces the path to the module gets split up as well.
            # The path items then need to get joined.
            numItems="${#items[@]}"
            if [[ ${numItems} > $itemNum ]]
            then
                count=$itemNum
                while [[ $count < $numItems ]]
                do
                     modDir+=" "${items[${count}]}
                     count=$((count+1))
                done
            elif [[ ${numItems} < $itemNum ]]
            then
                echo
                echo "${BOLD}${RED}Error:${BLACK} The following module file doesn't seem to be properly formatted.${NORMAL}"
                echo "${BOLD}The module description needs to have ${itemNum} space-separated items.${NORMAL}"
                echo $modFile
                logStatus "Error: Wrong formatting in module file."

                cancelInstall
            fi
            logStatus "Existing module content : $modDir"

            # Only display existing modules and files when the custom
            # install option has been selected.
            if [ "${installOption}" == 2 -o "${installOption}" == 4 ]
            then
                echo
                echo "${BOLD}Found existing module:${NORMAL}"
                echo "${BOLD}Module path:${NORMAL}"
                echo $modFile
                echo "${BOLD}Module content:${NORMAL}"
                echo $modDir
                echo

                validEntry=0
                while [[ $validEntry == 0 ]]
                do
                    read -p "${BOLD}Do you want to delete the files? [y/n]: ${NORMAL}" input
                    validEntry=$(verifyInput $input)
                    if [[ $validEntry == 0 ]]
                    then
                        wrongInput
                    fi
                done
            else
                input=y
            fi

            if [ "${input}" == "y" -o "${input}" == "Y" ]
            then
                # ------------------------------------------------------
                # Delete old files
                # ------------------------------------------------------
                doDelete=1
                deleteFile=$modFile
                deleteDir=$modDir
                logStatus "Marked for deletion : $modFile"
                logStatus "Marked for deletion : $modDir"
            else
                # ------------------------------------------------------
                # Backup old files
                # ------------------------------------------------------
                echo

                validEntry=0
                while [[ $validEntry == 0 ]]
                do
                    read -p "${BOLD}Do you want to backup the files? [y/n]: ${NORMAL}" input
                    validEntry=$(verifyInput $input)
                    if [[ $validEntry == 0 ]]
                    then
                        wrongInput
                    fi
                done

                if [ "${input}" == "y" -o "${input}" == "Y" ]
                then
                    doBackup=1
                    timestamp=`date +%Y%m%d_%H%M%S`
                    backupPath="$(dirname "$modDir")"
                    backupPath+=/${name}_$timestamp
                    backupFile=$modFile
                    backupDir=$modDir
                    logStatus "Marked for backup : $modFile"
                    logStatus "Marked for backup : $modDir"
                    logStatus "Backup folder : $backupPath"
                else
                    echo
                    echo "${BOLD}${RED}Error:${BLACK} The previous files must be removed to continue with the installation.${NORMAL}"
                    logStatus "Error: The previous files must be removed to continue with the installation."
                    cancelInstall
                fi
            fi

            visitedPaths+="$modPath;"
        fi
    done
done

echo

if [ "${installOption}" == 2 -o "${installOption}" == 4 ]
then
    echo "${BOLD}${BLUE}... Check completed.${NORMAL}"
    echo
    logStatus "Check completed"
fi

# ----------------------------------------------------------------------
# setup the module content path
# ----------------------------------------------------------------------

# If the custom install option has been chosen check if previous user
# defined paths have been saved. In this case use the first stored path
# as the default path for the installation.
if [ $installPathFileExists == 1 -a "${#installPathList[@]}" -gt 0 -a "${installOption}" == 2 ]
then
    modulePath="${installPathList[0]}"
    logStatus "Module path using stored custom path : $modulePath"
fi

if [ ! "${installOption}" == 4 ]
then
    echo "${BOLD}The module will be installed in:${NORMAL}"
    echo $modulePath
fi

moduleFilePath=$modulePath

if [ "${installOption}" == 1 ]
then
    if [ "${doDelete}" == 1 ]
    then
        echo
        echo "${BOLD}A previous installation has been detected and will be deleted.${NORMAL}"
    fi

    echo

    validEntry=0
    while [[ $validEntry == 0 ]]
    do
        read -p "${BOLD}Do you want to continue? [y/n]: ${NORMAL}" input
        validEntry=$(verifyInput $input)
        if [[ $validEntry == 0 ]]
        then
            wrongInput
        fi
    done

    if [ "${input}" == "n" -o "${input}" == "N" ]
    then
        cancelInstall
    fi

# Prompt for the installation path.
elif [ "${installOption}" == 2 ]
then
    echo

    validEntry=0
    while [[ $validEntry == 0 ]]
    do
        read -p "${BOLD}Do you want to use this location? [y/n]: ${NORMAL}" input
        validEntry=$(verifyInput $input)
        if [[ $validEntry == 0 ]]
        then
            wrongInput
        fi
    done

    if [ "${input}" == "n" -o "${input}" == "N" ]
    then
        needsPathInput=1

        # In case the file exists which contains the previous install
        # paths display these as a list the user can choose from.
        if [ $installPathFileExists == 1 ]
        then
            echo
            echo "${BOLD}Previous install paths:${NORMAL}"
            count=1
            for path in "${installPathList[@]}"
            do
                echo "${BOLD}    $count. ${path}${NORMAL}"
                count=$((count+1))
            done
            echo "${BOLD}    $count. New path${NORMAL}"
            echo
            read -p "${BOLD}Please enter choice [1-$count]: ${NORMAL}" input

            if [ $input -lt $count ]
            then
                index=$input-1
                modulePath=${installPathList[$index]}
                logStatus "User defined install path : ${installPathList[$index]}"
                needsPathInput=0
            elif [ $input == $count ]
            then
                needsPathInput=1
            else
                cancelInstall
            fi
        fi

        if [ $needsPathInput == 1 ]
        then
            echo
            read -p "${BOLD}Please enter a custom install path: ${NORMAL}" input
            userDefinedPath=$input
            if [ "${input}" != "" ]
            then
                logStatus "User defined install path : $userDefinedPath"
                # Check if the entered path is valid
                if [ ! -d $input ]
                then
                    echo
                    echo "${BOLD}The path does not exist.${NORMAL}"
                    echo

                    validEntry=0
                    while [[ $validEntry == 0 ]]
                    do
                        read -p "${BOLD}Do you want to create it? [y/n]: ${NORMAL}" input
                        validEntry=$(verifyInput $input)
                        if [[ $validEntry == 0 ]]
                        then
                            wrongInput
                        fi
                    done

                    if [ "${input}" == "y" -o "${input}" == "Y" ]
                    then
                        mkdir -p $userDefinedPath
                        logStatus "Created folder : $userDefinedPath"
                    else
                        echo
                        echo "${BOLD}${RED}Error:${BLACK} No valid path provided for the installation.${NORMAL}"
                        logStatus "Error: No valid path provided for the installation."
                        cancelInstall
                    fi
                fi
                modulePath=$userDefinedPath

                # Add the new path the custom path file for later reference.
                if [ $installPathFileExists == 1 ]
                then
                    echo -e "$userDefinedPath" >> "$installPathFile"
                else
                    echo -e "$userDefinedPath" > "$installPathFile"
                fi
                logStatus "Added user defined path $userDefinedPath to $installPathFile"
            else
                echo
                echo "${BOLD}${RED}Error:${BLACK} No path provided for the installation.${NORMAL}"
                logStatus "Error: No path provided for the installation."
                cancelInstall
            fi
        fi
    fi
fi

# ----------------------------------------------------------------------
# setup module file path
# ----------------------------------------------------------------------

if [ ! "${installOption}" == 4 ]
then
    echo
    echo "${BOLD}The module file $name.mod will be created in:${NORMAL}"
    echo $moduleFilePath
fi

# Prompt for the installation path.
if [ "${installOption}" == 2 ]
then
    echo

    validEntry=0
    while [[ $validEntry == 0 ]]
    do
        read -p "${BOLD}Do you want to use this location? [y/n]: ${NORMAL}" input
        validEntry=$(verifyInput $input)
        if [[ $validEntry == 0 ]]
        then
            wrongInput
        fi
    done

    if [ "${input}" == "n" -o "${input}" == "N" ]
    then
        needsPathInput=1

        # In case the file exists which contains the previous install
        # paths display these as a list the user can choose from.
        if [ $installPathFileExists == 1 ]
        then
            echo
            echo "${BOLD}Previous install paths:${NORMAL}"
            count=1
            for path in "${installPathList[@]}"
            do
                echo "${BOLD}    $count. ${path}${NORMAL}"
                count=$((count+1))
            done
            echo "${BOLD}    $count. New path${NORMAL}"
            echo
            read -p "${BOLD}Please enter choice [1-$count]: ${NORMAL}" input

            if [ $input -lt $count ]
            then
                index=$input-1
                moduleFilePath=${installPathList[$index]}
                logStatus "User defined module file path : ${installPathList[$index]}"
                needsPathInput=0
            elif [ $input == $count ]
            then
                needsPathInput=1
            else
                cancelInstall
            fi
        fi

        if [ $needsPathInput == 1 ]
        then
            echo
            read -p "${BOLD}Please enter a custom path for the module file: ${NORMAL}" input
            userDefinedModuleFilePath=$input
            if [ "${input}" != "" ]
            then
                logStatus "User defined module file path : $userDefinedModuleFilePath"
                # Check if the entered path is valid
                if [ ! -d $input ]
                then
                    echo
                    echo "${BOLD}The path does not exist.${NORMAL}"
                    echo

                    validEntry=0
                    while [[ $validEntry == 0 ]]
                    do
                        read -p "${BOLD}Do you want to create it? [y/n]: ${NORMAL}" input
                        validEntry=$(verifyInput $input)
                        if [[ $validEntry == 0 ]]
                        then
                            wrongInput
                        fi
                    done

                    if [ "${input}" == "y" -o "${input}" == "Y" ]
                    then
                        mkdir -p $userDefinedModuleFilePath
                        logStatus "Created folder : $userDefinedModuleFilePath"
                    else
                        echo
                        echo "${BOLD}${RED}Error:${BLACK} No valid module file path provided for the installation.${NORMAL}"
                        logStatus "Error: No valid module file path provided for the installation."
                        cancelInstall
                    fi
                fi
                moduleFilePath=$userDefinedModuleFilePath

                if [ $modulePath != $moduleFilePath ]
                then
                    # Add the new path the custom path file for later reference.
                    if [ $installPathFileExists == 1 ]
                    then
                        echo -e "$userDefinedModuleFilePath" >> "$installPathFile"
                    else
                        echo -e "$userDefinedModuleFilePath" > "$installPathFile"
                    fi
                    logStatus "Added user defined path $userDefinedModuleFilePath to $installPathFile"
                fi
            else
                echo
                echo "${BOLD}${RED}Error:${BLACK} No module file path provided for the installation.${NORMAL}"
                logStatus "Error: No module file path provided for the installation."
                cancelInstall
            fi
        fi
    fi
fi

# ----------------------------------------------------------------------
# Delete old files
# ----------------------------------------------------------------------

if [ "${doDelete}" == 1 ]
then
    rm "$deleteFile"
    rm -rf "$deleteDir"

    echo
    echo "${BOLD}Deleted previous module:${NORMAL}"
    echo $deleteFile
    echo "${BOLD}Deleted previous module content:${NORMAL}"
    echo $deleteDir
    logStatus "Deleted previous module : $deleteFile"
    logStatus "Deleted previous module content : $deleteDir"

# ----------------------------------------------------------------------
# Backup old files
# ----------------------------------------------------------------------

else
    if [ "${doBackup}" == 1 ]
    then
        mkdir -p "$backupPath"
        chmod -R 755 "$backupPath"
        mv "$backupDir" "$backupPath"
        mv "$backupFile" "$backupPath"

        echo
        echo "${BOLD}Moved old files to:${NORMAL}"
        echo $backupPath
        logStatus "Moved old files to : $backupPath"
    fi
fi

if [ "${installOption}" == 4 ]
then
    finishInstall
fi

# ----------------------------------------------------------------------
# Create the module
# ----------------------------------------------------------------------

mkdir -p "$moduleFilePath/"
logStatus "Using module path : $moduleFilePath"

echo
echo "${BOLD}${BLUE}Writing module file...${NORMAL}"

writeModuleFile "$moduleFilePath/$name.mod"

echo "${BOLD}${BLUE}... Done${NORMAL}"

echo
echo "${BOLD}${BLUE}Copying files...${NORMAL}"

cp -r "$moduleBase" "$modulePath"

echo "${BOLD}${BLUE}... Done${NORMAL}"

# Fix any folder and file permissions.
chmod -R 755 "$moduleFilePath/$name.mod"
chmod -R 755 "$modulePath/$name"

echo
echo "${BOLD}Installed module to:${NORMAL}"
echo $modulePath/$name
logStatus "Installed module to : $modulePath/$name"


finishInstall

#: <<'END'

# ----------------------------------------------------------------------
# MIT License
#
# Copyright (c) 2019 Ingo Clemens, brave rabbit
# moduleInstaller under the terms of the MIT License
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Author: Ingo Clemens    www.braverabbit.com
# ----------------------------------------------------------------------
