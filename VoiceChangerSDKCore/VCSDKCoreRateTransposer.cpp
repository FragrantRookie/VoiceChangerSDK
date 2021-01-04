////  VCSDKCoreRateTransposer.cpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/21.
//  
//

#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "VCSDKCoreRateTransposer.hpp"
#include "VCSDKCoreInterpolateLinear.hpp"
#include "VCSDKCoreInterpolateCubic.hpp"
#include "VCSDKCoreInterpolateShannon.hpp"
#include "VCSDKCoreAAFilter.hpp"

using namespace vcsdkcore;


// Define default interpolation algorithm here
VCSDKCoreTransposerBase::ALGORITHM VCSDKCoreTransposerBase::algorithm = VCSDKCoreTransposerBase::CUBIC;


// Constructor
VCSDKCoreRateTransposer::VCSDKCoreRateTransposer() : VCSDKCoreFIFOProcessor(&outputBuffer)
{
    bUseAAFilter = TRUE;

    // Instantiates the anti-alias filter
    pAAFilter = new VCSDKCoreAAFilter(64);
    pTransposer = VCSDKCoreTransposerBase::newInstance();
}



VCSDKCoreRateTransposer::~VCSDKCoreRateTransposer()
{
    delete pAAFilter;
    delete pTransposer;
}



/// Enables/disables the anti-alias filter. Zero to disable, nonzero to enable
void VCSDKCoreRateTransposer::enableAAFilter(BOOL newMode)
{
    bUseAAFilter = newMode;
}


/// Returns nonzero if anti-alias filter is enabled.
BOOL VCSDKCoreRateTransposer::isAAFilterEnabled() const
{
    return bUseAAFilter;
}


VCSDKCoreAAFilter *VCSDKCoreRateTransposer::getAAFilter()
{
    return pAAFilter;
}



// Sets new target iRate. Normal iRate = 1.0, smaller values represent slower
// iRate, larger faster iRates.
void VCSDKCoreRateTransposer::setRate(float newRate)
{
    double fCutoff;

    pTransposer->setRate(newRate);

    // design a new anti-alias filter
    if (newRate > 1.0f)
    {
        fCutoff = 0.5f / newRate;
    }
    else
    {
        fCutoff = 0.5f * newRate;
    }
    pAAFilter->setCutoffFreq(fCutoff);
}


// Adds 'nSamples' pcs of samples from the 'samples' memory position into
// the input of the object.
void VCSDKCoreRateTransposer::putSamples(const SAMPLETYPE *samples, uint nSamples)
{
    processSamples(samples, nSamples);
}


// Transposes sample rate by applying anti-alias filter to prevent folding.
// Returns amount of samples returned in the "dest" buffer.
// The maximum amount of samples that can be returned at a time is set by
// the 'set_returnBuffer_size' function.
void VCSDKCoreRateTransposer::processSamples(const SAMPLETYPE *src, uint nSamples)
{
    uint count;

    if (nSamples == 0) return;

    // Store samples to input buffer
    inputBuffer.putSamples(src, nSamples);

    // If anti-alias filter is turned off, simply transpose without applying
    // the filter
    if (bUseAAFilter == FALSE)
    {
        count = pTransposer->transpose(outputBuffer, inputBuffer);
        return;
    }

    assert(pAAFilter);

    // Transpose with anti-alias filter
    if (pTransposer->rate < 1.0f)
    {
        // If the parameter 'Rate' value is smaller than 1, first transpose
        // the samples and then apply the anti-alias filter to remove aliasing.

        // Transpose the samples, store the result to end of "midBuffer"
        pTransposer->transpose(midBuffer, inputBuffer);

        // Apply the anti-alias filter for transposed samples in midBuffer
        pAAFilter->evaluate(outputBuffer, midBuffer);
    }
    else
    {
        // If the parameter 'Rate' value is larger than 1, first apply the
        // anti-alias filter to remove high frequencies (prevent them from folding
        // over the lover frequencies), then transpose.

        // Apply the anti-alias filter for samples in inputBuffer
        pAAFilter->evaluate(midBuffer, inputBuffer);

        // Transpose the AA-filtered samples in "midBuffer"
        pTransposer->transpose(outputBuffer, midBuffer);
    }
}


// Sets the number of channels, 1 = mono, 2 = stereo
void VCSDKCoreRateTransposer::setChannels(int nChannels)
{
    assert(nChannels > 0);

    if (pTransposer->numChannels == nChannels) return;
    pTransposer->setChannels(nChannels);

    inputBuffer.setChannels(nChannels);
    midBuffer.setChannels(nChannels);
    outputBuffer.setChannels(nChannels);
}


// Clears all the samples in the object
void VCSDKCoreRateTransposer::clear()
{
    outputBuffer.clear();
    midBuffer.clear();
    inputBuffer.clear();
}


// Returns nonzero if there aren't any samples available for outputting.
int VCSDKCoreRateTransposer::isEmpty() const
{
    int res;

    res = VCSDKCoreFIFOProcessor::isEmpty();
    if (res == 0) return 0;
    return inputBuffer.isEmpty();
}


//////////////////////////////////////////////////////////////////////////////
//
// TransposerBase - Base class for interpolation
//

// static function to set interpolation algorithm
void VCSDKCoreTransposerBase::setAlgorithm(VCSDKCoreTransposerBase::ALGORITHM a)
{
    VCSDKCoreTransposerBase::algorithm = a;
}


// Transposes the sample rate of the given samples using linear interpolation.
// Returns the number of samples returned in the "dest" buffer
int VCSDKCoreTransposerBase::transpose(VCSDKCoreFIFOSampleBuffer &dest, VCSDKCoreFIFOSampleBuffer &src)
{
    int numSrcSamples = src.numSamples();
    int sizeDemand = (int)((float)numSrcSamples / rate) + 8;
    int numOutput;
    SAMPLETYPE *psrc = src.ptrBegin();
    SAMPLETYPE *pdest = dest.ptrEnd(sizeDemand);

#ifndef USE_MULTICH_ALWAYS
    if (numChannels == 1)
    {
        numOutput = transposeMono(pdest, psrc, numSrcSamples);
    }
    else if (numChannels == 2)
    {
        numOutput = transposeStereo(pdest, psrc, numSrcSamples);
    }
    else
#endif // USE_MULTICH_ALWAYS
    {
        assert(numChannels > 0);
        numOutput = transposeMulti(pdest, psrc, numSrcSamples);
    }
    dest.putSamples(numOutput);
    src.receiveSamples(numSrcSamples);
    return numOutput;
}


VCSDKCoreTransposerBase::VCSDKCoreTransposerBase()
{
    numChannels = 0;
    rate = 1.0f;
}


VCSDKCoreTransposerBase::~VCSDKCoreTransposerBase()
{
}


void VCSDKCoreTransposerBase::setChannels(int channels)
{
    numChannels = channels;
    resetRegisters();
}


void VCSDKCoreTransposerBase::setRate(float newRate)
{
    rate = newRate;
}


// static factory function
VCSDKCoreTransposerBase *VCSDKCoreTransposerBase::newInstance()
{
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
    // Notice: For integer arithmetics support only linear algorithm (due to simplest calculus)
    return ::new InterpolateLinearInteger;
#else
    switch (algorithm)
    {
        case LINEAR:
            return new InterpolateLinearFloat;

        case CUBIC:
            return new InterpolateCubic;

        case SHANNON:
            return new InterpolateShannon;

        default:
            assert(false);
            return NULL;
    }
#endif
}














