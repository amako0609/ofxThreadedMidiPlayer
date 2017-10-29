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




using namespace jdksmidi;

class ofxThreadedMidiPlayer: public ofThread
{
public:
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
    
    bool setBpm(float bpm);
    void goToZero();

    ofxMidiEvent midiEvent;
    
protected:
    void init();
    void dispatchMidiEvent(vector<unsigned char>& message);
    bool bIsInited;
};
