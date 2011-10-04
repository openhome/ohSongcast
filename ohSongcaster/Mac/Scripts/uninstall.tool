#!/bin/sh


launchFile="/Library/LaunchAgents/org.openhome.av.songcaster.plist"
driver="/System/Library/Extensions/ohSongcaster.kext"
prefs="/Library/PreferencePanes/ohSongcaster.prefPane"
app="/Library/OpenHome/ohSongcaster.app"


# stop the agent application

echo
echo Stopping application...
echo

if [ -e "$launchFile" ]
then
  launchctl unload "$launchFile" > /dev/null 2>&1
fi


# unload the driver

echo
echo Unloading driver...
echo

if [ -e "$driver" ]
then
  sudo kextunload "$driver" > /dev/null 2>&1
  if [ $? != 0 ]
  then
    sudo kextunload "$driver" > /dev/null 2>&1
  fi
fi


# remove installed files

echo
echo Removing files...
echo

sudo rm -rf "$launchFile"
sudo rm -rf "$driver"
sudo rm -rf "$prefs"
sudo rm -rf "$app"


echo
echo -------------------------------------------------------
echo Please restart you machine to complete the installation
echo -------------------------------------------------------
echo


