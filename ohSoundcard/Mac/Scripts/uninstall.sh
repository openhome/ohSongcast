#!/bin/sh


# stop the agent application

launchFile="/Library/LaunchAgents/org.openhome.av.songcaster.plist"
if [ -e "$launchFile" ]
then
  launchctl unload "$launchFile"
fi


# unload the driver

driver="/System/Library/Extensions/ohSongcaster.kext"
if [ -e "$driver" ]
then
  kextunload "$driver"
  if [ $? != 0 ]
  then
    kextunload "$driver"
  fi
fi


# remove installed files

rm -rf "$launchFile"
rm -rf "$driver"
rm -rf /Library/PreferencePanes/ohSongcaster.prefPane
rm -rf /Library/OpenHome/ohSongcaster.app


