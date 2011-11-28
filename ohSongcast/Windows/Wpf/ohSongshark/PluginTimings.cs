using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using PcapDotNet.Core;

namespace ohSongshark
{
    class PluginTimings : Plugin
    {
        public PluginTimings()
            : base("Timings")
        {
        }

        public override void Run(PacketDevice aDevice)
        {
            throw new NotImplementedException();
        }
    }
}
