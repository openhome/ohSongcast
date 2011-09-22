
objects_topology = $(ohnetdir)CpTopology.$(objext) \
                   $(ohnetdir)CpTopology1.$(objext) \
                   $(ohnetdir)CpTopology2.$(objext) \
                   $(ohnetdir)CpTopology3.$(objext) \
                   $(ohnetdir)CpAvOpenhomeOrgProduct1.$(objext) \
                   $(ohnetdir)CpAvOpenhomeOrgVolume1.$(objext)


objects_songcast = $(objdir)Ohm.$(objext) \
                   $(objdir)OhmSender.$(objext) \
                   $(ohnetdir)CpAvOpenhomeOrgReceiver1.$(objext) \
                   $(ohnetdir)DvAvOpenhomeOrgSender1.$(objext)

headers_songcast = Ohm.h \
                   OhmSender.h

$(objdir)Ohm.$(objext) : Ohm.cpp $(headers_songcast)
	$(compiler)Ohm.$(objext) -c $(cflags) $(includes) Ohm.cpp
$(objdir)OhmSender.$(objext) : OhmSender.cpp $(headers_songcast)
	$(compiler)OhmSender.$(objext) -c $(cflags) $(includes) OhmSender.cpp


objects_recvrmgrs = $(objdir)ReceiverManager1.$(objext) \
                    $(objdir)ReceiverManager2.$(objext) \
                    $(objdir)ReceiverManager3.$(objext)

objects_songcaster = $(objdir)ReceiverManager1.$(objext) \
                    $(objdir)ReceiverManager2.$(objext) \
                    $(objdir)ReceiverManager3.$(objext) \
                    $(objdir)Songcaster.$(objext)

headers_songcaster = ohSongcaster$(dirsep)ReceiverManager1.h \
                    ohSongcaster$(dirsep)ReceiverManager2.h \
                    ohSongcaster$(dirsep)ReceiverManager3.h \
                    ohSongcaster$(dirsep)Songcaster.h

$(objdir)ReceiverManager1.$(objext) : ohSongcaster$(dirsep)ReceiverManager1.cpp $(headers_songcaster)
	$(compiler)ReceiverManager1.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)ReceiverManager1.cpp
$(objdir)ReceiverManager2.$(objext) : ohSongcaster$(dirsep)ReceiverManager2.cpp $(headers_songcaster)
	$(compiler)ReceiverManager2.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)ReceiverManager2.cpp
$(objdir)ReceiverManager3.$(objext) : ohSongcaster$(dirsep)ReceiverManager3.cpp $(headers_songcaster)
	$(compiler)ReceiverManager3.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)ReceiverManager3.cpp
$(objdir)Songcaster.$(objext) : ohSongcaster$(dirsep)Songcaster.cpp $(headers_songcaster)
	$(compiler)Songcaster.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)Songcaster.cpp


all_common : TestReceiverManager1 TestReceiverManager2 TestReceiverManager3 ZoneWatcher WavSender


TestReceiverManager1 : $(objdir)TestReceiverManager1.$(exeext)
$(objdir)TestReceiverManager1.$(exeext) : ohSongcaster$(dirsep)TestReceiverManager1.cpp $(objects_songcast) $(objects_songcaster)
	$(compiler)TestReceiverManager1.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)TestReceiverManager1.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager1.$(exeext) $(objdir)TestReceiverManager1.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


TestReceiverManager2 : $(objdir)TestReceiverManager2.$(exeext)
$(objdir)TestReceiverManager2.$(exeext) : ohSongcaster$(dirsep)TestReceiverManager2.cpp $(objects_songcast) $(objects_songcaster)
	$(compiler)TestReceiverManager2.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)TestReceiverManager2.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager2.$(exeext) $(objdir)TestReceiverManager2.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


TestReceiverManager3 : $(objdir)TestReceiverManager3.$(exeext)
$(objdir)TestReceiverManager3.$(exeext) : ohSongcaster$(dirsep)TestReceiverManager3.cpp $(objects_songcast) $(objects_songcaster)
	$(compiler)TestReceiverManager3.$(objext) -c $(cflags) $(includes) ohSongcaster$(dirsep)TestReceiverManager3.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager3.$(exeext) $(objdir)TestReceiverManager3.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


ZoneWatcher : $(objdir)ZoneWatcher.$(exeext)
$(objdir)ZoneWatcher.$(exeext) : ZoneWatcher$(dirsep)ZoneWatcher.cpp $(objects_songcast)
	$(compiler)ZoneWatcher.$(objext) -c $(cflags) $(includes) ZoneWatcher$(dirsep)ZoneWatcher.cpp
	$(link) $(linkoutput)$(objdir)ZoneWatcher.$(exeext) $(objdir)ZoneWatcher.$(objext) $(objects_songcast) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


WavSender : $(objdir)WavSender.$(exeext)
$(objdir)WavSender.$(exeext) : WavSender$(dirsep)WavSender.cpp $(objects_songcast)
	$(compiler)WavSender.$(objext) -c $(cflags) $(includes) WavSender$(dirsep)WavSender.cpp
	$(link) $(linkoutput)$(objdir)WavSender.$(exeext) $(objdir)WavSender.$(objext) $(objects_songcast) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


