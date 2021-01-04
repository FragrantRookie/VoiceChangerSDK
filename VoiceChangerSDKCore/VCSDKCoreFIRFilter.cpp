////  VCSDKCoreFIRFilter.cpp
//  VoiceChanger
//
//  Created on 2020/12/15.
//  
//

#include <memory.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "VCSDKCoreFIRFilter.hpp"
#include "VCSDKCoreCpu_detect.h"



using namespace vcsdkcore;

VCSDKCoreFIRFilter::VCSDKCoreFIRFilter() {
    resultDivFactor = 0;
    resultDivider = 0;
    length = 0;
    lengthDiv8 = 0;
    filterCoeffs = NULL;
}
VCSDKCoreFIRFilter::~VCSDKCoreFIRFilter () {
    delete [] filterCoeffs;
}

// Usual C-version of the filter routine for stereo sound
uint VCSDKCoreFIRFilter::evaluateFilterStereo(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples) const {
        
    uint i, j, end;
    LONG_SAMPLETYPE suml, sumr;
#ifdef SOUNDTOUCH_FLOAT_SAMPLES
    // when using floating point samples, use a scaler instead of a divider
    // because division is much slower operation than multiplying.
    double dScaler = 1.0 / (double)resultDivider;
#endif
    
    assert(length != 0);
    assert(src != NULL);
    assert(dest != NULL);
    assert(filterCoeffs != NULL);
    
    end = 2 * (numSamples - length);
    
    for (j = 0; j < end; j += 2)
    {
        const SAMPLETYPE *ptr;
        
        suml = sumr = 0;
        ptr = src + j;
        
        for (i = 0; i < length; i += 4)
        {
            // loop is unrolled by factor of 4 here for efficiency
            suml += ptr[2 * i + 0] * filterCoeffs[i + 0] +
            ptr[2 * i + 2] * filterCoeffs[i + 1] +
            ptr[2 * i + 4] * filterCoeffs[i + 2] +
            ptr[2 * i + 6] * filterCoeffs[i + 3];
            sumr += ptr[2 * i + 1] * filterCoeffs[i + 0] +
            ptr[2 * i + 3] * filterCoeffs[i + 1] +
            ptr[2 * i + 5] * filterCoeffs[i + 2] +
            ptr[2 * i + 7] * filterCoeffs[i + 3];
        }
        
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
        suml >>= resultDivFactor;
        sumr >>= resultDivFactor;
        // saturate to 16 bit integer limits
        suml = (suml < -32768) ? -32768 : (suml > 32767) ? 32767 : suml;
        // saturate to 16 bit integer limits
        sumr = (sumr < -32768) ? -32768 : (sumr > 32767) ? 32767 : sumr;
#else
        suml *= dScaler;
        sumr *= dScaler;
#endif // SOUNDTOUCH_INTEGER_SAMPLES
        dest[j] = (SAMPLETYPE)suml;
        dest[j + 1] = (SAMPLETYPE)sumr;
    }
    return numSamples - length;
    
}

// Usual C-version of the filter routine for mono sound
uint VCSDKCoreFIRFilter::evaluateFilterMono(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples) const
{
    uint i, j, end;
    LONG_SAMPLETYPE sum;
#ifdef SOUNDTOUCH_FLOAT_SAMPLES
    // when using floating point samples, use a scaler instead of a divider
    // because division is much slower operation than multiplying.
    double dScaler = 1.0 / (double)resultDivider;
#endif


    assert(length != 0);

    end = numSamples - length;
    for (j = 0; j < end; j ++)
    {
        sum = 0;
        for (i = 0; i < length; i += 4)
        {
            // loop is unrolled by factor of 4 here for efficiency
            sum += src[i + 0] * filterCoeffs[i + 0] +
                   src[i + 1] * filterCoeffs[i + 1] +
                   src[i + 2] * filterCoeffs[i + 2] +
                   src[i + 3] * filterCoeffs[i + 3];
        }
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
        sum >>= resultDivFactor;
        // saturate to 16 bit integer limits
        sum = (sum < -32768) ? -32768 : (sum > 32767) ? 32767 : sum;
#else
        sum *= dScaler;
#endif // SOUNDTOUCH_INTEGER_SAMPLES
        dest[j] = (SAMPLETYPE)sum;
        src ++;
    }
    return end;
}


uint VCSDKCoreFIRFilter::evaluateFilterMulti(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples, uint numChannels) const
{
    uint i, j, end, c;
    LONG_SAMPLETYPE *sum=(LONG_SAMPLETYPE*)alloca(numChannels*sizeof(*sum));
#ifdef SOUNDTOUCH_FLOAT_SAMPLES
    // when using floating point samples, use a scaler instead of a divider
    // because division is much slower operation than multiplying.
    double dScaler = 1.0 / (double)resultDivider;
#endif

    assert(length != 0);
    assert(src != NULL);
    assert(dest != NULL);
    assert(filterCoeffs != NULL);

    end = numChannels * (numSamples - length);

    for (c = 0; c < numChannels; c ++)
    {
        sum[c] = 0;
    }

    for (j = 0; j < end; j += numChannels)
    {
        const SAMPLETYPE *ptr;

        ptr = src + j;

        for (i = 0; i < length; i ++)
        {
            SAMPLETYPE coef=filterCoeffs[i];
            for (c = 0; c < numChannels; c ++)
            {
                sum[c] += ptr[0] * coef;
                ptr ++;
            }
        }
        
        for (c = 0; c < numChannels; c ++)
        {
#ifdef SOUNDTOUCH_INTEGER_SAMPLES
            sum[c] >>= resultDivFactor;
#else
            sum[c] *= dScaler;
#endif // SOUNDTOUCH_INTEGER_SAMPLES
            *dest = (SAMPLETYPE)sum[c];
            dest++;
            sum[c] = 0;
        }
    }
    return numSamples - length;
}


// Set filter coeffiecients and length.
//
// Throws an exception if filter length isn't divisible by 8
void VCSDKCoreFIRFilter::setCoefficients(const SAMPLETYPE *coeffs, uint newLength, uint uResultDivFactor)
{
    assert(newLength > 0);
    if (newLength % 8) ST_THROW_RT_ERROR("FIR filter length not divisible by 8");

    lengthDiv8 = newLength / 8;
    length = lengthDiv8 * 8;
    assert(length == newLength);

    resultDivFactor = uResultDivFactor;
    resultDivider = (SAMPLETYPE)::pow(2.0, (int)resultDivFactor);

    delete[] filterCoeffs;
    filterCoeffs = new SAMPLETYPE[length];
    memcpy(filterCoeffs, coeffs, length * sizeof(SAMPLETYPE));
}


uint VCSDKCoreFIRFilter::getLength() const
{
    return length;
}



// Applies the filter to the given sequence of samples.
//
// Note : The amount of outputted samples is by value of 'filter_length'
// smaller than the amount of input samples.
uint VCSDKCoreFIRFilter::evaluate(SAMPLETYPE *dest, const SAMPLETYPE *src, uint numSamples, uint numChannels) const
{
    assert(length > 0);
    assert(lengthDiv8 * 8 == length);

    if (numSamples < length) return 0;

#ifndef USE_MULTICH_ALWAYS
    if (numChannels == 1)
    {
        return evaluateFilterMono(dest, src, numSamples);
    }
    else if (numChannels == 2)
    {
        return evaluateFilterStereo(dest, src, numSamples);
    }
    else
#endif // USE_MULTICH_ALWAYS
    {
        assert(numChannels > 0);
        return evaluateFilterMulti(dest, src, numSamples, numChannels);
    }
}



// Operator 'new' is overloaded so that it automatically creates a suitable instance
// depending on if we've a MMX-capable CPU available or not.
void * VCSDKCoreFIRFilter::operator new(size_t s)
{
    // Notice! don't use "new FIRFilter" directly, use "newInstance" to create a new instance instead!
    ST_THROW_RT_ERROR("Error in FIRFilter::new: Don't use 'new FIRFilter', use 'newInstance' member instead!");
    return newInstance();
}


VCSDKCoreFIRFilter * VCSDKCoreFIRFilter::newInstance()
{
    uint uExtensions;

    uExtensions = detectCPUextensions();

    // Check if MMX/SSE instruction set extensions supported by CPU

#ifdef SOUNDTOUCH_ALLOW_MMX
    // MMX routines available only with integer sample types
    if (uExtensions & SUPPORT_MMX)
    {
        return ::new VCSDKCoreFIRFilterMMX;
    }
    else
#endif // SOUNDTOUCH_ALLOW_MMX

#ifdef SOUNDTOUCH_ALLOW_SSE
    if (uExtensions & SUPPORT_SSE)
    {
        // SSE support
        return ::new FIRFilterSSE;
    }
    else
#endif // SOUNDTOUCH_ALLOW_SSE

    {
        // ISA optimizations not supported, use plain C version
        return ::new VCSDKCoreFIRFilter;
    }
}





