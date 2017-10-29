
#include "ofxThreadedMidiPlayer.h"



ofxThreadedMidiPlayer::ofxThreadedMidiPlayer(){
    count = 0;
    midiFileName = "";
    midiPort = 0;
    //currentTime = 0.0;
    //nextEventTime = 0.0;
    
    //musicDurationInSeconds = 0;
    //max_time = 0;
    //myTime = 0;
    //        lastTimedBigMessage = new MIDITimedBigMessage();
    tracks = NULL;
    sequencer = NULL;
    midiout = NULL;
    doLoop = false;
    isReady = true;
    bIsInited = false;
}
ofxThreadedMidiPlayer::~ofxThreadedMidiPlayer(){
    cout << __PRETTY_FUNCTION__ << endl;
    stop();
    clean();
}
void ofxThreadedMidiPlayer::stop(){
    cout << __PRETTY_FUNCTION__ << endl;
    stopThread();
    waitForThread();
    // clean();
    
}
void ofxThreadedMidiPlayer::DumpMIDITimedBigMessage( const MIDITimedBigMessage& msg )
{
    char msgbuf[1024];
    lastTimedBigMessage.Copy(msg);

    // note that Sequencer generate SERVICE_BEAT_MARKER in files dump,
    // but files themselves not contain this meta event...
    // see MIDISequencer::beat_marker_msg.SetBeatMarker()
    /*
    if ( msg.IsBeatMarker() )
    {
        ofLog(OF_LOG_VERBOSE, "%8ld : %s <------------------>", msg.GetTime(), msg.MsgToText ( msgbuf ) );
    }
    else
    {
        ofLog(OF_LOG_VERBOSE, "%8ld : %s", msg.GetTime(), msg.MsgToText ( msgbuf ) );
    }
    
    if ( msg.IsSystemExclusive() )
    {
        ofLog(OF_LOG_VERBOSE, "SYSEX length: %d", msg.GetSysEx()->GetLengthSE() );
    }
    //*/
    
    
}
void ofxThreadedMidiPlayer::start(){
    startThread();
}

void ofxThreadedMidiPlayer::setup(string fileName, int portNumber, bool shouldLoop){
    midiFileName = fileName;
    midiPort = portNumber;
    doLoop = shouldLoop;
    clean();
    init();
}

void ofxThreadedMidiPlayer::dispatchMidiEvent(vector<unsigned char>& message)
{
    ofxMidiMessage midiMessage(&message);
    
    if((message.at(0)) >= MIDI_SYSEX)
    {
        midiMessage.status = (MidiStatus)(message.at(0) & 0xFF);
        midiMessage.channel = 0;
    }
    else
    {
        midiMessage.status = (MidiStatus) (message.at(0) & 0xF0);
        midiMessage.channel = (int) (message.at(0) & 0x0F)+1;
    }
    
    unsigned currentMillis = ofGetElapsedTimeMillis();
    midiMessage.deltatime = currentMillis - lastMessageMillis;// deltatime;// * 1000; // convert s to ms
    lastMessageMillis = currentMillis;
    midiMessage.portNum = midiPort;
   // midiMessage.portName = portName;
    
    switch(midiMessage.status) {
        case MIDI_NOTE_ON :
        case MIDI_NOTE_OFF:
            midiMessage.pitch = (int) message.at(1);
            midiMessage.velocity = (int) message.at(2);
            break;
        case MIDI_CONTROL_CHANGE:
            midiMessage.control = (int) message.at(1);
            midiMessage.value = (int) message.at(2);
            break;
        case MIDI_PROGRAM_CHANGE:
        case MIDI_AFTERTOUCH:
            midiMessage.value = (int) message.at(1);
            break;
        case MIDI_PITCH_BEND:
            midiMessage.value = (int) (message.at(2) << 7) +
            (int) message.at(1); // msb + lsb
            break;
        case MIDI_POLY_AFTERTOUCH:
            midiMessage.pitch = (int) message.at(1);
            midiMessage.value = (int) message.at(2);
            break;
        default:
            break;
    }
    
    ofNotifyEvent(midiEvent, midiMessage, this);
    
}
void ofxThreadedMidiPlayer::threadedFunction()
{
    do
    {
        init();
        
        unsigned startTimeMillis = ofGetElapsedTimeMillis();
        
        if (sequencer)
        {
            float nextEventMs;
            while (isThreadRunning() && sequencer->GetNextEventTimeMs(&nextEventMs))
            {
                if (ofGetElapsedTimeMillis() - startTimeMillis > nextEventMs)
                {
                    MIDITimedBigMessage bigMessage;
                    int track;
                    if (sequencer->GetNextEvent(&track, &bigMessage))
                    {
                        if (bigMessage.GetLength() > 0)
                        {
                            vector<unsigned char> message;
                            message.push_back(bigMessage.GetStatus());
                            if (bigMessage.GetLength()>0) message.push_back(bigMessage.GetByte1());
                            if (bigMessage.GetLength()>1) message.push_back(bigMessage.GetByte2());
                            if (bigMessage.GetLength()>2) message.push_back(bigMessage.GetByte3());
                            if (bigMessage.GetLength()>3) message.push_back(bigMessage.GetByte4());
                            if (bigMessage.GetLength()>4) message.push_back(bigMessage.GetByte5());
                            message.resize(bigMessage.GetLength());
                            midiout->sendMessage(&message);
                            
                            dispatchMidiEvent(message);
                        }
                    }
                }
            }
        }
    }
    while (doLoop && isThreadRunning());
}

void ofxThreadedMidiPlayer::clean(){
    cout << __PRETTY_FUNCTION__ << endl;
    if(tracks){
        delete tracks;
        tracks = NULL;
    }
    if(sequencer){
        delete sequencer;
        sequencer = NULL;
    }
    //        if(lastTimedBigMessage){
    //            delete lastTimedBigMessage;
    //            lastTimedBigMessage = NULL;
    //        }
    if(midiout){
        delete midiout;
        midiout = NULL;
    }
    bIsInited = false;
}

void ofxThreadedMidiPlayer::DumpTrackNames ()
{
    fprintf ( stdout, "TEMPO = %f\n",
             sequencer->GetTrackState ( 0 )->tempobpm
             );
    
    for ( int i = 0; i < sequencer->GetNumTracks(); ++i )
    {
        fprintf ( stdout, "TRK #%2d : NAME = '%s'\n",
                 i,
                 sequencer->GetTrackState ( i )->track_name
                 );
    }
}

float ofxThreadedMidiPlayer::getBpm(){
    float bpm = -1.0f;
    if(sequencer){
        MIDISequencerState *state = sequencer->GetState();
        if(state)
            cout << state->num_tracks << endl;
        for(size_t i=0; i<state->num_tracks; i++){
            if(state->track_state[i]){
                bpm = state->track_state[i]->tempobpm;
            }
        }
    }
    return bpm;
}

bool ofxThreadedMidiPlayer::setBpm(float bpm){
    if(!sequencer){
        return false;
    }
    
    MIDISequencerState *state = sequencer->GetState();
    if(state)
        for(size_t i=0; i<state->num_tracks; i++){
            if(state->track_state[i]){
                state->track_state[i]->tempobpm = bpm;
            }
        }
    return true;
}

void ofxThreadedMidiPlayer::goToZero(){
    sequencer->GoToZero();
}

void ofxThreadedMidiPlayer::init()
{
    if(!bIsInited)
    {
        isReady = false;
        string filePath = ofToDataPath(midiFileName, true);
        
        MIDIFileReadStreamFile rs ( filePath.c_str() );
        
        if ( !rs.IsValid() )
        {
            ofLogError( "ERROR OPENING FILE AT: ",  filePath);
            
        }
        
        tracks = new MIDIMultiTrack();
        MIDIFileReadMultiTrack track_loader ( tracks );
        MIDIFileRead reader ( &rs, &track_loader );
        
        int numMidiTracks = reader.ReadNumTracks();
        int midiFormat = reader.GetFormat();
        
        tracks->ClearAndResize( numMidiTracks );
        cout << "numMidiTracks: " << numMidiTracks << endl;
        cout << "midiFormat: " << midiFormat << endl;
        
        if ( reader.Parse() ){
            cout << "reader parsed!: " << endl;
        }
        
        
        //MIDISequencer seq( &tracks );
        sequencer = new MIDISequencer ( tracks );//&seq;
        //musicDurationInSeconds = sequencer->GetMusicDurationInSeconds();
        
        //ofLogVerbose( "musicDurationInSeconds is ", ofToString(musicDurationInSeconds));
        
        midiout=new RtMidiOut();
        if (midiout->getPortCount()){
            midiout->openPort(midiPort);
            ofLogVerbose("Using Port name: " ,   ofToString(midiout->getPortName(0)) );
            //std::cout << "Using Port name: \"" << midiout->getPortName(0)<< "\"" << std::endl;
        }
        lastMessageMillis = 0;
        
        sequencer->GoToZero();
        /*
        if (!sequencer->GoToTimeMs ( currentTime )){
            ofLogError("Couldn't go to time in sequence: " ,   ofToString(currentTime) );
        }
        if ( !sequencer->GetNextEventTimeMs ( &nextEventTime ) ){
            ofLogVerbose("No next events for sequence", ofToString(nextEventTime));
        }*/
        //max_time = (musicDurationInSeconds *1000.);
        bIsInited = true;
    }
}

