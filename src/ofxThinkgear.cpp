#include "ofxThinkgear.h"

void tgHandleDataValueFunc( unsigned char extendedCodeLevel, unsigned char code, unsigned char valueLength, const unsigned char *value, void *customData){
    ofxThinkgear& tg = *reinterpret_cast<ofxThinkgear*>(customData);
    if (extendedCodeLevel == 0){
        switch (code) {
            case PARSER_CODE_BATTERY:
                tg.values.power = value[0] & 0xff;
                ofNotifyEvent(tg.onPower, tg.values);
                break;
            case PARSER_CODE_POOR_QUALITY:
                tg.values.poorSignal = value[0] & 0xff;
                ofNotifyEvent(tg.onPoorSignal, tg.values);
                break;
            case PARSER_CODE_HEART_RATE:
                tg.values.heartRate = value[0] & 0xff;
                ofNotifyEvent(tg.onHeartRate, tg.values);
                break;
            case PARSER_CODE_ATTENTION:
                tg.values.attention = value[0] & 0xff;
                ofNotifyEvent(tg.onAttention, tg.values);
                break;
            case PARSER_CODE_MEDITATION:
                tg.values.meditation = value[0] & 0xff;
                ofNotifyEvent(tg.onMeditation, tg.values);
                break;
            case 0x16:
                tg.values.blinkStrength = value[0] & 0xff;
                ofNotifyEvent(tg.onBlinkStrength, tg.values);
            case 0xd4:
                printf("Standby... autoconnecting\n");
                ofNotifyEvent(tg.onConnecting, tg.values);
                tg.device.writeByte(0xc2);
                break;
            case 0xd0:
                ofNotifyEvent(tg.onReady, tg.values);
                break;
            case 0xd1:
                {
                    ofMessage err("Headset not found");
                    ofNotifyEvent(tg.onError, err);
                }
                break;
            case PARSER_CODE_RAW_SIGNAL:
                tg.values.raw = (value[0] << 8) | value[1];
                ofNotifyEvent(tg.onRaw, tg.values);
                break;
            case PARSER_CODE_ASIC_EEG_POWER_INT:
                {
                    int pos = 0;
                    tg.values.eegDelta = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegTheta = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegLowAlpha = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegHighAlpha = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegLowBeta = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegHighBeta = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegLowGamma = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    tg.values.eegMidGamma = (value[pos] << 16) | (value[pos+1] << 8) | (value[pos+2]); pos += 3;
                    ofNotifyEvent(tg.onEeg, tg.values);
                    break;
                }
            /* Other [CODE]s */
            default:
                printf( "EXCODE level: %d CODE: 0x%02X vLength: %d\n", extendedCodeLevel, code, valueLength );
                printf( "Data value(s):" );
                for( int i=0; i<valueLength; i++ ) printf( " %02X", value[i] & 0xFF );
                printf( "\n" );
                break;
        }
    }
}


ofxThinkgear::ofxThinkgear() : isReady(false) {
}

ofxThinkgear::~ofxThinkgear(){
    close();
}

void ofxThinkgear::setup(string devicePort, int baudRate){
    printf( "ofxThinkgear Setup \t \t %d\n", baudRate );
    this->devicePort = devicePort;
    this->baudRate = baudRate;
}

void ofxThinkgear::close(){
    if (isReady){
        device.writeByte(0xC1);
        device.flush();
        device.close();
    }
}

void ofxThinkgear::update(){
    // printf( "." );
    if (!isReady){
        if (device.setup(devicePort, baudRate)){
            device.flush();
            THINKGEAR_initParser(&parser, PARSER_TYPE_PACKETS, tgHandleDataValueFunc, this);
            isReady = true;
        }
    }
    if (!isReady)
        return;
    int n = device.available();
    // printf( "isReady %d\t", n );
    if (n > 0){
        n = device.readBytes(buffer, min(n,512));
        for (int i=0; i<n; ++i){
            int returnValue = THINKGEAR_parseByte(&parser, buffer[i]);
            // printf( "parsePackage \t %d\n", returnValue );
        }
    }
}

void ofxThinkgear::flush(){
    if (isReady)
        device.flush();	
}

std::ostream& operator<<( std::ostream& stream, const EegDataPart& p){
    switch (p.kind){
        case EEG_DELTA: stream << "D"; break;
        case EEG_THETA: stream << "T"; break;
        case EEG_LOW_ALPHA: stream << "LA"; break;
        case EEG_HIGH_ALPHA: stream << "HA"; break;
        case EEG_LOW_BETA: stream << "LB"; break;
        case EEG_HIGH_BETA: stream << "HB"; break;
        case EEG_LOW_GAMMA: stream << "LG"; break;
        case EEG_MID_GAMMA: stream << "MG"; break;
        default: break;
    }
    stream << "=" << (p.ratio * 100.0) << "(" << p.value << ")";
    return stream;
}
