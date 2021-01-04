////  VCSDKCoreFIFOSampleBuffer.cpp
//  VoiceChanger
//
//  Created on 2020/12/15.
//  
//

#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>


#include "VCSDKCoreFIFOSampleBuffer.hpp"

using namespace vcsdkcore;


// Constructor
VCSDKCoreFIFOSampleBuffer::VCSDKCoreFIFOSampleBuffer(int numChannels)
{
    assert(numChannels > 0);
    sizeInBytes = 0; // reasonable initial value
    buffer = NULL;
    bufferUnaligned = NULL;
    samplesInBuffer = 0;
    bufferPos = 0;
    channels = (uint)numChannels;
    ensureCapacity(32);     // allocate initial capacity
}


// destructor
VCSDKCoreFIFOSampleBuffer::~VCSDKCoreFIFOSampleBuffer()
{
    delete[] bufferUnaligned;
    bufferUnaligned = NULL;
    buffer = NULL;
}


// Sets number of channels, 1 = mono, 2 = stereo
void VCSDKCoreFIFOSampleBuffer::setChannels(int numChannels)
{
    uint usedBytes;

    assert(numChannels > 0);
    usedBytes = channels * samplesInBuffer;
    channels = (uint)numChannels;
    samplesInBuffer = usedBytes / channels;
}


// if output location pointer 'bufferPos' isn't zero, 'rewinds' the buffer and
// zeroes this pointer by copying samples from the 'bufferPos' pointer
// location on to the beginning of the buffer.
void VCSDKCoreFIFOSampleBuffer::rewind()
{
    if (buffer && bufferPos)
    {
        memmove(buffer, ptrBegin(), sizeof(SAMPLETYPE) * channels * samplesInBuffer);
        bufferPos = 0;
    }
}


// Adds 'numSamples' pcs of samples from the 'samples' memory position to
// the sample buffer.
void VCSDKCoreFIFOSampleBuffer::putSamples(const SAMPLETYPE *samples, uint nSamples)
{
    memcpy(ptrEnd(nSamples), samples, sizeof(SAMPLETYPE) * nSamples * channels);
    samplesInBuffer += nSamples;
}


// Increases the number of samples in the buffer without copying any actual
// samples.
//
// This function is used to update the number of samples in the sample buffer
// when accessing the buffer directly with 'ptrEnd' function. Please be
// careful though!
void VCSDKCoreFIFOSampleBuffer::putSamples(uint nSamples)
{
    uint req;

    req = samplesInBuffer + nSamples;
    ensureCapacity(req);
    samplesInBuffer += nSamples;
}


// Returns a pointer to the end of the used part of the sample buffer (i.e.
// where the new samples are to be inserted). This function may be used for
// inserting new samples into the sample buffer directly. Please be careful!
//
// Parameter 'slackCapacity' tells the function how much free capacity (in
// terms of samples) there _at least_ should be, in order to the caller to
// succesfully insert all the required samples to the buffer. When necessary,
// the function grows the buffer size to comply with this requirement.
//
// When using this function as means for inserting new samples, also remember
// to increase the sample count afterwards, by calling  the
// 'putSamples(numSamples)' function.
SAMPLETYPE *VCSDKCoreFIFOSampleBuffer::ptrEnd(uint slackCapacity)
{
    ensureCapacity(samplesInBuffer + slackCapacity);
    return buffer + samplesInBuffer * channels;
}


// Returns a pointer to the beginning of the currently non-outputted samples.
// This function is provided for accessing the output samples directly.
// Please be careful!
//
// When using this function to output samples, also remember to 'remove' the
// outputted samples from the buffer by calling the
// 'receiveSamples(numSamples)' function
SAMPLETYPE *VCSDKCoreFIFOSampleBuffer::ptrBegin()
{
    assert(buffer);
    return buffer + bufferPos * channels;
}


// Ensures that the buffer has enought capacity, i.e. space for _at least_
// 'capacityRequirement' number of samples. The buffer is grown in steps of
// 4 kilobytes to eliminate the need for frequently growing up the buffer,
// as well as to round the buffer size up to the virtual memory page size.
void VCSDKCoreFIFOSampleBuffer::ensureCapacity(uint capacityRequirement)
{
    SAMPLETYPE *tempUnaligned, *temp;

    if (capacityRequirement > getCapacity())
    {
        // enlarge the buffer in 4kbyte steps (round up to next 4k boundary)
        sizeInBytes = (capacityRequirement * channels * sizeof(SAMPLETYPE) + 4095) & (uint)-4096;
        assert(sizeInBytes % 2 == 0);
        tempUnaligned = new SAMPLETYPE[sizeInBytes / sizeof(SAMPLETYPE) + 16 / sizeof(SAMPLETYPE)];
        if (tempUnaligned == NULL)
        {
            ST_THROW_RT_ERROR("Couldn't allocate memory!\n");
        }
        // Align the buffer to begin at 16byte cache line boundary for optimal performance
        temp = (SAMPLETYPE *)SOUNDTOUCH_ALIGN_POINTER_16(tempUnaligned);
        if (samplesInBuffer)
        {
            memcpy(temp, ptrBegin(), samplesInBuffer * channels * sizeof(SAMPLETYPE));
        }
        delete[] bufferUnaligned;
        buffer = temp;
        bufferUnaligned = tempUnaligned;
        bufferPos = 0;
    }
    else
    {
        // simply rewind the buffer (if necessary)
        rewind();
    }
}


// Returns the current buffer capacity in terms of samples
uint VCSDKCoreFIFOSampleBuffer::getCapacity() const
{
    return sizeInBytes / (channels * sizeof(SAMPLETYPE));
}


// Returns the number of samples currently in the buffer
uint VCSDKCoreFIFOSampleBuffer::numSamples() const
{
    return samplesInBuffer;
}


// Output samples from beginning of the sample buffer. Copies demanded number
// of samples to output and removes them from the sample buffer. If there
// are less than 'numsample' samples in the buffer, returns all available.
//
// Returns number of samples copied.
uint VCSDKCoreFIFOSampleBuffer::receiveSamples(SAMPLETYPE *output, uint maxSamples)
{
    uint num;

    num = (maxSamples > samplesInBuffer) ? samplesInBuffer : maxSamples;

    memcpy(output, ptrBegin(), channels * sizeof(SAMPLETYPE) * num);
    return receiveSamples(num);
}


// Removes samples from the beginning of the sample buffer without copying them
// anywhere. Used to reduce the number of samples in the buffer, when accessing
// the sample buffer with the 'ptrBegin' function.
uint VCSDKCoreFIFOSampleBuffer::receiveSamples(uint maxSamples)
{
    if (maxSamples >= samplesInBuffer)
    {
        uint temp;

        temp = samplesInBuffer;
        samplesInBuffer = 0;
        return temp;
    }

    samplesInBuffer -= maxSamples;
    bufferPos += maxSamples;

    return maxSamples;
}


// Returns nonzero if the sample buffer is empty
int VCSDKCoreFIFOSampleBuffer::isEmpty() const
{
    return (samplesInBuffer == 0) ? 1 : 0;
}


// Clears the sample buffer
void VCSDKCoreFIFOSampleBuffer::clear()
{
    samplesInBuffer = 0;
    bufferPos = 0;
}


/// allow trimming (downwards) amount of samples in pipeline.
/// Returns adjusted amount of samples
uint VCSDKCoreFIFOSampleBuffer::adjustAmountOfSamples(uint numSamples)
{
    if (numSamples < samplesInBuffer)
    {
        samplesInBuffer = numSamples;
    }
    return samplesInBuffer;
}









