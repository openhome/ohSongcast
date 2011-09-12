#!/bin/sh


# stop the agent application

launchFile="/Library/LaunchAgents/org.openhome.ohSoundcard.plist"
if [ -e "$launchFile" ]
then
  launchctl unload "$launchFile"
fi


# unload the driver

driver="/System/Library/Extensions/ohSoundcard.kext"
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
rm -rf /Library/PreferencePanes/ohSoundcard.prefPane
rm -rf /Library/OpenHome/ohSoundcard.app


