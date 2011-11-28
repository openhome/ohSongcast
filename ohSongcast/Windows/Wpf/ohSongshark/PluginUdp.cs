using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

using PcapDotNet.Core;

namespace ohSongshark
{
    class PluginUdp : Plugin
    {
        public PluginUdp()
            : base("Udp")
        {
        }

        public override void Run(PacketDevice aDevice)
        {
            using (PacketCommunicator communicator = aDevice.Open(100, PacketDeviceOpenAttributes.Promiscuous, 1000))
            {
                // Compile and set the filter
                communicator.SetFilter("udp");

                // Put the interface in statstics mode
                communicator.Mode = PacketCommunicatorMode.Statistics;

                // Start the main loop
                communicator.ReceiveStatistics(0, StatisticsHandler);
            }
        }

        private void StatisticsHandler(PacketSampleStatistics statistics)
        {
            // Current sample time
            DateTime currentTimestamp = statistics.Timestamp;

            // Previous sample time
            DateTime previousTimestamp = iLastTimestamp;

            // Set _lastTimestamp for the next iteration
            iLastTimestamp = currentTimestamp;

            // If there wasn't a previous sample than skip this iteration (it's the first iteration)
            if (previousTimestamp == DateTime.MinValue)
                return;

            // Calculate the delay from the last sample
            double delayInSeconds = (currentTimestamp - previousTimestamp).TotalSeconds;

            // Calculate bits per second
            double bitsPerSecond = statistics.AcceptedBytes * 8 / delayInSeconds;

            // Calculate packets per second
            double packetsPerSecond = statistics.AcceptedPackets / delayInSeconds;

            // Print timestamp and samples
            Console.WriteLine(statistics.Timestamp + " BPS: " + bitsPerSecond + " PPS: " + packetsPerSecond);
        }

        DateTime iLastTimestamp;

    }
}
