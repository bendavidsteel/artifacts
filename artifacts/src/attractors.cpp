#include "attractors.h"

void Attractors::setup() {
    compute.setupShaderFromFile(GL_COMPUTE_SHADER, "shaders/attractors/compute_attractors.glsl");
	compute.linkProgram();

	attractor_type = 4;
	setAttractorParameters();
	spawnParticles();
	allocateParticles();
	assignVbos();
	bindBuffers();

	ofBackground(0);
	ofEnableBlendMode(OF_BLENDMODE_ADD);

    sphere1.setPosition(glm::vec3(0., 0., 0.));

	camera.setFarClip(ofGetWidth()*10);
	camera.setNearClip(0.1);
	theta = 0.;

	camera.setPosition(glm::vec3(0., 0., dist));
	camera.lookAt(glm::vec3(0., 0., 0.));
}

void Attractors::update(float weirdFactor1, float weirdFactor2, int numPoints, int numAudio, float bass, float bpm, int beat) {
    if (bResetAttractors) {
        resetAttractors();
        bResetAttractors = false;
    }

    compute.begin();
    compute.setUniform1f("timeLastFrame",ofGetLastFrameTime());
    compute.setUniform1f("elapsedTime",ofGetElapsedTimef());
    
    compute.setUniform1f("step_size",step_size);
    compute.setUniform1i("attractor_type", attractor_type);
    compute.setUniform1i("num_vertex", line1.size());
    compute.setUniform1i("numAudio", numAudio);
    compute.setUniform1i("numPoints", numPoints);
    compute.setUniform1f("dist", dist);
    compute.setUniform1f("weirdFactor1", weirdFactor1);
    compute.setUniform1f("weirdFactor2", weirdFactor2);
	compute.setUniform1f("bass", bass);
	compute.setUniform1f("bpm", bpm);
	compute.setUniform1i("beat", beat);

    // since each work group has a local_size of 1024 (this is defined in the shader)
    // we only have to issue 1 / 1024 workgroups to cover the full workload.
    // note how we add 1024 and subtract one, this is a fast way to do the equivalent
    // of std::ceil() in the float domain, i.e. to round up, so that we're also issueing
    // a work group should the total size of particles be < 1024
    compute.dispatchCompute((max(particles.size(), line1.size()) + 1024 -1 )/1024, 1, 1);

    compute.end();

    particlesBuffer.copyTo(particlesBuffer2);
    line1Buffer.copyTo(line1Buffer2);
    line2Buffer.copyTo(line2Buffer2);
}

void Attractors::draw(float cameraDist, float cameraRotateSpeed) {
    float thisDist = dist;
    thisDist += (cameraDist - 0.5) * 2 * 0.1 * dist;

    theta += 0.02 * cameraRotateSpeed;
    phi += 0.01 * cameraRotateSpeed;
    if (theta > 2 * PI) theta -= 2 * PI;
    if (phi > 2 * PI) phi -= 2 * PI;

    camera.setPosition(glm::vec3(thisDist * sin(theta) * cos(phi), thisDist * sin(theta) * sin(phi), thisDist * cos(theta)));
    camera.lookAt(glm::vec3(0., 0., 0.));

    camera.begin();

	sphere1.setRadius(thisDist * 1.4);
    sphere1.drawWireframe();

	if (!bResetAttractors) {
		glPointSize(2);
		vbo.draw(GL_POINTS,0,particles.size());
		vbo.draw(GL_POINTS,0,particles.size());

		// glLineWidth(10);
		ofSetLineWidth(100);
		vboLine1.draw(GL_LINE_STRIP,0,line1.size());
		vboLine2.draw(GL_LINE_STRIP,0,line2.size());
	}

    camera.end();
}

void Attractors::setAttractorParameters() {
	if (attractor_type == 0) {
		dist = 80;
		numVertex = 1024 * 256;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 0.1;
		step_size = 0.01;
	} else if (attractor_type == 1) {
		dist = 30;
		numVertex = 1024 * 256;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 0.01;
		step_size = 0.02;
	} else if (attractor_type == 2) {
		dist = 50;
		numVertex = 0;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 0.1;
		step_size = 0.1;
	} else if (attractor_type == 3) {
		dist = 3;
		numVertex = 1024 * 256;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 0.1;
		step_size = 0.01;
	} else if (attractor_type == 4) {
		dist = 50;
		numVertex = 0;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 10.0;
		step_size = 0.01;
	} else if (attractor_type == 5) {
		dist = 20;
		numVertex = 1024 * 256;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 1.0;
		step_size = 0.01;
	} else if (attractor_type == 6) {
		dist = 15;
		numVertex = 0;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 10.0;
		step_size = 0.01;
	} else if (attractor_type == 7) {
		dist = 5;
		numVertex = 0;
		numParticles = 1024 * 1024 * 2;
		spawnRadius = 0.1;
		step_size = 0.1;
	}
}


void Attractors::resetAttractors() {
	setAttractorParameters();
	spawnParticles();
	allocateParticles();
	assignVbos();
	bindBuffers();
}

void Attractors::allocateParticles() {
	particlesBuffer.allocate(particles,GL_DYNAMIC_DRAW);
	particlesBuffer2.allocate(particles,GL_DYNAMIC_DRAW);

	line1Buffer.allocate(line1,GL_DYNAMIC_DRAW);
	line1Buffer2.allocate(line1,GL_DYNAMIC_DRAW);

	line2Buffer.allocate(line2,GL_DYNAMIC_DRAW);
	line2Buffer2.allocate(line2,GL_DYNAMIC_DRAW);
}

void Attractors::loadParticles() {
	particlesBuffer.updateData(particles);
	particlesBuffer2.updateData(particles);

	line1Buffer.updateData(line1);
	line1Buffer2.updateData(line1);

	line2Buffer.updateData(line2);
	line2Buffer2.updateData(line2);
}

void Attractors::assignVbos() {
	vbo.setVertexBuffer(particlesBuffer,4,sizeof(Particle));
	vbo.setColorBuffer(particlesBuffer,sizeof(Particle),sizeof(glm::vec4));
	vbo.enableColors();

	vboLine1.setVertexBuffer(line1Buffer,4,sizeof(Particle));
	vboLine1.setColorBuffer(line1Buffer,sizeof(Particle),sizeof(glm::vec4));
	vboLine1.enableColors();

	vboLine2.setVertexBuffer(line2Buffer,4,sizeof(Particle));
	vboLine2.setColorBuffer(line2Buffer,sizeof(Particle),sizeof(glm::vec4));
	vboLine2.enableColors();
}

void Attractors::bindBuffers() {
	particlesBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	particlesBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	line1Buffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	line1Buffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 3);

	line2Buffer.bindBase(GL_SHADER_STORAGE_BUFFER, 4);
	line2Buffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 5);
}

void Attractors::spawnParticles() {
	particles.resize(numParticles);
	for (auto & p : particles)
	{
		float theta = ofRandom(0, 2 * PI);
		float phi = ofRandom(0, 2 * PI);
		float r = ofRandom(0, spawnRadius);
		p.pos.x = r * sin(theta) * cos(phi);
		p.pos.y = r * sin(theta) * sin(phi);
		p.pos.z = r * cos(theta);
		p.pos.w = 1.;
	}

	float theta = ofRandom(0, 2 * PI);
	float phi = ofRandom(0, 2 * PI);
	float r = ofRandom(0, spawnRadius);
	float x = r * sin(theta) * cos(phi);
	float y = r * sin(theta) * sin(phi);
	float z = r * cos(theta);
	float w = 1.;

	line1.resize(numVertex);
	for (auto & p : line1)
	{
		p.pos.x = x;
		p.pos.y = y;
		p.pos.z = z;
		p.pos.w = w;
	}

	theta = ofRandom(0, 2 * PI);
	phi = ofRandom(0, 2 * PI);
	r = ofRandom(0, spawnRadius);
	x = r * sin(theta) * cos(phi);
	y = r * sin(theta) * sin(phi);
	z = r * cos(theta);
	w = 1.;

	line2.resize(numVertex);
	for (auto & p : line2)
	{
		p.pos.x = x;
		p.pos.y = y;
		p.pos.z = z;
		p.pos.w = w;
	}
}

void Attractors::reloadShaders() {
	compute.setupShaderFromFile(GL_COMPUTE_SHADER,"shaders/attractors/compute_attractors.glsl");
	compute.linkProgram();
}

void Attractors::setAttractorType(int attractorType) {
	attractor_type = attractorType;
	bResetAttractors = true;
}