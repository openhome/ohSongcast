#include <OpenHome/OhNetTypes.h>
#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Net/Private/Stack.h>
#include <OpenHome/Private/Thread.h>
#include <OpenHome/Private/OptionParser.h>
#include <OpenHome/Private/Debug.h>

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


class OhmReceiverDriver : public IOhmReceiverDriver, public IOhmMsgProcessor
{
public:
	OhmReceiverDriver();

private:
	// IOhmReceiverDriver
	virtual void Add(OhmMsg& aMsg);
	virtual void SetTransportState(EOhmReceiverTransportState aValue);

	// IOhmMsgProcessor
	virtual void Process(OhmMsgAudio& aMsg);
	virtual void Process(OhmMsgTrack& aMsg);
	virtual void Process(OhmMsgMetatext& aMsg);
};


OhmReceiverDriver::OhmReceiverDriver()
{
}

void OhmReceiverDriver::Add(OhmMsg& aMsg)
{
	aMsg.Process(*this);
	aMsg.RemoveRef();
}

void OhmReceiverDriver::SetTransportState(EOhmReceiverTransportState aValue)
{
	printf("TRANSPORT STATE: ");
	switch(aValue)
	{
	case ePlaying:
		printf("Playing\n");
		break;
	case eStopped:
		printf("Stopped\n");
		break;
	case eWaiting:
		printf("Waiting\n");
		break;
	case eBuffering:
		printf("Buffering\n");
		break;
	default:
		printf("Unknown\n");
		break;
	}
}

void OhmReceiverDriver::Process(OhmMsgAudio& aMsg)
{
	printf("-%d-", aMsg.Frame());
}

void OhmReceiverDriver::Process(OhmMsgTrack& aMsg)
{
	printf("TRACK SEQUENCE %d\n", aMsg.Sequence());
	Brhz uri(aMsg.Uri());
	printf("TRACK URI\n%s\n", uri.CString());
	Brhz metadata(aMsg.Metadata());
	printf("TRACK METADATA\n%s\n", metadata.CString());
}

void OhmReceiverDriver::Process(OhmMsgMetatext& aMsg)
{
	printf("METATEXT SEQUENCE %d\n", aMsg.Sequence());
	Brhz metatext(aMsg.Metatext());
	printf("METATEXT\n%s\n", metatext.CString());
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

	UpnpLibrary::Initialise(initParams);

    std::vector<NetworkAdapter*>* subnetList = UpnpLibrary::CreateSubnetList();
    TIpAddress subnet = (*subnetList)[optionAdapter.Value()]->Subnet();
    TIpAddress adapter = (*subnetList)[optionAdapter.Value()]->Address();
    UpnpLibrary::DestroySubnetList(subnetList);

    printf("Using subnet %d.%d.%d.%d\n", subnet&0xff, (subnet>>8)&0xff, (subnet>>16)&0xff, (subnet>>24)&0xff);

    TUint ttl = optionTtl.Value();
	Brhz uri(optionUri.Value());

	OhmReceiverDriver* driver = new OhmReceiverDriver();

	OhmReceiver* receiver = new OhmReceiver(adapter, ttl, *driver);

    UpnpLibrary::StartCp(subnet);

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

	UpnpLibrary::Close();

	printf("\n");
	
    return (0);
}
