#pragma once

#include "ofMain.h"

#include <jdksmidi/world.h>
#include <jdksmidi/track.h>
#include <jdksmidi/multitrack.h>
#include <jdksmidi/filereadmultitrack.h>
#include <jdksmidi/fileread.h>
#include <jdksmidi/fileshow.h>
#include <jdksmidi/filewritemultitrack.h>
#include <jdksmidi/sequencer.h>
#include "ofxMidi.h"

#define USE_FRAMECOUNT_MANAGER

#ifdef USE_FRAMECOUNT_MANAGER
#include "FrameCountManager.h"
#endif
using namespace jdksmidi;

class ofxThreadedMidiPlayer: public ofThread
{
public:
    function<uint64_t(void)> getTimeMillis;
    int count;
    bool isReady;
    string midiFileName;
    int midiPort;
    unsigned lastMessageMillis;
    //float currentTime;
    //float nextEventTime;
    
    //double musicDurationInSeconds;
    //float max_time;
    //long myTime;
    //float startTimeMs;
    bool doLoop;
    
    MIDIMultiTrack *tracks;
    MIDISequencer *sequencer;
    MIDITimedBigMessage lastTimedBigMessage;
    
    RtMidiOut *midiout;
//    float currentTime = 0;
    ofxThreadedMidiPlayer();
    ~ofxThreadedMidiPlayer();
    void stop();
    void DumpMIDITimedBigMessage( const MIDITimedBigMessage& msg );
    void start();
    
    void setup(string fileName, int portNumber, bool shouldLoop = true);
    void threadedFunction();
    void clean();
    void DumpTrackNames();
    float getBpm();
    float bpm;
    bool setBpm(float bpm);
    void goToZero();

    double getCurrentTimeInMS(){
        if(!sequencer){
            return false;
        }
        return sequencer->GetCurrentTimeInMs();
    }
    bool GoToTimeMs(float time_ms){
        if(!sequencer){
            return false;
        }
        return sequencer->GoToTimeMs(time_ms);
    }
    ofxMidiEvent midiEvent;
    
protected:
    void init();
    void dispatchMidiEvent(vector<unsigned char>& message);
    bool bIsInited;
};
