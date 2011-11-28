using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Net;
using System.Collections.Generic;

namespace OpenHome.Songcast
{
    public class SongcastError : Exception
    {
        internal SongcastError()
            : base("Songcast audio driver not installed")
        {
        }
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
        [DllImport("ohSongcast.dll")]
        static extern IntPtr ReceiverUdn(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern IntPtr ReceiverRoom(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern IntPtr ReceiverGroup(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern IntPtr ReceiverName(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern uint ReceiverStatus(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void ReceiverPlay(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void ReceiverStop(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void ReceiverStandby(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void ReceiverAddRef(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
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
        [DllImport("ohSongcast.dll")]
        static extern IntPtr SubnetAddress(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern IntPtr SubnetAdapterName(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void SubnetAddRef(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void SubnetRemoveRef(IntPtr aHandle);

        public Subnet(IntPtr aSubnet)
        {
            iSubnet = aSubnet;
            iAddress = (uint)SubnetAddress(iSubnet);
            iAdapterName = Marshal.PtrToStringAnsi(SubnetAdapterName(iSubnet));
            SubnetAddRef(iSubnet);
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

    public class Songcast : IDisposable, IConfiguration
    {
        private enum ECallbackType
        {
            eAdded,
            eChanged,
            eRemoved
        }

        private delegate void DelegateReceiverCallback(IntPtr aPtr, ECallbackType aType, IntPtr aReceiver);
        private delegate void DelegateSubnetCallback(IntPtr aPtr, ECallbackType aType, IntPtr aSubnet);
        private delegate void DelegateConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcast);

        [DllImport("ohSongcast.dll")]
        static extern unsafe IntPtr SongcastCreate(string aDomain, uint aSubnet, uint aChannel, uint aTtl, uint aLatency, bool aMulticast, bool aEnabled, uint aPreset, DelegateReceiverCallback aReceiverCallback, IntPtr aReceiverPtr, DelegateSubnetCallback aSubnetCallback, IntPtr aSubnetPtr, DelegateConfigurationChangedCallback aConfigurationChangedCallback, IntPtr aConfigurationChangedPtr, string aManufacturer, string aManufacturerUrl, string aModelUrl, byte[] aImagePtr, int aImageBytes, string aMimeType);

        [DllImport("ohSongcast.dll")]
        static extern uint SongcastSubnet(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern uint SongcastChannel(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern uint SongcastTtl(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern uint SongcastLatency(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern bool SongcastMulticast(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern bool SongcastEnabled(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern uint SongcastPreset(IntPtr aHandle);
        
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetSubnet(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetChannel(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetTtl(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetLatency(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetMulticast(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetEnabled(IntPtr aHandle, bool aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastSetPreset(IntPtr aHandle, uint aValue);
        [DllImport("ohSongcast.dll")]
        static extern unsafe void SongcastSetTrack(IntPtr aHandle, char* aUri, char* aMetadata, long aSamplesTotal, long aSampleStart);
        [DllImport("ohSongcast.dll")]
        static extern unsafe void SongcastSetMetatext(IntPtr aHandle, char* aValue);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastRefreshReceivers(IntPtr aHandle);
        [DllImport("ohSongcast.dll")]
        static extern void SongcastDestroy(IntPtr aHandle);

        public unsafe Songcast(string aDomain, uint aSubnet, uint aChannel, uint aTtl, uint aLatency, bool aMulticast, bool aEnabled, uint aPreset, IReceiverHandler aReceiverHandler, ISubnetHandler aSubnetHandler, IConfigurationChangedHandler aConfigurationChangedHandler, string aManufacturer, string aManufacturerUrl, string aModelUrl, byte[] aImage, string aMimeType)
        {
            iReceiverHandler = aReceiverHandler;
            iSubnetHandler = aSubnetHandler;
            iConfigurationChangedHandler = aConfigurationChangedHandler;
            iReceiverCallback = new DelegateReceiverCallback(ReceiverCallback);
            iSubnetCallback = new DelegateSubnetCallback(SubnetCallback);
            iConfigurationChangedCallback = new DelegateConfigurationChangedCallback(ConfigurationChangedCallback);
            iReceiverList = new List<Receiver>();
            iSubnetList = new List<Subnet>();

            iHandle = SongcastCreate(aDomain, aSubnet, aChannel, aTtl, aLatency, aMulticast, aEnabled, aPreset, iReceiverCallback, IntPtr.Zero, iSubnetCallback, IntPtr.Zero, iConfigurationChangedCallback, IntPtr.Zero, aManufacturer, aManufacturerUrl, aModelUrl, aImage, aImage.Length, aMimeType);

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

        private void ConfigurationChangedCallback(IntPtr aPtr, IntPtr aSongcast)
        {
            iConfigurationChangedHandler.ConfigurationChanged(this);
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
            IntPtr uri = Marshal.StringToHGlobalAnsi(aUri);
            IntPtr metadata = Marshal.StringToHGlobalAnsi(aMetadata);
            SongcastSetTrack(iHandle, (char*)uri, (char*)metadata, aSamplesTotal, aSampleStart);
            Marshal.FreeHGlobal(uri);
            Marshal.FreeHGlobal(metadata);
        }

        public unsafe void SetMetatext(IntPtr aHandle, string aValue)
        {
            IntPtr value = Marshal.StringToHGlobalAnsi(aValue);
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
} // namespace OpenHome.Songcast
