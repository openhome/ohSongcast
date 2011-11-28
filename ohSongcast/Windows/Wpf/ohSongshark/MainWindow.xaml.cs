using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

using System.Net;
using System.Threading;

using PcapDotNet.Core;
using PcapDotNet.Packets;
using PcapDotNet.Packets.IpV4;


namespace ohSongshark
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        IList<LivePacketDevice> iDevices;
        List<IPlugin> iPlugins;
        List<string> iAdapters;

        IPEndPoint iEndpoint;

        Semaphore iStart;
        ManualResetEvent iStarted;
        ManualResetEvent iStopped;

        Thread iThread;

        Mutex iMutex;
        bool iStop;
        PacketDevice iRunningDevice;
        IPEndPoint iRunningEndpoint;
        IpV4Address iRunningAddress;
        Timer iTimer;

        public MainWindow()
        {
            InitializeComponent();

            iMutex = new Mutex();
            iStart = new Semaphore(0, 1);
            iStarted = new ManualResetEvent(false);
            iStopped = new ManualResetEvent(false);
            iThread = new Thread(Run);
            iThread.Start();

            // Send anonymous statistics about the usage of Pcap.Net
            PcapDotNet.Analysis.PcapDotNetAnalysis.OptIn = true;

            // Retrieve the device list from the local machine
            iDevices = LivePacketDevice.AllLocalMachine;

            if (iDevices.Count == 0)
            {
                throw (new ApplicationException("No interfaces found! Make sure WinPcap is installed."));
            }

            iAdapters = new List<string>();

            foreach (LivePacketDevice device in iDevices)
            {
                string description = device.Description;
                string[] parts = description.Split(new char[] { '\'' });
                iAdapters.Add(parts[1]);
            }

            comboAdapter.ItemsSource = iAdapters;

            iPlugins = new List<IPlugin>();
            iPlugins.Add(new PluginUdp());
            iPlugins.Add(new PluginTimings());

            comboPlugin.ItemsSource = iPlugins;

            buttonStart.IsEnabled = true;
            buttonStop.IsEnabled = false;

            textEndpoint.TextChanged += new TextChangedEventHandler(EventTextEndpointTextChanged);
            buttonStart.Click += new RoutedEventHandler(EventButtonStartClick);
            buttonStop.Click += new RoutedEventHandler(EventButtonStopClick);

            iTimer = new Timer(EventTimerExpired, null, 0, 1000);
        }

        private void EventTimerExpired(object aState)
        {
            Dispatcher.BeginInvoke(new Action(Refresh));
        }

        private void Refresh()
        {
            iMutex.WaitOne();
            uint count = iPacketCount;
            TimeSpan min = iPacketMinTime;
            TimeSpan max = iPacketMaxTime;
            iMutex.ReleaseMutex();
            textCount.Text = count.ToString();
            textMin.Text = min.TotalMilliseconds.ToString();
            textMax.Text = max.TotalMilliseconds.ToString();
        }

        public class SongcastDatagram : PcapDotNet.Packets.Datagram
        {
            public SongcastDatagram(byte[] aBytes)
                : base(aBytes)
            {
            }

            public bool Halt
            {
                get
                {
                    byte[] bytes = ReadBytes(51, 1);
                    uint flag = bytes[0];
                    return ((flag & 0x01) == 0x01);
                }
            }

            public uint Frame
            {
                get
                {
                    return (ReadUInt(54, Endianity.Big));
                }
            }
        }

        private void Run()
        {
            while (true)
            {
                iMutex.WaitOne();
                iStop = false;
                iMutex.ReleaseMutex();

                iStart.WaitOne();
                iStopped.Reset();
                iStarted.Set();

                iHalt = false;

                using (PacketCommunicator communicator = iRunningDevice.Open(65536, PacketDeviceOpenAttributes.MaximumResponsiveness, 1000))
                {
                    Console.WriteLine("Listening on " + iRunningDevice.Description + "...");

                    try
                    {
                        communicator.ReceivePackets(0, PacketHandler);
                    }
                    catch (Exception)
                    {
                    }
                }

                iStarted.Reset();
                iStopped.Set();
            }
        }

        private uint iPacketCount;
        private TimeSpan iPacketMinTime;
        private TimeSpan iPacketMaxTime;
        private DateTime iLastTimestamp;
        private uint iLastFrame;

        void ResetStats()
        {
            iPacketCount = 0;
            iPacketMinTime = new TimeSpan(1, 0, 0);
            iPacketMaxTime = new TimeSpan(0);
        }

        private bool iHalt;

        // 10.2.9.32:51974
        // Callback function invoked by Pcap.Net for every incoming packet
        private void PacketHandler(Packet aPacket)
        {
            iMutex.WaitOne();

            if (iStop)
            {
                iMutex.ReleaseMutex();
                throw (new Exception());
            }
            
            if (aPacket.Ethernet.EtherType == PcapDotNet.Packets.Ethernet.EthernetType.IpV4)
            {
                IpV4Datagram ipv4 = aPacket.Ethernet.IpV4;
                
                if (ipv4.Destination == iRunningAddress)
                {
                    if (ipv4.Protocol == IpV4Protocol.Udp)
                    {
                        var udp = ipv4.Udp;

                        if (udp.DestinationPort == iRunningEndpoint.Port)
                        {
                            SongcastDatagram datagram = new SongcastDatagram(aPacket.Buffer);

                            if (iPacketCount != 0)
                            {
                                if (!iHalt)
                                {
                                    TimeSpan span = aPacket.Timestamp.Subtract(iLastTimestamp);

                                    if (span < iPacketMinTime)
                                    {
                                        iPacketMinTime = span;
                                    }
                                    if (span > iPacketMaxTime)
                                    {
                                        iPacketMaxTime = span;
                                    }
                                }

                                if (datagram.Frame != iLastFrame + 1)
                                {
                                    Console.WriteLine("LOST FRAME before" + datagram.Frame);
                                }


                                iHalt = datagram.Halt;
                            }

                            iPacketCount++;
                            iLastTimestamp = aPacket.Timestamp;
                            iLastFrame = datagram.Frame;
                        }
                    }
                }
            }

            iMutex.ReleaseMutex();
        }

        void EventTextEndpointTextChanged(object sender, TextChangedEventArgs e)
        {
            if (textEndpoint.Text.Contains(':'))
            {
                string[] parts = textEndpoint.Text.Split(new char[] { ':' }, 2);

                if (parts.Length == 2)
                {
                    IPAddress address;

                    if (IPAddress.TryParse(parts[0], out address))
                    {
                        uint port;

                        if (uint.TryParse(parts[1], out port))
                        {
                            iEndpoint = new IPEndPoint(address, (int)port);
                            return;
                        }
                    }
                }
            }

            iEndpoint = null;
        }

        void EventButtonStartClick(object sender, RoutedEventArgs e)
        {
            if (comboAdapter.SelectedIndex < 0)
            {
                MessageBox.Show("Adapter not selected");
                return;
            }

            if (iEndpoint == null)
            {
                MessageBox.Show("Endpoint not specified");
                return;
            }

            iMutex.WaitOne();
            iRunningDevice = iDevices[comboAdapter.SelectedIndex];
            iRunningEndpoint = iEndpoint;
            iRunningAddress = new IpV4Address(iRunningEndpoint.Address.ToString());
            ResetStats();

            buttonStart.IsEnabled = false;
            buttonStop.IsEnabled = true;
            iMutex.ReleaseMutex();
            iStart.Release();
        }

        void EventButtonStopClick(object sender, RoutedEventArgs e)
        {
            iMutex.WaitOne();
            iStop = true;
            iMutex.ReleaseMutex();
            iStopped.WaitOne();
            buttonStop.IsEnabled = false;
            buttonStart.IsEnabled = true;
        }

        protected override void  OnClosing(System.ComponentModel.CancelEventArgs e)
        {
            iMutex.WaitOne();
            iStop = true;
            iMutex.ReleaseMutex();
            iStopped.WaitOne();
            iThread.Abort();
            iThread.Join();
            base.OnClosing(e);
        }
    }
}


