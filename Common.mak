
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

objects_netmon   = $(ohnetmondir)NetworkMonitor.$(objext) \
                   $(ohnetdir)DvAvOpenhomeOrgNetworkMonitor1.$(objext)

headers_songcast = Ohm.h \
                   OhmSender.h

$(objdir)Ohm.$(objext) : Ohm.cpp $(headers_songcast)
	$(compiler)Ohm.$(objext) -c $(cflags) $(includes) Ohm.cpp
$(objdir)OhmSender.$(objext) : OhmSender.cpp $(headers_songcast)
	$(compiler)OhmSender.$(objext) -c $(cflags) $(includes) OhmSender.cpp


objects_recvrmgrs = $(objdir)ReceiverManager1.$(objext) \
                    $(objdir)ReceiverManager2.$(objext) \
                    $(objdir)ReceiverManager3.$(objext)

objects_Songcast = $(objdir)ReceiverManager1.$(objext) \
                    $(objdir)ReceiverManager2.$(objext) \
                    $(objdir)ReceiverManager3.$(objext) \
                    $(objdir)Songcast.$(objext)

headers_Songcast = ohSongcast$(dirsep)ReceiverManager1.h \
                    ohSongcast$(dirsep)ReceiverManager2.h \
                    ohSongcast$(dirsep)ReceiverManager3.h \
                    ohSongcast$(dirsep)Songcast.h

$(objdir)ReceiverManager1.$(objext) : ohSongcast$(dirsep)ReceiverManager1.cpp $(headers_Songcast)
	$(compiler)ReceiverManager1.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)ReceiverManager1.cpp
$(objdir)ReceiverManager2.$(objext) : ohSongcast$(dirsep)ReceiverManager2.cpp $(headers_Songcast)
	$(compiler)ReceiverManager2.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)ReceiverManager2.cpp
$(objdir)ReceiverManager3.$(objext) : ohSongcast$(dirsep)ReceiverManager3.cpp $(headers_Songcast)
	$(compiler)ReceiverManager3.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)ReceiverManager3.cpp
$(objdir)Songcast.$(objext) : ohSongcast$(dirsep)Songcast.cpp $(headers_Songcast)
	$(compiler)Songcast.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)Songcast.cpp


all_common : TestReceiverManager1 TestReceiverManager2 TestReceiverManager3 ZoneWatcher WavSender


TestReceiverManager1 : $(objdir)TestReceiverManager1.$(exeext)
$(objdir)TestReceiverManager1.$(exeext) : ohSongcast$(dirsep)TestReceiverManager1.cpp $(objects_songcast) $(objects_Songcast)
	$(compiler)TestReceiverManager1.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)TestReceiverManager1.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager1.$(exeext) $(objdir)TestReceiverManager1.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


TestReceiverManager2 : $(objdir)TestReceiverManager2.$(exeext)
$(objdir)TestReceiverManager2.$(exeext) : ohSongcast$(dirsep)TestReceiverManager2.cpp $(objects_songcast) $(objects_Songcast)
	$(compiler)TestReceiverManager2.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)TestReceiverManager2.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager2.$(exeext) $(objdir)TestReceiverManager2.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


TestReceiverManager3 : $(objdir)TestReceiverManager3.$(exeext)
$(objdir)TestReceiverManager3.$(exeext) : ohSongcast$(dirsep)TestReceiverManager3.cpp $(objects_songcast) $(objects_Songcast)
	$(compiler)TestReceiverManager3.$(objext) -c $(cflags) $(includes) ohSongcast$(dirsep)TestReceiverManager3.cpp
	$(link) $(linkoutput)$(objdir)TestReceiverManager3.$(exeext) $(objdir)TestReceiverManager3.$(objext) $(objects_recvrmgrs) $(objects_songcast) $(objects_topology) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


ZoneWatcher : $(objdir)ZoneWatcher.$(exeext)
$(objdir)ZoneWatcher.$(exeext) : ZoneWatcher$(dirsep)ZoneWatcher.cpp $(objects_songcast)
	$(compiler)ZoneWatcher.$(objext) -c $(cflags) $(includes) ZoneWatcher$(dirsep)ZoneWatcher.cpp
	$(link) $(linkoutput)$(objdir)ZoneWatcher.$(exeext) $(objdir)ZoneWatcher.$(objext) $(objects_songcast) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


WavSender : $(objdir)WavSender.$(exeext)
$(objdir)WavSender.$(exeext) : WavSender$(dirsep)WavSender.cpp $(objects_songcast)
	$(compiler)WavSender.$(objext) -c $(cflags) $(includes) WavSender$(dirsep)WavSender.cpp
	$(link) $(linkoutput)$(objdir)WavSender.$(exeext) $(objdir)WavSender.$(objext) $(objects_songcast) $(ohnetdir)$(libprefix)ohNetCore.$(libext) $(ohnetdir)$(libprefix)TestFramework.$(libext)


