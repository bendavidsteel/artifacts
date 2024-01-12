#pragma once

#include "ofMain.h"
#include "ofBufferObject.h"
#include "ofxAudioAnalyzer.h"
#include "ofxBPMDetector.h"
#include "ofxMidi.h"
#include "ofxNetwork.h"

#include "attractors.h"
#include "scrolls.h"
#include "fractals.h"
#include "temple.h"

class ofApp : public ofBaseApp, public ofxMidiListener{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void audioIn(ofSoundBuffer & buffer);
		void newMidiMessage(ofxMidiMessage& eventArgs);

		void changeArtifactType(int type);
		void reloadShaders();

		Attractors attractors;
		Scrolls scrolls;
		Fractals fractals;
		Temple temple;

		bool bLoadPostShader;
		ofImage maskImage;
		ofImage artificerImage;
		ofVideoPlayer eyePlayer;
		ofTexture eyeTexture;

		vector<ofFbo> fbos;
		vector<ofFbo> lastFbos;
		vector<ofShader> post_shaders;

		ofFbo fboAttractors;
		ofFbo fboScrolls;
		ofFbo fboFractals;
		ofFbo fboTemple;

		int postType;
		float postVar;

		int artifactType;
		float weirdFactor1;
		float weirdFactor2;
	
		struct Component{
			glm::vec4 value;
		};

		ofxAudioAnalyzer audioAnalyzer;
		ofxBPMDetector bpmDetector;
		ofSoundStream soundStream;
		ofxMidiIn midiIn;
		ofxUDPManager udpConnection;

		ofBufferObject audioBuffer;
		ofBufferObject pointsBuffer;

		int audioMode;
		int audioBufferSize;
		vector<Component> audioVector;
		vector<Component> pointsVector;

		float cameraRotateSpeed;
		float cameraDist;

		int sampleRate;
		int bufferSize;
		int channels;
		float volume;
		int beat;
		float bpm;

		float lowSmoothing;
		float highSmoothing;

		// video and optical flow
		ofVideoGrabber vidGrabber;
		ofTexture vidTexture;

		int iSortPixels;
		int iNumScreens;
};
