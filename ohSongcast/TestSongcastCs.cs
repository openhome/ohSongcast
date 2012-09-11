using System;
using System.Net;

using OpenHome.Songcast;

    class Program : IReceiverHandler, ISubnetHandler, IConfigurationChangedHandler, IMessageHandler
    {
        public static void Main(string[] args)
        {
            Program program = new Program();

            program.Run();
        }

        public void Run()
        {
            bool enabled = true;

            try
            {
                Songcast songcast = new Songcast("av.openhome.org", 522, 1, 1, 100, false, enabled, 99, this, this, this, this, "OpenHome", "http://www.openhome.org", "http://www.openhome.org", new byte[] { }, String.Empty);

                while (true)
                {
                    ConsoleKeyInfo key = Console.ReadKey(true);

                    if (key.KeyChar == 'q')
                    {
                        break;
                    }

                    if (key.KeyChar == 'a')
                    {
                        songcast.SetSubnet(43200);
                    }

                    if (key.KeyChar == 'e')
                    {
                        if (enabled)
                        {
                            enabled = false;
                        }
                        else
                        {
                            enabled = true;
                        }

                        songcast.SetEnabled(enabled);

                        continue;
                    }
                }

                songcast.Dispose();
            }
            catch (SongcastError e)
            {
                Console.WriteLine(e.Message);
            }
        }

        public void ConfigurationChanged(IConfiguration aConfiguration)
        {
            Console.WriteLine("Configuration changed: channel={0}, ttl={1}, multicast={2}, enabled={3}", aConfiguration.Channel(), aConfiguration.Ttl(), aConfiguration.Multicast(), aConfiguration.Enabled());
        }

        public void ReceiverAdded(IReceiver aReceiver)
        {
            Console.WriteLine("Added   {0}:{1}:{2}:{3}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status);
        }

        public void ReceiverChanged(IReceiver aReceiver)
        {
            Console.WriteLine("Changed {0}:{1}:{2}:{3}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status);
        }

        public void ReceiverRemoved(IReceiver aReceiver)
        {
            Console.WriteLine("Removed {0}:{1}:{2}:{3}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status);
        }

        public void ReceiverVolumeControlChanged(IReceiver aReceiver)
        {
            Console.WriteLine("VolumeControlChanged {0}:{1}:{2} {3} {4}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status, aReceiver.HasVolumeControl);
        }

        public void ReceiverVolumeChanged(IReceiver aReceiver)
        {
            Console.WriteLine("ReceiverVolumeChanged {0}:{1}:{2} {3} {4}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status, aReceiver.Volume);
        }

        public void ReceiverMuteChanged(IReceiver aReceiver)
        {
            Console.WriteLine("ReceiverMuteChanged {0}:{1}:{2} {3} {4}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status, aReceiver.Mute);
        }

        public void ReceiverVolumeLimitChanged(IReceiver aReceiver)
        {
            Console.WriteLine("ReceiverVolumeLimitChanged {0}:{1}:{2} {3} {4}", aReceiver.Room, aReceiver.Group, aReceiver.Name, aReceiver.Status, aReceiver.VolumeLimit);
        }

        public void SubnetAdded(ISubnet aSubnet)
        {
            Console.WriteLine("Added   {0}:{1}", aSubnet.Address, aSubnet.AdapterName);
        }

        public void SubnetChanged(ISubnet aSubnet)
        {
            Console.WriteLine("Changed {0}:{1}", aSubnet.Address, aSubnet.AdapterName);
        }

        public void SubnetRemoved(ISubnet aSubnet)
        {
            Console.WriteLine("Removed {0}:{1}", aSubnet.Address, aSubnet.AdapterName);
        }

        public void Message(string aMessage)
        {
            Console.WriteLine("Message: " + aMessage);
        }
    }
