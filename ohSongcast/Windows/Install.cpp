#include "SoundcardDriver.h"

#include <OpenHome/Private/Ascii.h>
#include <OpenHome/Private/Maths.h>
#include <OpenHome/Private/Arch.h>
#include <OpenHome/Private/Debug.h>

#include <Setupapi.h>
#include <ks.h>
#include <ksmedia.h>
#include <initguid.h>
#include <newdev.h>

using namespace OpenHome;
using namespace OpenHome::Net;

//
// UpdateDriverForPlugAndPlayDevices
//
typedef BOOL (WINAPI *UpdateDriverForPlugAndPlayDevicesProto)(__in HWND hwndParent, __in LPCTSTR HardwareId,
                                                              __in LPCTSTR FullInfPath,
                                                              __in DWORD InstallFlags,
                                                              __out_opt PBOOL bRebootRequired);


#ifdef _UNICODE
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesW"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfW"
#else
#define UPDATEDRIVERFORPLUGANDPLAYDEVICES "UpdateDriverForPlugAndPlayDevicesA"
#define SETUPUNINSTALLOEMINF "SetupUninstallOEMInfA"
#endif

// OhmSenderDriverWindows

DEFINE_GUID(OHSOUNDCARD_GUID, 0x2685C863, 0x5E57, 0x4D9A, 0x86, 0xEC, 0x2E, 0xC9, 0xB7, 0xBB, 0xBC, 0xFD);

static const TUint KSPROPERTY_OHSOUNDCARD_VERSION = 0;
static const TUint KSPROPERTY_OHSOUNDCARD_ENABLED = 1;
static const TUint KSPROPERTY_OHSOUNDCARD_ACTIVE = 2;
static const TUint KSPROPERTY_OHSOUNDCARD_ENDPOINT = 3;
static const TUint KSPROPERTY_OHSOUNDCARD_TTL = 4;

TBool FindDriver(const char* aDomain)
{
	char hwid[200];

	strcpy(hwid, aDomain);
	strcpy(hwid + strlen(hwid), "/Songcast");

    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&KSCATEGORY_AUDIO, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PROFILE | DIGCF_PRESENT);

	if (deviceInfoSet == 0) {
		return (false);
	}

	SP_DEVICE_INTERFACE_DATA deviceInterfaceData;

	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	// build a DevInfo Data structure

	SP_DEVINFO_DATA deviceInfoData;

	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		 
	// build a Device Interface Detail Data structure

	TByte* detail = new TByte[1000];

	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)detail;

	deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    for (TUint i = 0; SetupDiEnumDeviceInterfaces(deviceInfoSet, 0, &KSCATEGORY_AUDIO, i, &deviceInterfaceData); i++)
	{
		// now we can get some more detailed information

        if (SetupDiGetDeviceInterfaceDetail(deviceInfoSet, &deviceInterfaceData, deviceInterfaceDetailData, 1000, 0, &deviceInfoData))
        {
            char buffer[1024];
            if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (LPBYTE)buffer, 1024, NULL)) {
                printf("%s\n", buffer);
                if(strcmp(hwid, buffer) == 0) {
                    try
                    {
                        printf("Found a valid driver: %s\n", deviceInterfaceDetailData->DevicePath);
                        
                        HANDLE handle = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);

                        KSPROPERTY prop;
                        
                        prop.Set = OHSOUNDCARD_GUID;
                        prop.Id = KSPROPERTY_OHSOUNDCARD_VERSION;
                        prop.Flags = KSPROPERTY_TYPE_GET;

                        TByte buffer[4];

                        DWORD bytes;

                        if (DeviceIoControl(handle, IOCTL_KS_PROPERTY, &prop, sizeof(KSPROPERTY), buffer, sizeof(buffer), &bytes, 0))
                        {
                            TUint version = *(TUint*)buffer;

                            if (version == 1) {
                                delete [] detail;
                                SetupDiDestroyDeviceInfoList(deviceInfoSet);
                                return (true);
                            }
                        }
                    }
                    catch (...)
                    {
                    }
                }
            }
        }
	}

	delete [] detail;

	SetupDiDestroyDeviceInfoList(deviceInfoSet);

	return (false);
}

TBool InstallDriver(const char* aDomain, const char* aInfFilename)
{
	char hwid[200];

	strcpy(hwid, aDomain);
	strcpy(hwid + strlen(hwid), "/Songcast");

	printf("Installing %s from %s\n", hwid, aInfFilename);

	HDEVINFO deviceInfoSet = SetupDiCreateDeviceInfoList(&KSCATEGORY_AUDIO, 0);

    if (deviceInfoSet == 0) {
		printf("1\n");
		return (false);
    }

    //
    // Now create the element.
    // Use the Class GUID and Name from the INF file.
    //

	SP_DEVINFO_DATA deviceInfoData;

    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (!SetupDiCreateDeviceInfo(deviceInfoSet,
        "MEDIA",
        &KSCATEGORY_AUDIO,
        NULL,
        0,
        DICD_GENERATE_ID,
        &deviceInfoData))
    {
		printf("2 %d\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
		return (false);
    }

	//char* hwid = "{D6BAC7AB-8758-43A9-917F-D702501F4DB0}\\Soundkard\0\0";

    //
    // Add the HardwareID to the Device's HardwareID property.
    //
    if (!SetupDiSetDeviceRegistryProperty(deviceInfoSet,
        &deviceInfoData,
        SPDRP_HARDWAREID,
        (LPBYTE)hwid,
        (lstrlen(hwid))))
    {
		printf("3 %d\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
		return (false);
    }

    //
    // Transform the registry element into an actual devnode
    // in the PnP HW tree.
    //
    if (!SetupDiCallClassInstaller(DIF_REGISTERDEVICE,
        deviceInfoSet,
        &deviceInfoData))
    {
		printf("4 %d\n", GetLastError());
        SetupDiDestroyDeviceInfoList(deviceInfoSet);
		return (false);
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

	// Now update driver files

    char inf[200];

	//
    // Convert inf to full pathname
    //

    //int res = GetFullPathName("Soundkard.inf", 200, inf, NULL);
    int res = GetFullPathName(aInfFilename, 200, inf, NULL);

    if ((res >= 200) || (res == 0)) {
		printf("5 %d\n", GetLastError());
		return (false);
    }

    if (GetFileAttributes(inf) < 0) {
		printf("6 %d\n", GetLastError());
		return (false);
    }

    //
    // make use of UpdateDriverForPlugAndPlayDevices
    //

	HMODULE newDevDll = LoadLibrary(TEXT("newdev.dll"));

    if (newDevDll == 0) {
		printf("7 %d\n", GetLastError());
		return (false);
    }

    UpdateDriverForPlugAndPlayDevicesProto updateFunction = (UpdateDriverForPlugAndPlayDevicesProto)GetProcAddress(newDevDll, UPDATEDRIVERFORPLUGANDPLAYDEVICES);

    if (updateFunction == 0)
    {
		printf("8 %d\n", GetLastError());
        FreeLibrary(newDevDll);
		return (false);
    }

	BOOL reboot = FALSE;

	printf(inf);
	printf("\n");

	if (!updateFunction(NULL, hwid, inf, INSTALLFLAG_FORCE, &reboot)) {
		printf("9 %d\n", GetLastError());
        FreeLibrary(newDevDll);
		return (false);
    }

    FreeLibrary(newDevDll);

	return (true);
}

TBool DeleteDriver(const char* aDomain, const char* aInfFilename)
{
	char hwid[200];

	strcpy(hwid, aDomain);
	strcpy(hwid + strlen(hwid), "/Songcast");

	printf("Deleting %s from %s\n", hwid, aInfFilename);
    
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(&KSCATEGORY_AUDIO, 0, 0, DIGCF_DEVICEINTERFACE | DIGCF_PROFILE | DIGCF_PRESENT);

	if (deviceInfoSet == 0) {
		return (false);
	}

	// build a DevInfo Data structure

	SP_DEVINFO_DATA deviceInfoData;

	deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

	for (TUint i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++)
	{
        char buffer[1024];
        if (SetupDiGetDeviceRegistryProperty(deviceInfoSet, &deviceInfoData, SPDRP_HARDWAREID, NULL, (LPBYTE)buffer, 1024, NULL)) {
            printf("%s\n", buffer);
            if (strcmp(hwid, buffer) == 0) {
                if (SetupDiBuildDriverInfoList(deviceInfoSet, &deviceInfoData, SPDIT_COMPATDRIVER)) {
                    SP_DRVINFO_DATA driverInfoData;
                    
                    driverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
                    
                    SP_DRVINFO_DETAIL_DATA driverInfoDetail;
                    
                    driverInfoDetail.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
                    
                    if (SetupDiEnumDriverInfo(deviceInfoSet, &deviceInfoData, SPDIT_COMPATDRIVER, 0, &driverInfoData)) {
                        bool deleteFiles = true;
                        if (!SetupDiGetDriverInfoDetail(deviceInfoSet, &deviceInfoData, &driverInfoData, &driverInfoDetail, sizeof(driverInfoDetail), NULL)) {
                            HRESULT result = GetLastError();
                            if (result != ERROR_INSUFFICIENT_BUFFER) {
                                printf("1 %d\n", GetLastError());
                                deleteFiles = false;
                            }
                        }
                        
                        if (deleteFiles) {
                            size_t length = strlen(driverInfoDetail.InfFileName);
                            char pnfFilename[200];
                            strncpy(pnfFilename, driverInfoDetail.InfFileName, length - 3);
                            strcpy(pnfFilename + length - 3, "pnf");
                            
                            if (DeleteFile(driverInfoDetail.InfFileName)) {
                                printf("Deleted %s\n", driverInfoDetail.InfFileName);
                            }
                            else {
                                printf("Couldn't find %s\n", driverInfoDetail.InfFileName);
                            }
                            if (DeleteFile(pnfFilename)) {
                                printf("Deleted %s\n", pnfFilename);
                            }
                            else {
                                printf("Couldn't find %s\n", pnfFilename);
                            }
                        }
                    }
                    else {
                        printf("2 %d\n", GetLastError());
                    }
                }
                else {
                    printf("3 %d\n", GetLastError());
                }
        
                if(SetupDiCallClassInstaller(DIF_REMOVE, deviceInfoSet, &deviceInfoData)) {
                    printf("Deleted device with hardware id %d\n", hwid);
                }
                else {
                    printf("4 %d\n", GetLastError());
                }
            }            
        }
        else {
            printf("5 %d\n", GetLastError());
        }
	}

	SetupDiDestroyDeviceInfoList(deviceInfoSet);
    
    return (true);
}

int __cdecl main(int aArgc, char* aArgv[])
{
    if(aArgc != 4)
    {
        printf("Incorrect number of args\n\n");
        printf("Usage:\n");
        printf("Install driver and device:  InstallXX.exe -i <Hardware Id> <Inf>\n");
        printf("Delete driver and device :  InstallXX.exe -d <Hardware Id> <Inf>\n");
        return(1);
    }
    
	
    if (strcmp(aArgv[1], "-i") == 0) {
        if (!FindDriver(aArgv[2])) {
            if (!InstallDriver(aArgv[2], aArgv[3])) {
                return (1);
            }
            if (!FindDriver(aArgv[2])) {
                return (1);
            }
        }
    }
    else if (strcmp(aArgv[1], "-d") == 0) {
        if(!DeleteDriver(aArgv[2], aArgv[3])) {
            return (1);
        }
        if (FindDriver(aArgv[2])) {
            return (1);
        }
    }
    else {
        printf("Unrecognised option %s\n\n", aArgv[1]);
        printf("Usage:\n");
        printf("Install driver and device:  InstallXX.exe -i <Hardware Id> <Inf>\n");
        printf("Delete driver and device :  InstallXX.exe -d <Hardware Id> <Inf>\n");
        return(1);
    }
    
	return (0);
}

