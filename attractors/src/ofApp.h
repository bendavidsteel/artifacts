#pragma once

#include "ofMain.h"
#include "ofBufferObject.h"
#include "ofxAudioAnalyzer.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void audioIn(ofSoundBuffer & buffer);

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

		float step_size;
		int attractor_type;
		int numVertex;
		int numParticles;
		float spawnRadius;

		ofxAudioAnalyzer audioAnalyzer;
		ofSoundStream soundStream;

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
