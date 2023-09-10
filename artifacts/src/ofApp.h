#pragma once

#include "ofMain.h"
#include "ofBufferObject.h"
#include "ofxAudioAnalyzer.h"
#include "ofxBPMDetector.h"
#include "ofxMidi.h"

#include "attractors.h"
#include "scrolls.h"
#include "fractals.h"

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

		bool bLoadPostShader;
		ofShader post_shader;
		ofImage maskImage;
		ofImage artificerImage;
		ofVideoPlayer eyePlayer;
		ofTexture eyeTexture;

		ofFbo fbo1, fbo2;
		ofFbo last;

		ofFbo fboAttractors;
		ofFbo fboScrolls;
		ofFbo fboFractals;

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

		float lowSmoothing;
		float highSmoothing;

		// video and optical flow
		ofVideoGrabber vidGrabber;
		ofTexture vidTexture;

		int iSortPixels;
};
