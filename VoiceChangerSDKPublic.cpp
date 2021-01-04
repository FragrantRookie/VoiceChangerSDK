////  VoiceChangerSDKPublic.cpp
//  VoiceChangerCFramework
//
//  Created on 2021/1/4.
//  
//

#include "VoiceChangerSDKPublic.hpp"




//using namespace vcsdkcore;



VoiceChangerSDKPublic::VoiceChangerSDKPublic() {
    
    _vcsdkCore = new vcsdkcore::VCSDKCore();
    
    
    
    // set default
    _vcsdkCore->setSetting(SETTING_SEQUENCE_MS, 40); // 帧长
    _vcsdkCore->setSetting(SETTING_SEEKWINDOW_MS, 15); // 叠加的时候寻找窗的范围长度（ms）
    _vcsdkCore->setSetting(SETTING_OVERLAP_MS, 8);  //  叠加范围（ms），样例中用的是8
    
    
    
}

VoiceChangerSDKPublic::~VoiceChangerSDKPublic() {
    
    
}


/** 1. 实时输入CSSampleBuffer数据处理
 * putSamples输入,receiveSamples输出处理后的sampleBuffer。
 */
// 往ST中输入buffer
void VoiceChangerSDKPublic::putSamples(const short *samples,uint length) {
    _vcsdkCore->putSamples(samples, length);
}
// 处理完成的buffers从ST中输出
void VoiceChangerSDKPublic::receiveSample(short *buffer,uint length) {
    _vcsdkCore->receiveSamples(buffer,length);
}

/** 2. 读取文件get SampleBuffer
 *
 */
void VoiceChangerSDKPublic::readFileToVoiceChanger(char *filepath) {
    
    
    
    
}




/** 1.  萝莉音
 *
 * 提高8个音调(女声)
 *
 */
void VoiceChangerSDKPublic::luoliSound() {
    _vcsdkCore->setPitchSemiTones(8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(1);
}


/** 2. 大叔音
 * 降低音调到0.8
 *
 */
void VoiceChangerSDKPublic::uncleSound() {
    _vcsdkCore->setPitch(0.8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(1);
}

/** 3. 搞怪音
 *  提高语速x2
 *
 */
void VoiceChangerSDKPublic::funnySound() {
    _vcsdkCore->setPitchSemiTones(0);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRate(2);
}

    
/** 4. 小黄人音
 *
 */
void VoiceChangerSDKPublic::littleYellowSound() {
    _vcsdkCore->setPitchSemiTones(8);
    _vcsdkCore->setTempo(1);
    _vcsdkCore->setRateChange(120);
}













