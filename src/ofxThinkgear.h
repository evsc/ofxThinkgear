#ifndef _OFX_THINKGEAR_
#define _OFX_THINKGEAR_

#include "ofSerial.h"
#include "ofEvents.h"
#include "ThinkGearStreamParser.h"
#include <list>

#ifdef TARGET_LINUX
#define THINKGEAR_PORT "/dev/rfcomm0"
#endif
#ifdef TARGET_OSX
#define THINKGEAR_PORT "/dev/tty.MindWave"
#endif
#define THINKGEAR_BAUD 57600

class ofxThinkgearEventArgs : public ofEventArgs {
public:
    short raw;                  // -32768 to 32767, at 128 or 512 Hz (depends on chip)
    unsigned char power;
    unsigned char poorSignal;   // 0 is good. 200 = electrodes not in contact with skin
    unsigned char heartRate;    // 0-255
    unsigned char blinkStrength;
    unsigned char attention;    // 0 = unable to calculate, 40-60 = neutral
    unsigned char meditation;   // 0 = unable to calculate, 40-60 = neutral

    unsigned int eegDelta;      // 100000 / 1500000 . 0.5-2.75hz
    unsigned int eegTheta;      // 300000 / 600000 . 3.5-6.75hz
    unsigned int eegLowAlpha;   // 2500 / 75000 . 7.5-9.25hz
    unsigned int eegHighAlpha;  // 2500 / 150000 . 10-11.75hz
    unsigned int eegLowBeta;    // 1500 / 60000 . 13-16.75hz
    unsigned int eegHighBeta;   // 2500 / 60000 . 18-29.75hz
    unsigned int eegLowGamma;   // 5000 / 300000 . 31-39.75hz
    unsigned int eegMidGamma;   // 5000 / 300000 . 41-49.75hz
};

enum EEG_KIND {
    EEG_DELTA,
    EEG_THETA,
    EEG_LOW_ALPHA,
    EEG_HIGH_ALPHA,
    EEG_LOW_BETA,
    EEG_HIGH_BETA,
    EEG_LOW_GAMMA,
    EEG_MID_GAMMA
};

class EegDataPart {
public:
    EEG_KIND kind;
    unsigned int value;
    unsigned int previousValue;
    float ratio;
    float previousRatio;
    
    EegDataPart(EEG_KIND k) : kind(k) {
        value = 0;
        ratio = 0;
        previousValue = 0;
        previousRatio = 0;
    }
    
    void feed(unsigned int v, unsigned long int total){
        previousRatio = ratio;
        previousValue = value;
        value = v;
        ratio = (float)v / (float)total;
    }
};

class EegData {
public:
    std::list<EegDataPart*> data;
    typedef std::list<EegDataPart*>::iterator iterator;
    typedef std::list<EegDataPart*>::const_iterator const_iterator;
    
    EegDataPart delta;
    EegDataPart theta;
    EegDataPart lowAlpha;
    EegDataPart highAlpha;
    EegDataPart lowBeta;
    EegDataPart highBeta;
    EegDataPart lowGamma;
    EegDataPart midGamma;
    
    EegData() : 
        delta(EEG_DELTA),
        theta(EEG_THETA),
        lowAlpha(EEG_LOW_ALPHA),
        highAlpha(EEG_HIGH_ALPHA),
        lowBeta(EEG_LOW_BETA),
        highBeta(EEG_HIGH_BETA),
        lowGamma(EEG_LOW_GAMMA),
        midGamma(EEG_MID_GAMMA)
    {
        data.push_back(&delta);
        data.push_back(&theta);
        data.push_back(&lowAlpha);
        data.push_back(&highAlpha);
        data.push_back(&lowBeta);
        data.push_back(&highBeta);
        data.push_back(&lowGamma);
        data.push_back(&midGamma);
    }
        
    void feed(ofxThinkgearEventArgs& owner){
        data.clear();
        unsigned long int total 
        = owner.eegDelta
        + owner.eegTheta
        + owner.eegLowAlpha
        + owner.eegHighAlpha
        + owner.eegLowBeta
        + owner.eegHighBeta
        + owner.eegLowGamma
        + owner.eegMidGamma;
        if (total > 0){
            delta.feed(owner.eegDelta, total);
            theta.feed(owner.eegTheta, total);
            lowAlpha.feed(owner.eegLowAlpha, total);
            highAlpha.feed(owner.eegHighAlpha, total);
            lowBeta.feed(owner.eegLowBeta, total);
            highBeta.feed(owner.eegHighBeta, total);
            lowGamma.feed(owner.eegLowGamma, total);
            midGamma.feed(owner.eegMidGamma, total);
            data.sort(compare);
        }
    }
    
    iterator begin(){ return data.begin(); }
    iterator end(){ return data.end(); }
    const_iterator begin() const { return data.begin(); }
    const_iterator end() const { return data.end(); }

    static int compare(const EegDataPart* a, const EegDataPart* b){
        if (a->value == b->value)
            return 0;
        if (a->value > b->value)
            return -1;
        return 1;
    }
    
private:
};

std::ostream& operator<<(std::ostream&, const EegDataPart&);

class ofxThinkgear {
public:
    ofSerial device;
    int baudRate;
    string devicePort;
    ofxThinkgearEventArgs values;
    bool isReady;

    ofEvent<ofxThinkgearEventArgs> onRaw;
    ofEvent<ofxThinkgearEventArgs> onPower;
    ofEvent<ofxThinkgearEventArgs> onPoorSignal;
    ofEvent<ofxThinkgearEventArgs> onHeartRate;
    ofEvent<ofxThinkgearEventArgs> onBlinkStrength;
    ofEvent<ofxThinkgearEventArgs> onAttention;
    ofEvent<ofxThinkgearEventArgs> onMeditation;
    ofEvent<ofxThinkgearEventArgs> onEeg;
    ofEvent<ofxThinkgearEventArgs> onConnecting;
    ofEvent<ofxThinkgearEventArgs> onReady;
    ofEvent<ofMessage> onError;

    void setup(string devicePort, int baudRate);

    ofxThinkgear();
    ~ofxThinkgear();

    void flush();
    void update();
    bool open();
    void close();

    template <class ListenerClass>
	void addEventListener(ListenerClass * listener){
		ofAddListener(onRaw,listener,&ListenerClass::onThinkgearRaw);
		ofAddListener(onPower,listener,&ListenerClass::onThinkgearPower);
        ofAddListener(onPoorSignal,listener,&ListenerClass::onThinkgearPoorSignal);
		ofAddListener(onHeartRate,listener,&ListenerClass::onThinkgearHeartRate);
		ofAddListener(onBlinkStrength,listener,&ListenerClass::onThinkgearBlinkStrength);
		ofAddListener(onAttention,listener,&ListenerClass::onThinkgearAttention);
		ofAddListener(onMeditation,listener,&ListenerClass::onThinkgearMeditation);
		ofAddListener(onEeg,listener,&ListenerClass::onThinkgearEeg);
		ofAddListener(onConnecting,listener,&ListenerClass::onThinkgearConnecting);
		ofAddListener(onReady,listener,&ListenerClass::onThinkgearReady);
		ofAddListener(onError,listener,&ListenerClass::onThinkgearError);
	}

    template <class ListenerClass>
	void removeEventListener(ListenerClass * listener){
		ofRemoveListener(onRaw,listener,&ListenerClass::onThinkgearRaw);
		ofRemoveListener(onPower,listener,&ListenerClass::onThinkgearPower);
        ofRemoveListener(onPoorSignal,listener,&ListenerClass::onThinkgearPoorSignal);
		ofRemoveListener(onHeartRate,listener,&ListenerClass::onThinkgearHeartRate);
		ofRemoveListener(onBlinkStrength,listener,&ListenerClass::onThinkgearBlinkStrength);
		ofRemoveListener(onAttention,listener,&ListenerClass::onThinkgearAttention);
		ofRemoveListener(onMeditation,listener,&ListenerClass::onThinkgearMeditation);
		ofRemoveListener(onEeg,listener,&ListenerClass::onThinkgearEeg);
		ofRemoveListener(onConnecting,listener,&ListenerClass::onThinkgearConnecting);
		ofRemoveListener(onReady,listener,&ListenerClass::onThinkgearReady);
		ofRemoveListener(onError,listener,&ListenerClass::onThinkgearError);
	}

private:
    ThinkGearStreamParser parser;
    unsigned char buffer[512];
};

#endif