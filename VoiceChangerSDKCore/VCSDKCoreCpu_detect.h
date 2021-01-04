////  VCSDKCoreCpu_detect.h
//  VoiceChangerCFramework
//
//  Created on 2020/12/15.
//  
//

#ifndef VCSDKCoreCpu_detect_h
#define VCSDKCoreCpu_detect_h

#include "VCSDKCoreType.h"

#define SUPPORT_MMX         0x0001
#define SUPPORT_3DNOW       0x0002
#define SUPPORT_ALTIVEC     0x0004
#define SUPPORT_SSE         0x0008
#define SUPPORT_SSE2        0x0010


/// Checks which instruction set extensions are supported by the CPU.
///
/// \return A bitmask of supported extensions, see SUPPORT_... defines.
uint detectCPUextensions(void);

/// Disables given set of instruction extensions. See SUPPORT_... defines.
void disableExtensions(uint wDisableMask);



#endif /* VCSDKCoreCpu_detect_h */
