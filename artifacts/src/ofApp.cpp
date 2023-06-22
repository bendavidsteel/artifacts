#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	lowSmoothing = 0.4;
	highSmoothing = 0.8;
	volume = 5.0;

	sampleRate = 48000;
    bufferSize = 1024;
    channels = 2;
    
    audioAnalyzer.setup(sampleRate, bufferSize, channels);

	ofSoundStreamSettings settings;

	// if you want to set the device id to be different than the default
	// auto devices = soundStream.getDeviceList();
	// settings.device = devices[4];

	// you can also get devices for an specific api
	// auto devices = soundStream.getDevicesByApi(ofSoundDevice::Api::PULSE);
	// settings.device = devices[0];

	// or get the default device for an specific api:
	// settings.api = ofSoundDevice::Api::PULSE;

	// or by name
	// auto devices = soundStream.getDeviceList(ofSoundDevice::Api::ALSA);
	// if(!devices.empty()){
	// 	settings.setInDevice(devices[0]);
	// }

	auto devices = soundStream.getDeviceList();
	if (!devices.empty()) {
		settings.setInDevice(devices[1]);
	}

	settings.setInListener(this);
	settings.sampleRate = sampleRate;
	#ifdef TARGET_EMSCRIPTEN
		settings.numOutputChannels = 2;
	#else
		settings.numOutputChannels = 0;
	#endif
	settings.numInputChannels = channels;
	settings.bufferSize = bufferSize;
	soundStream.setup(settings);

	audioMode = RMS;
	if (audioMode == RMS){
		audioBufferSize = 30;
	} else if (audioMode == MEL_BANDS) {
		audioBufferSize = 24;
	}
	audioVector.resize(audioBufferSize);
	for (int i = 0; i < audioBufferSize; i++) {
		audioVector[i].value.x = 0.;
	}

	audioBuffer.allocate(audioVector, GL_DYNAMIC_DRAW);
	audioBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 6);

	pointsVector.resize(1);
	pointsVector[0].value.x = 0.;
	pointsVector[0].value.y = 0.;
	pointsVector[0].value.z = 0.;
	pointsVector[0].value.w = 0.;

	pointsBuffer.allocate(pointsVector, GL_DYNAMIC_DRAW);
	pointsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 7);

	ofSetFrameRate(60);

	compute.setupShaderFromFile(GL_COMPUTE_SHADER,"compute_attractors.glsl");
	compute.linkProgram();

	attractor_type = 0;
	setAttractorParameters();
	spawnParticles();

	particlesBuffer.allocate(particles,GL_DYNAMIC_DRAW);
	particlesBuffer2.allocate(particles,GL_DYNAMIC_DRAW);

	line1Buffer.allocate(line1,GL_DYNAMIC_DRAW);
	line1Buffer2.allocate(line1,GL_DYNAMIC_DRAW);

	line2Buffer.allocate(line2,GL_DYNAMIC_DRAW);
	line2Buffer2.allocate(line2,GL_DYNAMIC_DRAW);

	vbo.setVertexBuffer(particlesBuffer,4,sizeof(Particle));
	vbo.setColorBuffer(particlesBuffer,sizeof(Particle),sizeof(glm::vec4));
	vbo.enableColors();

	vboLine1.setVertexBuffer(line1Buffer,4,sizeof(Particle));
	vboLine1.setColorBuffer(line1Buffer,sizeof(Particle),sizeof(glm::vec4));
	vboLine1.enableColors();

	vboLine2.setVertexBuffer(line2Buffer,4,sizeof(Particle));
	vboLine2.setColorBuffer(line2Buffer,sizeof(Particle),sizeof(glm::vec4));
	vboLine2.enableColors();

	ofBackground(0);
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	particlesBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 0);
	particlesBuffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 1);

	line1Buffer.bindBase(GL_SHADER_STORAGE_BUFFER, 2);
	line1Buffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 3);

	line2Buffer.bindBase(GL_SHADER_STORAGE_BUFFER, 4);
	line2Buffer2.bindBase(GL_SHADER_STORAGE_BUFFER, 5);

	camera.setFarClip(ofGetWidth()*10);
	camera.setNearClip(0.1);
	theta = 0.;

	camera.setPosition(glm::vec3(0., 0., dist));
	camera.lookAt(glm::vec3(0., 0., 0.));

	// fractals
	fbo1.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fbo2.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	last.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

	fractal_shader.load("generic.vert", "fractals.frag");

	// post
	post_shader.load("generic.vert", "post.frag");

	artifactType = 0;

	// midi
	// setup midi
	// open port by number (you may need to change this)
	midiIn.openPort(1);
	//midiIn.openPort("IAC Pure Data In");	// by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port

	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);

	// add ofApp as a listener
	midiIn.addListener(this);

	// load images
	maskImage.load("mask.png");
	maskImage.resize(ofGetWidth(), ofGetHeight());

	eyeImage1.load("eye1.jpeg");
	eyeImage1.resize(ofGetWidth(), ofGetHeight());
	eyeImage2.load("eye2.jpeg");
	eyeImage2.resize(ofGetWidth(), ofGetHeight());
	eyeImage3.load("eye3.jpeg");
	eyeImage3.resize(ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::update(){
	bool normalize = true;
	vector<float> melBands = audioAnalyzer.getValues(MEL_BANDS, 0, lowSmoothing);

	if (audioMode == MEL_BANDS) {
		
		int numBands = 24;
		audioVector.resize(numBands);
		for(int i = 0; i < numBands; i++){
			audioVector[i].value.x = ofMap(melBands[i], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
		}
	} else if (audioMode == RMS) {
		audioVector[0].value.x = ofMap(melBands[0], DB_MIN, DB_MAX, 0.0, 1.0, true);//clamped value
		for (int i = audioBufferSize - 1; i > 0; i--) {
			audioVector[i].value.x = audioVector[i - 1].value.x;
		}
	}
	audioBuffer.updateData(audioVector);

	if (artifactType >= 4) {
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
		compute.setUniform1i("numAudio", audioVector.size());
		compute.setUniform1i("numPoints", pointsVector.size());
		compute.setUniform1f("dist", dist);

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

	if (bLoadPostShader) {
		post_shader.load("generic.vert", "post.frag");
		bLoadPostShader = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	vector<float> melBands = audioAnalyzer.getValues(MEL_BANDS, 0, lowSmoothing);
	bool isOnset = audioAnalyzer.getOnsetValue(0);

	float thisDist = dist;
	if (isOnset) {
		thisDist = dist * 0.999;
	} else {
		thisDist = dist;
	}

	fbo1.begin();
	ofClear(0, 0, 0, 0);

	if (artifactType < 4) {
		
		fractal_shader.begin();
		fractal_shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
		fractal_shader.setUniform1f("time", ofGetElapsedTimef());
		fractal_shader.setUniform1i("fractal_type", artifactType);
		fractal_shader.setUniform1f("scale", 1.0);
		fractal_shader.setUniform1f("transform_1", 0.0);
		fractal_shader.setUniform1f("transform_2", 0.0);
		fractal_shader.setUniform1f("ambient", 0.0);
		fractal_shader.setUniform1f("camera_amp", 0.0);
		fractal_shader.setUniform1f("camera_speed", 0.1);
		fractal_shader.setUniform1i("numAudio", audioVector.size());
		fractal_shader.setUniform1i("numPoints", pointsVector.size());
		ofSetColor(255);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		fractal_shader.end();
		
	} else if (artifactType >= 4) {
		theta += 0.01;
		phi += 0.005;
		if (theta > 2 * PI) theta -= 2 * PI;
		if (phi > 2 * PI) phi -= 2 * PI;
		camera.setPosition(glm::vec3(thisDist * sin(theta) * cos(phi), thisDist * sin(theta) * sin(phi), thisDist * cos(theta)));
		camera.lookAt(glm::vec3(0., 0., 0.));

		camera.begin();

		glPointSize(2);
		vbo.draw(GL_POINTS,0,particles.size());
		vbo.draw(GL_POINTS,0,particles.size());

		// glLineWidth(10);
		ofSetLineWidth(100);
		vboLine1.draw(GL_LINE_STRIP,0,line1.size());
		vboLine2.draw(GL_LINE_STRIP,0,line2.size());

		camera.end();
	}

	fbo1.end();

	ofTexture & tex = fbo1.getTexture();

	fbo2.begin();
	ofClear(0, 0, 0, 0);
	post_shader.begin();
	post_shader.setUniform2i("resolution", ofGetWidth(), ofGetHeight());
	post_shader.setUniform1f("time", ofGetElapsedTimef());
	post_shader.setUniformTexture("last", last.getTexture(), 0);
	post_shader.setUniformTexture("tex", tex, 1);
	post_shader.setUniformTexture("mask", maskImage.getTexture(), 2);
	post_shader.setUniformTexture("eye1", eyeImage1.getTexture(), 3);
	post_shader.setUniformTexture("eye2", eyeImage2.getTexture(), 4);
	post_shader.setUniformTexture("eye3", eyeImage3.getTexture(), 5);
	post_shader.setUniform1f("low", melBands[0]);
	post_shader.setUniform1f("high", melBands[melBands.size() - 1]);
	ofSetColor(255);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	post_shader.end();
	fbo2.end();
	
	last.begin();
	ofClear(0, 0, 0, 0);
	fbo2.draw(0, 0, ofGetWidth(), ofGetHeight());
	last.end();
	last.draw(0, 0, ofGetWidth(), ofGetHeight());

	// fbo2.draw(0, 0, ofGetWidth(), ofGetHeight());
}

void ofApp::setAttractorParameters() {
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

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& message) {

	if(message.status < MIDI_SYSEX) {
		if (message.status == MIDI_NOTE_ON) {
			// 48 - 72
			int key = message.pitch - 48;
			changeArtifactType(key);
			
		} else if (message.status == MIDI_CONTROL_CHANGE) {
			if (message.control == 32) {
				// rms
				weirdFactor1 = ofMap(message.value, 0, 127, 0., 1.);
			} else if (message.control == 33) {
				weirdFactor2 = ofMap(message.value, 0, 127, 0., 1.);
			} else if (message.control == 5) {
				cameraRotateSpeed = ofMap(message.value, 0, 127, 0., 1.);
			} else if (message.control == 6) {
				cameraDist = ofMap(message.value, 0, 127, 0., 1.);
			} else if (message.control == 52) {
				
			}
		}
	}
}

void ofApp::audioIn(ofSoundBuffer & buffer){
	buffer *= volume;
	audioAnalyzer.analyze(buffer);
}

void ofApp::keyPressed(int key) {
	int num = key - '0';
	changeArtifactType(num);
}

void ofApp::changeArtifactType(int type) {

	artifactType = type;
	if ((artifactType >= 4) && (artifactType <= 11)) {
		attractor_type = artifactType - 4;
		bResetAttractors = true;
	} else if (artifactType == 24) {
		bLoadPostShader = true;
	}
}

void ofApp::resetAttractors() {
	setAttractorParameters();
	spawnParticles();
	loadParticles();
}

void ofApp::loadParticles() {
	particlesBuffer.updateData(particles);
	particlesBuffer2.updateData(particles);

	line1Buffer.updateData(line1);
	line1Buffer2.updateData(line1);

	line2Buffer.updateData(line2);
	line2Buffer2.updateData(line2);
}

void ofApp::spawnParticles() {
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