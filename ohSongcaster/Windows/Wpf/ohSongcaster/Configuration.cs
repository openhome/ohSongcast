using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using System.Xml;
using System.Xml.Serialization;
using System.IO;

namespace OpenHome.Songcaster
{
    [XmlRoot("Configuration")]

    public class Configuration
    {
        public static Configuration Load()
        {
            XmlSerializer xml = new XmlSerializer(typeof(Configuration));

            string common = Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData);
            string folder = Path.Combine(common, "ohSongcaster");

            if (!Directory.Exists(folder))
            {
                Directory.CreateDirectory(folder);
            }

            string path = Path.Combine(folder, "Configuration.xml");

            if (!File.Exists(path))
            {
                return (New(path));
            }

            try
            {
                using (TextReader reader = new StreamReader(path))
                {
                    Configuration configuration = (Configuration)xml.Deserialize(reader);
                    configuration.SetPath(path);
                    return (configuration);
                }
            }
            catch (Exception)
            {
                return (New(path));
            }

        }

        private static Configuration New(string aPath)
        {
            Configuration configuration = new Configuration();
            configuration.Subnet = 0;
            configuration.Multicast = false;
            configuration.Channel = (uint)(new Random().Next(65535) + 1);
            configuration.Ttl = 1;
            configuration.Latency = 100;
            configuration.Preset = 0;
            configuration.SetPath(aPath);
            configuration.Save();
            return (configuration);
        }

        public void Save()
        {
            XmlSerializer xml = new XmlSerializer(typeof(Configuration));

            try
            {
                using (TextWriter writer = new StreamWriter(iPath))
                {

                    xml.Serialize(writer, this);
                }
            }
            catch (Exception e)
            {
                throw (new ApplicationException("Unable to save configuration file", e));
            }
        }

        private void SetPath(string aPath)
        {
            iPath = aPath;
        }

        private string iPath;

        [XmlElement("Subnet")]

        public uint Subnet
        {
            get
            {
                return (iSubnet);
            }
            set
            {
                iSubnet = value;
            }
        }

        [XmlElement("Multicast")]

        public bool Multicast
        {
            get
            {
                return (iMulticast);
            }
            set
            {
                iMulticast = value;
            }
        }

        [XmlElement("MulticastChannel")]

        public uint Channel
        {
            get
            {
                return (iMulticastChannel);
            }
            set
            {
                iMulticastChannel = value;
            }
        }

        [XmlElement("Ttl")]

        public uint Ttl
        {
            get
            {
                return (iTtl);
            }
            set
            {
                iTtl = value;
            }
        }

        [XmlElement("Latency")]

        public uint Latency
        {
            get
            {
                return (iLatency);
            }
            set
            {
                iLatency = value;
            }
        }

        [XmlElement("Preset")]

        public uint Preset
        {
            get
            {
                return (iPreset);
            }
            set
            {
                iPreset = value;
            }
        }

        [XmlElement("Enabled")]

        public bool Enabled
        {
            get
            {
                return (iEnabled);
            }
            set
            {
                iEnabled = value;
            }
        }


        private uint iSubnet;
        private bool iMulticast;
        private uint iMulticastChannel;
        private uint iTtl;
        private uint iLatency;
        private uint iPreset;
        private bool iEnabled;
    }


}