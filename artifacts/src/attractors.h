#pragma once

#include "ofMain.h"

class Attractors {
    public:
        void setup();
        void update(float weirdFactor1, float weirdFactor2, int numPoints, int numAudio, float bass, float bpm, int beat);
        void draw(float cameraDist, float cameraRotateSpeed);

		void setAttractorType(int attractorType);
		void spawnParticles();
		void resetAttractors();
		void setAttractorParameters();
		void allocateParticles();
		void loadParticles();
		void assignVbos();
		void bindBuffers();
		void reloadShaders();

    private:
        struct Particle{
			glm::vec4 pos;
			glm::vec4 color;
		};

        ofBoxPrimitive box1;
        ofBoxPrimitive box2;
        ofIcoSpherePrimitive sphere1;

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
};