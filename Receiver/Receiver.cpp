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

int CDECL main(int aArgc, char* aArgv[])
{
    OptionParser parser;
    
    OptionUint optionChannel("-c", "--channel", 0, "[0..65535] sender channel");
    parser.AddOption(&optionChannel);

    OptionUint optionAdapter("-a", "--adapter", 0, "[adapter] index of network adapter to use");
    parser.AddOption(&optionAdapter);

    OptionUint optionTtl("-t", "--ttl", 1, "[ttl] ttl");
    parser.AddOption(&optionTtl);

    if (!parser.Parse(aArgc, aArgv)) {
        return (1);
    }

    InitialisationParams* initParams = InitialisationParams::Create();

	UpnpLibrary::Initialise(initParams);

    std::vector<NetworkAdapter*>* subnetList = UpnpLibrary::CreateSubnetList();
    TIpAddress subnet = (*subnetList)[optionAdapter.Value()]->Subnet();
    //TIpAddress adapter = (*subnetList)[optionAdapter.Value()]->Address();
    UpnpLibrary::DestroySubnetList(subnetList);

    printf("Using subnet %d.%d.%d.%d\n", subnet&0xff, (subnet>>8)&0xff, (subnet>>16)&0xff, (subnet>>24)&0xff);

    //TUint channel = optionChannel.Value();
    //TUint ttl = optionTtl.Value();

    UpnpLibrary::StartCp(subnet);

	printf("q = quit\n");
	
    for (;;) {
    	int key = mygetch();
    	
    	if (key == 'q') {
    		break;
    	}
    }
       
	UpnpLibrary::Close();

	printf("\n");
	
    return (0);
}
