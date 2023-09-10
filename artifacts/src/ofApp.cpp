#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	lowSmoothing = 0.4;
	highSmoothing = 0.8;
	volume = 10.0;

	sampleRate = 48000;
    bufferSize = 1024;
    channels = 2;
    
    audioAnalyzer.setup(sampleRate, bufferSize, channels);
	bpmDetector.setup(channels, sampleRate, 64);

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
	auto devices = soundStream.getDeviceList();
	if(!devices.empty()){
		settings.setInDevice(devices[0]);
	}

	// auto devices = soundStream.getDeviceList();
	// if (!devices.empty()) {
	// 	settings.setInDevice(devices[1]);
	// }

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
		audioBufferSize = 20;
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
	pointsVector[0].value.z = 1.;
	pointsVector[0].value.w = 0.;

	pointsBuffer.allocate(pointsVector, GL_DYNAMIC_DRAW);
	pointsBuffer.bindBase(GL_SHADER_STORAGE_BUFFER, 7);

	ofSetFrameRate(60);

	attractors.setup();
	fractals.setup();
	scrolls.setup();

	fboAttractors.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fboFractals.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fboScrolls.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

	// fractals
	fbo1.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fbo2.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	last.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

	// post
	post_shader.load("shaders/generic.vert", "shaders/post.frag");

	artifactType = 0;

	cameraRotateSpeed = 0.2;

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
	maskImage.load("assets/hyperportals_mask.png");
	maskImage.resize(ofGetWidth(), ofGetHeight());

	artificerImage.load("assets/artitficer-title.png");
	artificerImage.resize(ofGetWidth(), ofGetHeight());

	eyePlayer.load("assets/eyeMovement.mkv");
	eyePlayer.setLoopState(OF_LOOP_NORMAL);
    eyePlayer.setVolume(0);
    eyePlayer.play();

	eyeTexture.allocate(eyePlayer.getWidth(), eyePlayer.getHeight(), GL_RGBA16);

	vidGrabber.setVerbose(true);
	vidGrabber.setDeviceID(0);
	int sourceWidth = ofGetWidth();
	int sourceHeight = ofGetHeight();
	vidGrabber.initGrabber(sourceWidth, sourceHeight);

	vidTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
}

//--------------------------------------------------------------
void ofApp::update(){
	// video and optical flow
	vidGrabber.update();
	bool bNewFrame = vidGrabber.isFrameNew();
	
	if (bNewFrame){
		ofPixels & pixels = vidGrabber.getPixels();
		vidTexture.loadData(pixels);
	}

	bool normalize = true;
	vector<float> melBands = audioAnalyzer.getValues(MEL_BANDS, 0, lowSmoothing);
	float bass = ofMap(melBands[0], DB_MIN, DB_MAX, 0., 1., true);

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

	int numPoints = pointsVector.size();
	int numAudio = audioVector.size();

	attractors.update(weirdFactor1, weirdFactor2, numPoints, numAudio);
	scrolls.update(vidTexture, numAudio, numPoints, bass);

	if (bLoadPostShader) {
		reloadShaders();
		bLoadPostShader = false;
	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	vector<float> melBands = audioAnalyzer.getValues(MEL_BANDS, 0, lowSmoothing);
	bool isOnset = audioAnalyzer.getOnsetValue(0);

	float bpm = bpmDetector.getBPM();
	float bps = bpm / 60.0;

	fboFractals.begin();
	ofClear(0, 0, 0, 0);
	fractals.draw(audioVector.size(), pointsVector.size(), cameraRotateSpeed, cameraDist, weirdFactor1, weirdFactor2, melBands[0]);
	fboFractals.end();

	fboAttractors.begin();
	ofClear(0, 0, 0, 0);
	attractors.draw(cameraDist);
	fboAttractors.end();

	fboScrolls.begin();
	ofClear(0, 0, 0, 0);
	scrolls.draw();
	fboScrolls.end();

	eyePlayer.update();
    if(eyePlayer.isFrameNew()){
        ofPixels & pixels = eyePlayer.getPixels();
        eyeTexture.loadData(pixels);
	}

	fbo2.begin();
	ofClear(0, 0, 0, 0);
	post_shader.begin();
	post_shader.setUniform2i("resolution", ofGetWidth(), ofGetHeight());
	post_shader.setUniform1f("time", ofGetElapsedTimef());
	post_shader.setUniformTexture("last", last.getTexture(), 0);
	post_shader.setUniformTexture("fractals", fboFractals.getTexture(), 1);
	post_shader.setUniformTexture("attractors", fboAttractors.getTexture(), 2);
	post_shader.setUniformTexture("scrolls", fboScrolls.getTexture(), 3);
	post_shader.setUniformTexture("mask", maskImage.getTexture(), 4);
	post_shader.setUniformTexture("artificer", artificerImage.getTexture(), 5);
	post_shader.setUniformTexture("eye", eyeTexture, 6);
	post_shader.setUniform1f("bps", bps);
	post_shader.setUniform1f("low", melBands[0]);
	post_shader.setUniform1f("high", melBands[melBands.size() - 1]);
	post_shader.setUniform1i("postType", postType);
	post_shader.setUniform1f("postVar", postVar);
	ofSetColor(255);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	post_shader.end();
	fbo2.end();

	last.begin();
	ofClear(0, 0, 0, 0);
	fbo2.draw(0, 0, ofGetWidth(), ofGetHeight());
	last.end();
	last.draw(0, 0, ofGetWidth(), ofGetHeight());
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
				postVar = ofMap(message.value, 0, 127, 0., 1.);
			}
		}
	}
}

void ofApp::audioIn(ofSoundBuffer & buffer){
	buffer *= volume;
	audioAnalyzer.analyze(buffer);
	bpmDetector.processFrame(buffer.getBuffer().data(), buffer.getNumFrames(), buffer.getNumChannels());
}

void ofApp::keyPressed(int key) {
	int num = key - '0';
	changeArtifactType(num);
}

void ofApp::changeArtifactType(int type) {

	if (type <= 11) {
		artifactType = type;
	}

	if (artifactType < 4) {
		int fractalType = artifactType;
		fractals.setFractalType(fractalType);
	}
	
	if ((artifactType >= 4) && (artifactType <= 11)) {
		int attractorType = artifactType - 4;
		attractors.setAttractorType(attractorType);
	}
	
	if ((artifactType > 11) && (artifactType < 23)) {
		postType = type - 12;
	}

	if (type == 24) {
		bLoadPostShader = true;
	}
}

void ofApp::reloadShaders() {
	post_shader.load("generic.vert", "post.frag");
	attractors.reloadShaders();
	fractals.reloadShaders();
}
