#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Private/TestFramework.h>

#include <vector>
#include <stdio.h>

#include "ReceiverManager2.h"


#ifdef _WIN32
#define CDECL __cdecl
#else
#define CDECL 
#endif


namespace OpenHome {
namespace Av {

	class ReceiverManager2Logger : IReceiverManager2Handler
	{
	public:
		ReceiverManager2Logger(Net::CpStack& aCpStack);
		virtual void ReceiverAdded(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverChanged(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverRemoved(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver);
		virtual void ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver);
		~ReceiverManager2Logger();
	private:
		ReceiverManager2* iReceiverManager;
	};

} // namespace Av
} // namespace OpenHome

using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::TestFramework;
using namespace OpenHome::Av;

ReceiverManager2Logger::ReceiverManager2Logger(Net::CpStack& aCpStack)
{
	iReceiverManager = new ReceiverManager2(aCpStack, *this);
}

ReceiverManager2Logger::~ReceiverManager2Logger()
{
	delete (iReceiverManager);
}

void ReceiverManager2Logger::ReceiverAdded(ReceiverManager2Receiver& aReceiver)
{
    Print("Added   ");
    Print(aReceiver.Room());
    Print("(");
    Print(aReceiver.Group());
    Print(")");
	if (aReceiver.Selected()) {
		Print(" Selected ");
	}
	else {
		Print(" Not Selected ");
	}
	Bws<10000> state;
	aReceiver.TransportState(state);
	Print(state);
    Print(" ");
	Bws<10000> uri;
	aReceiver.SenderUri(uri);
	Print(uri);
    Print("\n");
}

void ReceiverManager2Logger::ReceiverChanged(ReceiverManager2Receiver& aReceiver)
{
    Print("Changed ");
    Print(aReceiver.Room());
    Print("(");
    Print(aReceiver.Group());
    Print(")");
	if (aReceiver.Selected()) {
		Print(" Selected ");
	}
	else {
		Print(" Not Selected ");
	}
	Bws<10000> state;
	aReceiver.TransportState(state);
	Print(state);
    Print(" ");
	Bws<10000> uri;
	aReceiver.SenderUri(uri);
	Print(uri);
    Print("\n");
}

void ReceiverManager2Logger::ReceiverRemoved(ReceiverManager2Receiver& aReceiver)
{
    Print("Removed ");
    Print(aReceiver.Room());
    Print("(");
    Print(aReceiver.Group());
    Print(")");
    Print("\n");
}

void ReceiverManager2Logger::ReceiverVolumeControlChanged(ReceiverManager2Receiver& aReceiver)
{
	Print("Vol Control Changed ");
	Print(aReceiver.Room());
	Print("(");
	Print(aReceiver.Group());
	Print("): ");
    aReceiver.HasVolumeControl() ? printf("Yes\n") : printf("No\n");
	if(aReceiver.HasVolumeControl())
	{
		Print("Vol      ");
		Bws<Ascii::kMaxUintStringBytes> bufferVol;
		Ascii::AppendDec(bufferVol, aReceiver.Volume());
		Print(bufferVol);
		Print("\n");
		Print("Mute      ");
		Bws<Ascii::kMaxUintStringBytes> bufferMute;
		Ascii::AppendDec(bufferMute, aReceiver.Mute());
		Print(bufferMute);
		Print("\n");
		Print("Vol Limit      ");
		Bws<Ascii::kMaxUintStringBytes> bufferVolLim;
		Ascii::AppendDec(bufferVolLim, aReceiver.VolumeLimit());
		Print(bufferVolLim);
		Print("\n");
	}
}

void ReceiverManager2Logger::ReceiverVolumeChanged(ReceiverManager2Receiver& aReceiver)
{
	Print("Vol Changed ");
	Print(aReceiver.Room());
	Print("(");
	Print(aReceiver.Group());
	Print("): ");
	Bws<Ascii::kMaxUintStringBytes> buffer;
	Ascii::AppendDec(buffer, aReceiver.Volume());
    Print(buffer);
    Print("\n");
}

void ReceiverManager2Logger::ReceiverMuteChanged(ReceiverManager2Receiver& aReceiver)
{
	Print("Mute Changed ");
	Print(aReceiver.Room());
	Print("(");
	Print(aReceiver.Group());
	Print("): ");
	Bws<Ascii::kMaxUintStringBytes> buffer;
	Ascii::AppendDec(buffer, aReceiver.Mute());
    Print(buffer);
    Print("\n");
}

void ReceiverManager2Logger::ReceiverVolumeLimitChanged(ReceiverManager2Receiver& aReceiver)
{
	Print("Vol Limit Changed ");
	Print(aReceiver.Room());
	Print("(");
	Print(aReceiver.Group());
	Print("): ");
	Bws<Ascii::kMaxUintStringBytes> buffer;
	Ascii::AppendDec(buffer, aReceiver.VolumeLimit());
    Print(buffer);
    Print("\n");
}

int CDECL main(int aArgc, char* aArgv[])
{
	InitialisationParams* initParams = InitialisationParams::Create();

    OptionParser parser;
    OptionUint optionDuration("-d", "--duration", 30, "Number of seconds to run the test");
    parser.AddOption(&optionDuration);
    if (!parser.Parse(aArgc, aArgv)) {
        return (1);
    }

    Library* lib = new Library(initParams);
    std::vector<NetworkAdapter*>* subnetList = lib->CreateSubnetList();
    TIpAddress subnet = (*subnetList)[0]->Subnet();
    Library::DestroySubnetList(subnetList);
    CpStack* cpStack = lib->StartCp(subnet);

    // Debug::SetLevel(Debug::kTopology);

	ReceiverManager2Logger* logger = new ReceiverManager2Logger(*cpStack);
	
    Blocker* blocker = new Blocker(lib->Env());
    blocker->Wait(optionDuration.Value());
    delete blocker;
	
	delete (logger);

	delete lib;
}
