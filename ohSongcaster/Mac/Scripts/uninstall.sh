#!/bin/sh


launchFile="/Library/LaunchAgents/org.openhome.av.songcaster.plist"
driver="/System/Library/Extensions/ohSongcaster.kext"
prefs="/Library/PreferencePanes/ohSongcaster.prefPane"
app="/Library/OpenHome/ohSongcaster.app"


# stop the agent application

if [ -e "$launchFile" ]
then
  launchctl unload "$launchFile"
fi


# unload the driver

if [ -e "$driver" ]
then
  sudo kextunload "$driver"
  if [ $? != 0 ]
  then
    sudo kextunload "$driver"
  fi
fi


# remove installed files

sudo rm -rf "$launchFile"
sudo rm -rf "$driver"
sudo rm -rf "$prefs"
sudo rm -rf "$app"


