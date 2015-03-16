using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;
using System.Collections.Generic;
using System.Threading;
using OpenHome.Net.Core;

namespace OpenHome.Songcast
{
    public class SongcastError : Exception
    {
        internal SongcastError()
            : base("Songcast audio driver not installed")
        {
        }

        internal SongcastError(string aMsg)
            : base(aMsg)
        {
        }
    }

    public interface IMessageHandler
    {
        void Message(string aMessage);
    }

    public interface IConfiguration
    {
        uint Subnet();
        uint Channel();
        uint Ttl();
        uint Latency();
        bool Multicast();
        bool Enabled(); 
        uint Preset();
    }

    public interface IConfigurationChangedHandler
    {
        void ConfigurationChanged(IConfiguration aConfiguration);
    }

    public interface IReceiverHandler
    {
        void ReceiverAdded(IReceiver aReceiver);
        void ReceiverChanged(IReceiver aReceiver);
        void ReceiverRemoved(IReceiver aReceiver);
        void ReceiverVolumeControlChanged(IReceiver aReceiver);
        void ReceiverVolumeChanged(IReceiver aReceiver);
        void ReceiverMuteChanged(IReceiver aReceiver);
        void ReceiverVolumeLimitChanged(IReceiver aReceiver);
    }

    public enum EReceiverStatus
    {
        eDisconnected,
        eConnecting,
        eConnected
    }

    public interface IReceiver
    {
        void SetVolume(uint aValue);
        void VolumeInc();
        void VolumeDec();
        void SetMute(bool aValue);
        void Play();
        void Stop();
        void Standby();
        string Udn { get; }
        string Room { get; }
        string Group { get; }
        string Name { get; }
        EReceiverStatus Status { get; }
        bool HasVolumeControl { get; }
	    uint Volume { get; }
	    bool Mute { get; }
        uint VolumeLimit { get; }
        uint IpAddress { get; }
    }

    internal class Receiver : IReceiver, IDisposable
    {
        [DllImport("ohSongcast")]
        static extern IntPtr ReceiverUdn(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern IntPtr ReceiverRoom(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern IntPtr ReceiverGroup(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern IntPtr ReceiverName(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint ReceiverStatus(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint ReceiverIpAddress(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern bool ReceiverHasVolumeControl(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint ReceiverVolume(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern bool ReceiverMute(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint ReceiverVolumeLimit(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverSetVolume(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern void ReceiverVolumeInc(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverVolumeDec(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverSetMute(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcast")]
        static extern void ReceiverPlay(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverStop(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverStandby(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverAddRef(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void ReceiverRemoveRef(IntPtr aHandle);

        internal Receiver(IntPtr aReceiver)
        {
            iReceiver = aReceiver;
            ReceiverAddRef(iReceiver);
            iUdn = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(ReceiverUdn(iReceiver));
            iRoom = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(ReceiverRoom(iReceiver));
            iGroup = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(ReceiverGroup(iReceiver));
            iName = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(ReceiverName(iReceiver));
        }

        internal bool Owns(IntPtr aReceiver)
        {
            return (iReceiver == aReceiver);
        }

        public void Dispose()
        {
            ReceiverRemoveRef(iReceiver);
        }

        public void SetVolume(uint aValue)
        {
            ReceiverSetVolume(iReceiver, aValue);
        }

        public void VolumeInc()
        {
            ReceiverVolumeInc(iReceiver);
        }

        public void VolumeDec()
        {
            ReceiverVolumeDec(iReceiver);
        }

        public void SetMute(bool aValue)
        {
            ReceiverSetMute(iReceiver, aValue);
        }

        public void Play()
        {
            ReceiverPlay(iReceiver);
        }

        public void Stop()
        {
            ReceiverStop(iReceiver);
        }

        public void Standby()
        {
            ReceiverStandby(iReceiver);
        }

        public string Udn
        {
            get
            {
                return (iUdn);
            }
        }

        public string Room
        {
            get
            {
                return (iRoom);
            }
        }

        public string Group
        {
            get
            {
                return (iGroup);
            }
        }

        public string Name
        {
            get
            {
                return (iName);
            }
        }

        public EReceiverStatus Status
        {
            get
            {
                return ((EReceiverStatus)ReceiverStatus(iReceiver));
            }
        }

        public bool HasVolumeControl
        {
            get
            {
                return ReceiverHasVolumeControl(iReceiver);
            }
        }

        public uint Volume
        {
            get
            {
                return ReceiverVolume(iReceiver);
            }
        }

        public bool Mute
        {
            get
            {
                return ReceiverMute(iReceiver);
            }
        }

        public uint VolumeLimit
        {
            get
            {
                return ReceiverVolumeLimit(iReceiver);
            }
        }

        public uint IpAddress
        {
            get
            {
                return ReceiverIpAddress(iReceiver);
            }
        }

        IntPtr iReceiver;
        string iUdn;
        string iRoom;
        string iGroup;
        string iName;
    }

    public interface ISubnetHandler
    {
        void SubnetAdded(ISubnet aSubnet);
        void SubnetChanged(ISubnet aSubnet);
        void SubnetRemoved(ISubnet aSubnet);
    }

    public interface ISubnet
    {
        uint Address { get; }
        string AdapterName { get; }
    }

    internal class Subnet : ISubnet, IDisposable
    {
        [DllImport("ohSongcast")]
        static extern IntPtr SubnetAddress(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern IntPtr SubnetAdapterName(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void SubnetAddRef(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void SubnetRemoveRef(IntPtr aHandle);

        public Subnet(IntPtr aSubnet)
        {
            iSubnet = aSubnet;
            iMutex = new Mutex();
            iAddress = (uint)SubnetAddress(iSubnet);
            iAdapterName = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(SubnetAdapterName(iSubnet));
            SubnetAddRef(iSubnet);
        }

        internal bool Owns(IntPtr aSubnet)
        {
            iMutex.WaitOne();
            bool owns = (iSubnet == aSubnet);
            iMutex.ReleaseMutex();
            return (owns);
        }

        internal void Update()
        {
            iMutex.WaitOne();
            iAddress = (uint)SubnetAddress(iSubnet);
            iAdapterName = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8(SubnetAdapterName(iSubnet));
            iMutex.ReleaseMutex();
        }

        public uint Address
        {
            get
            {
                iMutex.WaitOne();
                uint address = iAddress;
                iMutex.ReleaseMutex();
                return (address);
            }
        }

        public string AdapterName
        {
            get
            {
                iMutex.WaitOne();
                string name = iAdapterName;
                iMutex.ReleaseMutex();
                return (name);
            }
        }

        public void Dispose()
        {
            iMutex.WaitOne();
            SubnetRemoveRef(iSubnet);
            iMutex.ReleaseMutex();
        }

        IntPtr iSubnet;
        Mutex iMutex;
        uint iAddress;
        string iAdapterName;
    }

    public class Songcast : IDisposable, IConfiguration
    {
        private enum ECallbackType
        {
            eAdded,
            eChanged,
            eRemoved,
            eVolumeControlChanged,
            eVolumeChanged,
            eMuteChanged,
            eVolumeLimitChanged
        }

        private delegate void DelegateReceiverCallback(IntPtr aPtr, ECallbackType aType, IntPtr aReceiver);
        private delegate void DelegateSubnetCallback(IntPtr aPtr, ECallbackType aType, IntPtr aSubnet);
        private delegate void DelegateConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcast);
        private unsafe delegate void DelegateMessageCallback(IntPtr aPtr, char* aMessage);

        [DllImport("ohSongcast")]
        static extern unsafe IntPtr SongcastCreate(string aDomain, uint aSubnet, uint aChannel, uint aTtl, uint aLatency, bool aMulticast, bool aEnabled, uint aPreset, DelegateReceiverCallback aReceiverCallback, IntPtr aReceiverPtr, DelegateSubnetCallback aSubnetCallback, IntPtr aSubnetPtr, DelegateConfigurationChangedCallback aConfigurationChangedCallback, IntPtr aConfigurationChangedPtr, DelegateMessageCallback aFatalErrorCallback, IntPtr aFatalErrorPtr, DelegateMessageCallback aLogOutputCallback, IntPtr aLogOutputPtr, string aManufacturer, string aManufacturerUrl, string aModelUrl, byte[] aImagePtr, int aImageBytes, string aMimeType);

        [DllImport("ohSongcast")]
        static extern uint SongcastSubnet(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint SongcastChannel(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint SongcastTtl(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint SongcastLatency(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern bool SongcastMulticast(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern bool SongcastEnabled(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern uint SongcastPreset(IntPtr aHandle);
        
        [DllImport("ohSongcast")]
        static extern void SongcastSetSubnet(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetChannel(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetTtl(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetLatency(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetMulticast(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetEnabled(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastSetPreset(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast")]
        static extern unsafe void SongcastSetTrack(IntPtr aHandle, char* aUri, char* aMetadata, long aSamplesTotal, long aSampleStart);
        [DllImport("ohSongcast")]
        static extern unsafe void SongcastSetMetatext(IntPtr aHandle, char* aValue);
        [DllImport("ohSongcast")]
        static extern void SongcastRefreshReceivers(IntPtr aHandle);
        [DllImport("ohSongcast")]
        static extern void SongcastDestroy(IntPtr aHandle);

        public unsafe Songcast(string aDomain, uint aSubnet, uint aChannel, uint aTtl, uint aLatency, bool aMulticast, bool aEnabled, uint aPreset, IReceiverHandler aReceiverHandler, ISubnetHandler aSubnetHandler, IConfigurationChangedHandler aConfigurationChangedHandler, IMessageHandler aLogOutputHandler, string aManufacturer, string aManufacturerUrl, string aModelUrl, byte[] aImage, string aMimeType)
        {
            iReceiverHandler = aReceiverHandler;
            iSubnetHandler = aSubnetHandler;
            iConfigurationChangedHandler = aConfigurationChangedHandler;
            iLogOutputHandler = aLogOutputHandler;
            iReceiverCallback = new DelegateReceiverCallback(ReceiverCallback);
            iSubnetCallback = new DelegateSubnetCallback(SubnetCallback);
            iConfigurationChangedCallback = new DelegateConfigurationChangedCallback(ConfigurationChangedCallback);
            iFatalErrorCallback = new DelegateMessageCallback(FatalErrorCallback);
            iLogOutputCallback = new DelegateMessageCallback(LogOutputCallback);
            iReceiverList = new List<Receiver>();
            iSubnetList = new List<Subnet>();
            iInitParams = new InitParams();
            iLibrary = Library.Create(iInitParams);

            iHandle = SongcastCreate(aDomain, aSubnet, aChannel, aTtl, aLatency, aMulticast, aEnabled, aPreset, iReceiverCallback, IntPtr.Zero, iSubnetCallback, IntPtr.Zero, iConfigurationChangedCallback, IntPtr.Zero, iFatalErrorCallback, IntPtr.Zero, iLogOutputCallback, IntPtr.Zero, aManufacturer, aManufacturerUrl, aModelUrl, aImage, aImage.Length, aMimeType);

            if (iHandle == IntPtr.Zero)
            {
                throw (new SongcastError());
            }
        }

        private void ReceiverCallback(IntPtr aPtr, ECallbackType aType, IntPtr aReceiver)
        {
            switch (aType)
            {
                case ECallbackType.eAdded:
                    ReceiverAdded(aReceiver);
                    break;
                case ECallbackType.eChanged:
                    ReceiverChanged(aReceiver);
                    break;
                case ECallbackType.eRemoved:
                    ReceiverRemoved(aReceiver);
                    break;
                case ECallbackType.eVolumeControlChanged:
                    ReceiverVolumeControlChanged(aReceiver);
                    break;
                case ECallbackType.eVolumeChanged:
                    ReceiverVolumeChanged(aReceiver);
                    break;
                case ECallbackType.eMuteChanged:
                    ReceiverMuteChanged(aReceiver);
                    break;
                case ECallbackType.eVolumeLimitChanged:
                    ReceiverVolumeLimitChanged(aReceiver);
                    break;
            }
        }

        private void ReceiverAdded(IntPtr aReceiver)
        {
            Receiver receiver = new Receiver(aReceiver);
            iReceiverList.Add(receiver);
            iReceiverHandler.ReceiverAdded(receiver);
        }

        private void ReceiverChanged(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverChanged(receiver);
                    return;
                }
            }
        }

        private void ReceiverRemoved(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverRemoved(receiver);
                    receiver.Dispose();
                    iReceiverList.Remove(receiver);
                    return;
                }
            }
        }

        private void ReceiverVolumeControlChanged(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverVolumeControlChanged(receiver);
                    return;
                }
            }
        }

        private void ReceiverVolumeChanged(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverVolumeChanged(receiver);
                    return;
                }
            }
        }

        private void ReceiverMuteChanged(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverMuteChanged(receiver);
                    return;
                }
            }
        }

        private void ReceiverVolumeLimitChanged(IntPtr aReceiver)
        {
            foreach (Receiver receiver in iReceiverList)
            {
                if (receiver.Owns(aReceiver))
                {
                    iReceiverHandler.ReceiverVolumeLimitChanged(receiver);
                    return;
                }
            }
        }

        private void SubnetCallback(IntPtr aPtr, ECallbackType aType, IntPtr aSubnet)
        {
            switch (aType)
            {
                case ECallbackType.eAdded:
                    SubnetAdded(aSubnet);
                    break;
                case ECallbackType.eChanged:
                    SubnetChanged(aSubnet);
                    break;
                case ECallbackType.eRemoved:
                    SubnetRemoved(aSubnet);
                    break;
            }
        }

        private void SubnetAdded(IntPtr aSubnet)
        {
            Subnet subnet = new Subnet(aSubnet);
            iSubnetList.Add(subnet);
            iSubnetHandler.SubnetAdded(subnet);
        }

        private void SubnetChanged(IntPtr aSubnet)
        {
            foreach (Subnet subnet in iSubnetList)
            {
                if (subnet.Owns(aSubnet))
                {
                    subnet.Update();
                    iSubnetHandler.SubnetChanged(subnet);
                    return;
                }
            }
        }

        private void SubnetRemoved(IntPtr aSubnet)
        {
            foreach (Subnet subnet in iSubnetList)
            {
                if (subnet.Owns(aSubnet))
                {
                    iSubnetHandler.SubnetRemoved(subnet);
                    subnet.Dispose();
                    iSubnetList.Remove(subnet);
                    return;
                }
            }
        }

        private void ConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcast)
        {
            iConfigurationChangedHandler.ConfigurationChanged(this);
        }

        private unsafe void FatalErrorCallback(IntPtr aPtr, char* aMessage)
        {
            string msg = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8((IntPtr)aMessage);
            throw new SongcastError(msg);
        }

        private unsafe void LogOutputCallback(IntPtr aPtr, char* aMessage)
        {
            string msg = OpenHome.Net.Core.InteropUtils.PtrToStringUtf8((IntPtr)aMessage);
            iLogOutputHandler.Message(msg);
        }

        public uint Subnet()
        {
            return (SongcastSubnet(iHandle));
        }

        public uint Channel()
        {
            return (SongcastChannel(iHandle));
        }

        public uint Ttl()
        {
            return (SongcastTtl(iHandle));
        }

        public uint Latency()
        {
            return (SongcastLatency(iHandle));
        }

        public bool Multicast()
        {
            return (SongcastMulticast(iHandle));
        }

        public bool Enabled()
        {
            return (SongcastEnabled(iHandle));
        }

        public uint Preset()
        {
            return (SongcastPreset(iHandle));
        }

        public void SetSubnet(uint aValue)
        {
            SongcastSetSubnet(iHandle, aValue);
        }

        public void SetChannel(uint aValue)
        {
            SongcastSetChannel(iHandle, aValue);
        }

        public void SetTtl(uint aValue)
        {
            SongcastSetTtl(iHandle, aValue);
        }

        public void SetLatency(uint aValue)
        {
            SongcastSetLatency(iHandle, aValue);
        }

        public void SetMulticast(bool aValue)
        {
            SongcastSetMulticast(iHandle, aValue);
        }

        public void SetEnabled(bool aValue)
        {
            SongcastSetEnabled(iHandle, aValue);
        }

        public void SetPreset(uint aValue)
        {
            SongcastSetPreset(iHandle, aValue);
        }

        public unsafe void SetTrack(IntPtr aHandle, string aUri, string aMetadata, long aSamplesTotal, long aSampleStart)
        {
            IntPtr uri = OpenHome.Net.Core.InteropUtils.StringToHGlobalUtf8(aUri);
            IntPtr metadata = OpenHome.Net.Core.InteropUtils.StringToHGlobalUtf8(aMetadata);
            SongcastSetTrack(iHandle, (char*)uri, (char*)metadata, aSamplesTotal, aSampleStart);
            Marshal.FreeHGlobal(uri);
            Marshal.FreeHGlobal(metadata);
        }

        public unsafe void SetMetatext(IntPtr aHandle, string aValue)
        {
            IntPtr value = OpenHome.Net.Core.InteropUtils.StringToHGlobalUtf8(aValue);
            SongcastSetMetatext(iHandle, (char*)value);
            Marshal.FreeHGlobal(value);
        }

        public void RefreshReceivers()
        {
            SongcastRefreshReceivers(iHandle);
        }

        public void Dispose()
        {
            SongcastDestroy(iHandle);
        }

        private InitParams iInitParams;
        private Library iLibrary;
        private IReceiverHandler iReceiverHandler;
        private ISubnetHandler iSubnetHandler;
        private IConfigurationChangedHandler iConfigurationChangedHandler;
        private IMessageHandler iLogOutputHandler;
        private IntPtr iHandle;
        private List<Receiver> iReceiverList;
        private List<Subnet> iSubnetList;
        private DelegateReceiverCallback iReceiverCallback;
        private DelegateSubnetCallback iSubnetCallback;
        private DelegateConfigurationChangedCallback iConfigurationChangedCallback;
        private DelegateMessageCallback iFatalErrorCallback;
        private DelegateMessageCallback iLogOutputCallback;
    }
} // namespace OpenHome.Songcast
