////  VCSDKCorePeakFinder.cpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/15.
//  
//


#include "VCSDKCorePeakFinder.hpp"


#include <math.h>
#include <assert.h>


using namespace vcsdkcore;

#define max(x, y) (((x) > (y)) ? (x) : (y))


VCSDKCorePeakFinder::VCSDKCorePeakFinder()
{
    minPos = maxPos = 0;
}


// Finds real 'top' of a peak hump from neighnourhood of the given 'peakpos'.
int VCSDKCorePeakFinder::findTop(const float *data, int peakpos) const
{
    int i;
    int start, end;
    float refvalue;

    refvalue = data[peakpos];

    // seek within ±10 points
    start = peakpos - 10;
    if (start < minPos) start = minPos;
    end = peakpos + 10;
    if (end > maxPos) end = maxPos;

    for (i = start; i <= end; i ++)
    {
        if (data[i] > refvalue)
        {
            peakpos = i;
            refvalue = data[i];
        }
    }

    // failure if max value is at edges of seek range => it's not peak, it's at slope.
    if ((peakpos == start) || (peakpos == end)) return 0;

    return peakpos;
}


// Finds 'ground level' of a peak hump by starting from 'peakpos' and proceeding
// to direction defined by 'direction' until next 'hump' after minimum value will
// begin
int VCSDKCorePeakFinder::findGround(const float *data, int peakpos, int direction) const
{
    int lowpos;
    int pos;
    int climb_count;
    float refvalue;
    float delta;

    climb_count = 0;
    refvalue = data[peakpos];
    lowpos = peakpos;

    pos = peakpos;

    while ((pos > minPos+1) && (pos < maxPos-1))
    {
        int prevpos;

        prevpos = pos;
        pos += direction;

        // calculate derivate
        delta = data[pos] - data[prevpos];
        if (delta <= 0)
        {
            // going downhill, ok
            if (climb_count)
            {
                climb_count --;  // decrease climb count
            }

            // check if new minimum found
            if (data[pos] < refvalue)
            {
                // new minimum found
                lowpos = pos;
                refvalue = data[pos];
            }
        }
        else
        {
            // going uphill, increase climbing counter
            climb_count ++;
            if (climb_count > 5) break;    // we've been climbing too long => it's next uphill => quit
        }
    }
    return lowpos;
}


// Find offset where the value crosses the given level, when starting from 'peakpos' and
// proceeds to direction defined in 'direction'
int VCSDKCorePeakFinder::findCrossingLevel(const float *data, float level, int peakpos, int direction) const
{
    float peaklevel;
    int pos;

    peaklevel = data[peakpos];
    assert(peaklevel >= level);
    pos = peakpos;
    while ((pos >= minPos) && (pos < maxPos))
    {
        if (data[pos + direction] < level) return pos;   // crossing found
        pos += direction;
    }
    return -1;  // not found
}


// Calculates the center of mass location of 'data' array items between 'firstPos' and 'lastPos'
double VCSDKCorePeakFinder::calcMassCenter(const float *data, int firstPos, int lastPos) const
{
    int i;
    float sum;
    float wsum;

    sum = 0;
    wsum = 0;
    for (i = firstPos; i <= lastPos; i ++)
    {
        sum += (float)i * data[i];
        wsum += data[i];
    }

    if (wsum < 1e-6) return 0;
    return sum / wsum;
}



/// get exact center of peak near given position by calculating local mass of center
double VCSDKCorePeakFinder::getPeakCenter(const float *data, int peakpos) const
{
    float peakLevel;            // peak level
    int crosspos1, crosspos2;   // position where the peak 'hump' crosses cutting level
    float cutLevel;             // cutting value
    float groundLevel;          // ground level of the peak
    int gp1, gp2;               // bottom positions of the peak 'hump'

    // find ground positions.
    gp1 = findGround(data, peakpos, -1);
    gp2 = findGround(data, peakpos, 1);

    groundLevel = 0.5f * (data[gp1] + data[gp2]);
    peakLevel = data[peakpos];

    // calculate 70%-level of the peak
    cutLevel = 0.70f * peakLevel + 0.30f * groundLevel;
    // find mid-level crossings
    crosspos1 = findCrossingLevel(data, cutLevel, peakpos, -1);
    crosspos2 = findCrossingLevel(data, cutLevel, peakpos, 1);

    if ((crosspos1 < 0) || (crosspos2 < 0)) return 0;   // no crossing, no peak..

    // calculate mass center of the peak surroundings
    return calcMassCenter(data, crosspos1, crosspos2);
}



double VCSDKCorePeakFinder::detectPeak(const float *data, int aminPos, int amaxPos)
{

    int i;
    int peakpos;                // position of peak level
    double highPeak, peak;

    this->minPos = aminPos;
    this->maxPos = amaxPos;

    // find absolute peak
    peakpos = minPos;
    peak = data[minPos];
    for (i = minPos + 1; i < maxPos; i ++)
    {
        if (data[i] > peak)
        {
            peak = data[i];
            peakpos = i;
        }
    }
    
    // Calculate exact location of the highest peak mass center
    highPeak = getPeakCenter(data, peakpos);
    peak = highPeak;

    // Now check if the highest peak were in fact harmonic of the true base beat peak
    // - sometimes the highest peak can be Nth harmonic of the true base peak yet
    // just a slightly higher than the true base

    for (i = 3; i < 10; i ++)
    {
        double peaktmp, harmonic;
        int i1,i2;

        harmonic = (double)i * 0.5;
        peakpos = (int)(highPeak / harmonic + 0.5f);
        if (peakpos < minPos) break;
        peakpos = findTop(data, peakpos);   // seek true local maximum index
        if (peakpos == 0) continue;         // no local max here

        // calculate mass-center of possible harmonic peak
        peaktmp = getPeakCenter(data, peakpos);

        // accept harmonic peak if
        // (a) it is found
        // (b) is within ±4% of the expected harmonic interval
        // (c) has at least half x-corr value of the max. peak

        double diff = harmonic * peaktmp / highPeak;
        if ((diff < 0.96) || (diff > 1.04)) continue;   // peak too afar from expected

        // now compare to highest detected peak
        i1 = (int)(highPeak + 0.5);
        i2 = (int)(peaktmp + 0.5);
        if (data[i2] >= 0.4*data[i1])
        {
            // The harmonic is at least half as high primary peak,
            // thus use the harmonic peak instead
            peak = peaktmp;
        }
    }

    return peak;
}








