#pragma once

#include "ofMain.h"
#include "ofBufferObject.h"
#include "ofxAudioAnalyzer.h"
#include "ofxMidi.h"

class ofApp : public ofBaseApp, public ofxMidiListener{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void audioIn(ofSoundBuffer & buffer);
		void newMidiMessage(ofxMidiMessage& eventArgs);

		bool bLoadPostShader;
		ofShader post_shader;
		ofImage maskImage;
		ofImage eyeImage1;
		ofImage eyeImage2;
		ofImage eyeImage3;

		ofShader fractal_shader;
		ofFbo fbo1, fbo2;
		ofFbo last;

		int artifactType;
		float weirdFactor1;
		float weirdFactor2;
		float cameraRotateSpeed;
		float cameraDist;

		void changeArtifactType(int type);
		void resetAttractors();
		void setAttractorParameters();
		void loadParticles();
		void spawnParticles();

		struct Particle{
			glm::vec4 pos;
			glm::vec4 color;
		};

		struct Component{
			glm::vec4 value;
		};

		vector<Particle> particles;
		vector<Particle> line1;
		vector<Particle> line2;

		ofShader compute;
		ofBufferObject particlesBuffer, particlesBuffer2;
		ofBufferObject line1Buffer, line1Buffer2;
		ofBufferObject line2Buffer, line2Buffer2;
		ofVbo vbo, vboLine1, vboLine2;

		ofCamera camera;
		float theta;
		float phi;
		float dist;

		int attractor_type;
		float step_size;
		int numVertex;
		int numParticles;
		float spawnRadius;

		bool bResetAttractors;

		ofxAudioAnalyzer audioAnalyzer;
		ofSoundStream soundStream;
		ofxMidiIn midiIn;

		ofBufferObject audioBuffer;
		ofBufferObject pointsBuffer;

		int audioMode;
		int audioBufferSize;
		vector<Component> audioVector;
		vector<Component> pointsVector;

		int sampleRate;
		int bufferSize;
		int channels;
		float volume;

		float lowSmoothing;
		float highSmoothing;
};
