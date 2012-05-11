#ifndef HEADER_AUDIODEVICEINTERFACE
#define HEADER_AUDIODEVICEINTERFACE


enum EDriverMethod
{
    eOpen,              // ()
    eClose,             // ()
    eSetActive,         // (uint64_t aActive)
    eSetEndpoint,       // (uint64_t aIpAddress, uint64_t aPort, uint64_t aAdapter)
    eSetTtl,            // (uint64_t aTtl)
    eSetLatencyMs,      // (uint64_t aLatencyMs)
    eResend,            // (uint64_t aCount, byte[ResendMaxBytes] aFrames)
    eNumDriverMethods
};


const uint64_t ResendMaxBytes = 1024;



#endif // HEADER_AUDIODEVICEINTERFACE



