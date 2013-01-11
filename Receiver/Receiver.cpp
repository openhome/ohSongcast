#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Debug.h>
#include <OpenHome/Net/Core/OhNet.h>

#include <vector>
#include <stdio.h>

#include "../OhmReceiver.h"

#ifdef _WIN32

#pragma warning(disable:4355) // use of 'this' in ctor lists safe in this case

#define CDECL __cdecl

#include <conio.h>

int mygetch()
{
    return (_getch());
}

#else

#define CDECL

#include <termios.h>
#include <unistd.h>

int mygetch()
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~( ICANON | ECHO );
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

#endif


using namespace OpenHome;
using namespace OpenHome::Net;
using namespace OpenHome::TestFramework;
using namespace OpenHome::Av;


class OhmReceiverDriver : public IOhmReceiverDriver, public IOhmMsgProcessor
{
public:
	OhmReceiverDriver();

private:
	// IOhmReceiverDriver
	virtual void Add(OhmMsg& aMsg);
	virtual void Timestamp(OhmMsg& aMsg);
	virtual void Started();
	virtual void Connected();
	virtual void Playing();
	virtual void Disconnected();
	virtual void Stopped();

	// IOhmMsgProcessor
	virtual void Process(OhmMsgAudio& aMsg);
	virtual void Process(OhmMsgTrack& aMsg);
	virtual void Process(OhmMsgMetatext& aMsg);

private:
	TBool iReset;
	TUint iCount;
	TUint iFrame;
};


OhmReceiverDriver::OhmReceiverDriver()
{
	iReset = true;
	iCount = 0;
}

void OhmReceiverDriver::Add(OhmMsg& aMsg)
{
	aMsg.Process(*this);
	aMsg.RemoveRef();
}

void OhmReceiverDriver::Timestamp(OhmMsg& /*aMsg*/)
{
}

void OhmReceiverDriver::Started()
{
	printf("=== STARTED ====\n");
}

void OhmReceiverDriver::Connected()
{
	iReset = true;
	printf("=== CONNECTED ====\n");
}

void OhmReceiverDriver::Playing()
{
	printf("=== PLAYING ====\n");
}

void OhmReceiverDriver::Disconnected()
{
	printf("=== DISCONNECTED ====\n");
}

void OhmReceiverDriver::Stopped()
{
	printf("=== STOPPED ====\n");
}

void OhmReceiverDriver::Process(OhmMsgAudio& aMsg)
{
	if (++iCount == 100) {
		printf(".");
		iCount = 0;
	}

	if (iReset) {
		iFrame = aMsg.Frame();
		iReset = false;
	}
	else {
		if (aMsg.Frame() != iFrame + 1) {
			printf("Missed frames between %d and %d\n", iFrame, aMsg.Frame());
		}
		iFrame = aMsg.Frame();
	}
}

void OhmReceiverDriver::Process(OhmMsgTrack& aMsg)
{
	printf("TRACK %d\n", aMsg.Sequence());
	/*
	printf("TRACK SEQUENCE %d\n", aMsg.Sequence());
	Brhz uri(aMsg.Uri());
	printf("TRACK URI %s\n", uri.CString());
	Brhz metadata(aMsg.Metadata());
	printf("TRACK METADATA %s\n", metadata.CString());
	*/
}

void OhmReceiverDriver::Process(OhmMsgMetatext& aMsg)
{
	printf("METATEXT %d\n", aMsg.Sequence());
	/*
	printf("METATEXT SEQUENCE %d\n", aMsg.Sequence());
	Brhz metatext(aMsg.Metatext());
	printf("METATEXT %s\n", metatext.CString());
	*/
}

int CDECL main(int aArgc, char* aArgv[])
{
    OptionParser parser;
    
    OptionUint optionAdapter("-a", "--adapter", 0, "[adapter] index of network adapter to use");
    parser.AddOption(&optionAdapter);

    OptionUint optionTtl("-t", "--ttl", 1, "[ttl] ttl");
    parser.AddOption(&optionTtl);

    OptionString optionUri("-u", "--uri", Brn("mpus://0.0.0.0:0"), "[uri] uri of the sender");
    parser.AddOption(&optionUri);
    
    if (!parser.Parse(aArgc, aArgv)) {
        return (1);
    }

    InitialisationParams* initParams = InitialisationParams::Create();

	Library* lib = new Library(initParams);

    std::vector<NetworkAdapter*>* subnetList = lib->CreateSubnetList();
    TIpAddress subnet = (*subnetList)[optionAdapter.Value()]->Subnet();
    TIpAddress adapter = (*subnetList)[optionAdapter.Value()]->Address();
    Library::DestroySubnetList(subnetList);

    printf("Using subnet %d.%d.%d.%d\n", subnet&0xff, (subnet>>8)&0xff, (subnet>>16)&0xff, (subnet>>24)&0xff);

    TUint ttl = optionTtl.Value();
	Brhz uri(optionUri.Value());

	OhmReceiverDriver* driver = new OhmReceiverDriver();

	OhmReceiver* receiver = new OhmReceiver(lib->Env(), adapter, ttl, *driver);

    CpStack* cpStack = lib->StartCp(subnet);
    cpStack = cpStack; // avoid unused variable warning

	printf("q = quit\n");
	
	Debug::SetLevel(Debug::kMedia);

    for (;;) {
    	int key = mygetch();
    	
    	if (key == 'q') {
			printf("QUIT\n");
			break;
    	}
    	else if (key == 'p') {
			printf("PLAY %s\n", uri.CString());
			receiver->Play(uri);
    	}
		else if (key == 's') {
			printf("STOP\n");
			receiver->Stop();
    	}
    }
       
	delete(receiver);

	delete lib;

	printf("\n");
	
    return (0);
}
