using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;
using System.Collections.Generic;

namespace OpenHome.Songcaster
{
    public class SongcasterError : Exception
    {
        internal SongcasterError()
            : base("Songcaster audio driver not installed")
        {
        }
    }

    public interface IConfiguration
    {
        uint Subnet();
        uint Channel();
        uint Ttl();
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
    }

    public enum EReceiverStatus
    {
        eDisconnected,
        eConnecting,
        eConnected
    }

    public interface IReceiver
    {
        void Play();
        void Stop();
        void Standby();
        string Udn { get; }
        string Room { get; }
        string Group { get; }
        string Name { get; }
        EReceiverStatus Status { get; }
    }

    internal class Receiver : IReceiver, IDisposable
    {
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr ReceiverUdn(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr ReceiverRoom(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr ReceiverGroup(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr ReceiverName(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern uint ReceiverStatus(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void ReceiverPlay(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void ReceiverStop(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void ReceiverStandby(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void ReceiverAddRef(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void ReceiverRemoveRef(IntPtr aHandle);

        internal Receiver(IntPtr aReceiver)
        {
            iReceiver = aReceiver;
            ReceiverAddRef(iReceiver);
            iUdn = Marshal.PtrToStringAnsi(ReceiverUdn(iReceiver));
            iRoom = Marshal.PtrToStringAnsi(ReceiverRoom(iReceiver));
            iGroup = Marshal.PtrToStringAnsi(ReceiverGroup(iReceiver));
            iName = Marshal.PtrToStringAnsi(ReceiverName(iReceiver));
        }

        internal bool Owns(IntPtr aReceiver)
        {
            return (iReceiver == aReceiver);
        }

        public void Dispose()
        {
            ReceiverRemoveRef(iReceiver);
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
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr SubnetAddress(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern IntPtr SubnetAdapterName(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void SubnetAddRef(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void SubnetRemoveRef(IntPtr aHandle);

        public Subnet(IntPtr aSubnet)
        {
            iSubnet = aSubnet;
            iAddress = (uint)SubnetAddress(iSubnet);
            iAdapterName = Marshal.PtrToStringAnsi(SubnetAdapterName(iSubnet));
        }

        internal bool Owns(IntPtr aSubnet)
        {
            return (iSubnet == aSubnet);
        }

        public uint Address
        {
            get
            {
                return (iAddress);
            }
        }

        public string AdapterName
        {
            get
            {
                return (iAdapterName);
            }
        }

        public void Dispose()
        {
            SubnetRemoveRef(iSubnet);
        }

        IntPtr iSubnet;
        uint iAddress;
        string iAdapterName;
    }

    public class Songcaster : IDisposable, IConfiguration
    {
        private enum ECallbackType
        {
            eAdded,
            eChanged,
            eRemoved
        }

        private delegate void DelegateReceiverCallback(IntPtr aPtr, ECallbackType aType, IntPtr aReceiver);
        private delegate void DelegateSubnetCallback(IntPtr aPtr, ECallbackType aType, IntPtr aSubnet);
        private delegate void DelegateConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcaster);

        [DllImport("ohSongcaster.dll")]
        static extern unsafe IntPtr SongcasterCreate(string aDomain, uint aSubnet, uint aChannel, uint aTtl, bool aMulticast, bool aEnabled, uint aPreset, DelegateReceiverCallback aReceiverCallback, IntPtr aReceiverPtr, DelegateSubnetCallback aSubnetCallback, IntPtr aSubnetPtr, DelegateConfigurationChangedCallback aConfigurationChangedCallback, IntPtr aConfigurationChangedPtr, string aManufacturer, string aManufacturerUrl, string aModelUrl);

        [DllImport("ohSongcaster.dll")]
        static extern uint SongcasterSubnet(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern uint SongcasterChannel(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern uint SongcasterTtl(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern bool SongcasterMulticast(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern bool SongcasterEnabled(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern uint SongcasterPreset(IntPtr aHandle);
        
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetSubnet(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetChannel(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetTtl(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetMulticast(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetEnabled(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterSetPreset(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcaster.dll")]
        static extern unsafe void SongcasterSetTrack(IntPtr aHandle, char* aUri, char* aMetadata, long aSamplesTotal, long aSampleStart);
        [DllImport("ohSongcaster.dll")]
        static extern unsafe void SongcasterSetMetatext(IntPtr aHandle, char* aValue);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterRefreshReceivers(IntPtr aHandle);
        [DllImport("ohSongcaster.dll")]
        static extern void SongcasterDestroy(IntPtr aHandle);

        public unsafe Songcaster(string aDomain, uint aSubnet, uint aChannel, uint aTtl, bool aMulticast, bool aEnabled, uint aPreset, IReceiverHandler aReceiverHandler, ISubnetHandler aSubnetHandler, IConfigurationChangedHandler aConfigurationChangedHandler, string aManufacturer, string aManufacturerUrl, string aModelUrl)
        {
            iReceiverHandler = aReceiverHandler;
            iSubnetHandler = aSubnetHandler;
            iConfigurationChangedHandler = aConfigurationChangedHandler;
            iReceiverCallback = new DelegateReceiverCallback(ReceiverCallback);
            iSubnetCallback = new DelegateSubnetCallback(SubnetCallback);
            iConfigurationChangedCallback = new DelegateConfigurationChangedCallback(ConfigurationChangedCallback);
            iReceiverList = new List<Receiver>();
            iSubnetList = new List<Subnet>();
            iHandle = SongcasterCreate(aDomain, aSubnet, aChannel, aTtl, aMulticast, aEnabled, aPreset, iReceiverCallback, IntPtr.Zero, iSubnetCallback, IntPtr.Zero, iConfigurationChangedCallback, IntPtr.Zero, aManufacturer, aManufacturerUrl, aModelUrl);

            if (iHandle == IntPtr.Zero)
            {
                throw (new SongcasterError());
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

        private void ConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcaster)
        {
            iConfigurationChangedHandler.ConfigurationChanged(this);
        }

        public uint Subnet()
        {
            return (SongcasterSubnet(iHandle));
        }

        public uint Channel()
        {
            return (SongcasterChannel(iHandle));
        }

        public uint Ttl()
        {
            return (SongcasterTtl(iHandle));
        }

        public bool Multicast()
        {
            return (SongcasterMulticast(iHandle));
        }

        public bool Enabled()
        {
            return (SongcasterEnabled(iHandle));
        }

        public uint Preset()
        {
            return (SongcasterPreset(iHandle));
        }

        public void SetSubnet(uint aValue)
        {
            SongcasterSetSubnet(iHandle, aValue);
        }

        public void SetChannel(uint aValue)
        {
            SongcasterSetChannel(iHandle, aValue);
        }

        public void SetTtl(uint aValue)
        {
            SongcasterSetTtl(iHandle, aValue);
        }

        public void SetMulticast(bool aValue)
        {
            SongcasterSetMulticast(iHandle, aValue);
        }

        public void SetEnabled(bool aValue)
        {
            SongcasterSetEnabled(iHandle, aValue);
        }

        public void SetPreset(uint aValue)
        {
            SongcasterSetPreset(iHandle, aValue);
        }

        public unsafe void SetTrack(IntPtr aHandle, string aUri, string aMetadata, long aSamplesTotal, long aSampleStart)
        {
            IntPtr uri = Marshal.StringToHGlobalAnsi(aUri);
            IntPtr metadata = Marshal.StringToHGlobalAnsi(aMetadata);
            SongcasterSetTrack(iHandle, (char*)uri, (char*)metadata, aSamplesTotal, aSampleStart);
            Marshal.FreeHGlobal(uri);
            Marshal.FreeHGlobal(metadata);
        }

        public unsafe void SetMetatext(IntPtr aHandle, string aValue)
        {
            IntPtr value = Marshal.StringToHGlobalAnsi(aValue);
            SongcasterSetMetatext(iHandle, (char*)value);
            Marshal.FreeHGlobal(value);
        }

        public void RefreshReceivers()
        {
            SongcasterRefreshReceivers(iHandle);
        }

        public void Dispose()
        {
            SongcasterDestroy(iHandle);
        }

        private IReceiverHandler iReceiverHandler;
        private ISubnetHandler iSubnetHandler;
        private IConfigurationChangedHandler iConfigurationChangedHandler;
        private IntPtr iHandle;
        private List<Receiver> iReceiverList;
        private List<Subnet> iSubnetList;
        private DelegateReceiverCallback iReceiverCallback;
        private DelegateSubnetCallback iSubnetCallback;
        private DelegateConfigurationChangedCallback iConfigurationChangedCallback;
    }
} // namespace OpenHome.Songcaster