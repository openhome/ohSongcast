using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using PcapDotNet.Core;

namespace ohSongshark
{
    public interface IPlugin
    {
        string Name { get; }
        void Run(PacketDevice aDevice);
    }

    public abstract class Plugin : IPlugin
    {
        private string iName;

        protected Plugin(string aName)
        {
            iName = aName;
        }

        public string Name
        {
            get
            {
                return (iName);
            }
        }

        public abstract void Run(PacketDevice aDevice);

        public override string ToString()
        {
            return (iName);
        }

    }
}
