
#include <IOKit/IOLib.h>


// implementation of the floating point parts of the driver

IOReturn AudioEngineClipOutputSamples(const void* aMixBuffer, void* aSampleBuffer, UInt32 aFirstSampleFrame, UInt32 aNumSampleFrames, UInt32 aNumChannels, UInt32 aBitDepth)
{
    // The audio in the input buffer is always in floating point format in range [-1,1]
    uint8_t* inBuffer = ((uint8_t*)aMixBuffer) + (aFirstSampleFrame * aNumChannels * sizeof(float));
    uint8_t* outBuffer = ((uint8_t*)aSampleBuffer) + (aFirstSampleFrame * aNumChannels * aBitDepth / 8);
    
    // convert the floating point audio into integer PCM
    int numSamples = aNumSampleFrames * aNumChannels;
    
    // calculate the maximum integer value for the sample - the output sample
    // will be in the range [-maxVal, maxVal-1]
    int maxVal = 1 << (aBitDepth - 1);
    float maxFloatVal = 1.0f - (1.0f / maxVal);
    int outSampleBytes = aBitDepth / 8;
    
    while (numSamples > 0)
    {
        float inSample = *((float*)inBuffer);
        
        // clamp the input sample
        if (inSample > maxFloatVal) {
            inSample = maxFloatVal;
        }
        else if (inSample < -1.0f) {
            inSample = -1.0f;
        }
        
        // convert to integer in range [maxVal-1, -maxVal]
        int32_t outSample = (int32_t)(inSample * maxVal);
        
        // copy the output sample into the buffer in big endian format
        for (int i=0 ; i<outSampleBytes ; i++)
        {
            outBuffer[i] = (outSample >> (8*(outSampleBytes - i - 1))) & 0xff;
        }
        
        inBuffer += sizeof(float);
        outBuffer += outSampleBytes;
        numSamples--;
    }
    
    return kIOReturnSuccess;
}


