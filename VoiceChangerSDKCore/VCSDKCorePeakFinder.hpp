////  VCSDKCorePeakFinder.hpp
//  VoiceChangerCFramework
//
//  Created on 2020/12/15.
//  
//

#ifndef VCSDKCorePeakFinder_hpp
#define VCSDKCorePeakFinder_hpp




namespace vcsdkcore {

class VCSDKCorePeakFinder
{
protected:
    /// Min, max allowed peak positions within the data vector
    int minPos, maxPos;

    /// Calculates the mass center between given vector items.
    double calcMassCenter(const float *data, ///< Data vector.
                         int firstPos,      ///< Index of first vector item beloging to the peak.
                         int lastPos        ///< Index of last vector item beloging to the peak.
                         ) const;

    /// Finds the data vector index where the monotoniously decreasing signal crosses the
    /// given level.
    int   findCrossingLevel(const float *data,  ///< Data vector.
                            float level,        ///< Goal crossing level.
                            int peakpos,        ///< Peak position index within the data vector.
                            int direction       /// Direction where to proceed from the peak: 1 = right, -1 = left.
                            ) const;

    // Finds real 'top' of a peak hump from neighnourhood of the given 'peakpos'.
    int findTop(const float *data, int peakpos) const;


    /// Finds the 'ground' level, i.e. smallest level between two neighbouring peaks, to right-
    /// or left-hand side of the given peak position.
    int   findGround(const float *data,     /// Data vector.
                     int peakpos,           /// Peak position index within the data vector.
                     int direction          /// Direction where to proceed from the peak: 1 = right, -1 = left.
                     ) const;

    /// get exact center of peak near given position by calculating local mass of center
    double getPeakCenter(const float *data, int peakpos) const;

public:
    /// Constructor.
    VCSDKCorePeakFinder();

    /// Detect exact peak position of the data vector by finding the largest peak 'hump'
    /// and calculating the mass-center location of the peak hump.
    ///
    /// \return The location of the largest base harmonic peak hump.
    double detectPeak(const float *data, /// Data vector to be analyzed. The data vector has
                                        /// to be at least 'maxPos' items long.
                     int minPos,        ///< Min allowed peak location within the vector data.
                     int maxPos         ///< Max allowed peak location within the vector data.
                     );
};


}




#endif /* VCSDKCorePeakFinder_hpp */
