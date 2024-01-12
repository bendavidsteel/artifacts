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
		settings.setInDevice(devices[1]);
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

	ofSetFrameRate(120);

	attractors.setup();
	fractals.setup();
	scrolls.setup();
	temple.setup();

	fboAttractors.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fboFractals.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fboScrolls.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
	fboTemple.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

	fbos.resize(4);
	lastFbos.resize(4);
	post_shaders.resize(4);
	for (int i = 0; i < 4; i++) {
		fbos[i].allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
		lastFbos[i].allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);
		string shaderName = "shaders/post" + ofToString(i+1) + ".frag";
		post_shaders[i].load("shaders/generic.vert", shaderName);
	}
	iNumScreens = 1;

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

	ofxUDPSettings udpSettings;
	udpSettings.receiveOn(11999);
	udpSettings.blocking = false;

	udpConnection.Setup(udpSettings);

	// load images
	maskImage.load("assets/artificer2.png");
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
	vidGrabber.setUseTexture(true);
	// vidGrabber.setPixelFormat(OF_PIXELS_YUY2);
	vidGrabber.setup(sourceWidth, sourceHeight);

	vidTexture.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA8);

	// fileName = "testMovie";
    // fileExt = ".mov"; // ffmpeg uses the extension to determine the container type. run 'ffmpeg -formats' to see supported formats

    // override the default codecs if you like
    // run 'ffmpeg -codecs' to find out what your implementation supports (or -formats on some older versions)
    // vidRecorder.setVideoCodec("mpeg4");
    // vidRecorder.setVideoBitrate("800k");
    // vidRecorder.setAudioCodec("mp3");
    // vidRecorder.setAudioBitrate("192k");

    // ofAddListener(vidRecorder.outputFileCompleteEvent, this, &ofApp::recordingComplete);

//    soundStream.listDevices();
//    soundStream.setDeviceID(11);
    soundStream.setup(this, 0, channels, sampleRate, 1024, 4);

    ofSetWindowShape(vidGrabber.getWidth(), vidGrabber.getHeight()	);
    ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::update(){
	// video and optical flow
	vidGrabber.update();
	bool bNewFrame = vidGrabber.isFrameNew();
	
	if (bNewFrame){
		ofPixels & cameraPixels = vidGrabber.getPixels();
		vidTexture.loadData(cameraPixels);
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

	attractors.update(weirdFactor1, weirdFactor2, numPoints, numAudio, bass, bpm, beat);
	scrolls.update(vidTexture, numAudio, numPoints, bass, bpm, beat);
	temple.update(bass, bpm, beat);

	if (bLoadPostShader) {
		reloadShaders();
		bLoadPostShader = false;
	}

	// receive bpm data
	// receive network data
	char udpMessage[100000];
	udpConnection.Receive(udpMessage,100000);
	string allMessages = udpMessage;
	if (allMessages != "") {
		vector<string> messages = ofSplitString(allMessages,"\n");
		// remove empty messages
		for (int i = 0; i < messages.size(); i++) {
			if (messages[i] == "") {
				messages.erase(messages.begin() + i);
			}
		}
		if (messages.size() > 0) {
			string lastMessage = messages[messages.size() - 1];
			vector<string> messageParts = ofSplitString(lastMessage, ";");
			string beatString = messageParts[0];
			string bpmString = messageParts[1];
			beat = ofToInt(beatString);
			bpm = ofToFloat(bpmString);
		}
	}	
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofEnableBlendMode(OF_BLENDMODE_ADD);

	vector<float> melBands = audioAnalyzer.getValues(MEL_BANDS, 0, lowSmoothing);
	float bass = ofMap(melBands[0], DB_MIN, DB_MAX, 0., 1., true);
	float treble = ofMap(melBands[melBands.size() - 1], DB_MIN, DB_MAX, 0., 1., true);
	bool isOnset = audioAnalyzer.getOnsetValue(0);
	int iIsOnset;
	if (isOnset) {
		iIsOnset = 1;
	} else {
		iIsOnset = 0;
	}

	float bpm = bpmDetector.getBPM();
	float bps = bpm / 60.0;

	fboFractals.begin();
	ofClear(0, 0, 0, 0);
	fractals.draw(audioVector.size(), pointsVector.size(), cameraRotateSpeed, cameraDist, weirdFactor1, weirdFactor2, bass, bpm, beat);
	fboFractals.end();

	fboAttractors.begin();
	ofClear(0, 0, 0, 0);
	attractors.draw(cameraDist, cameraRotateSpeed);
	fboAttractors.end();

	fboScrolls.begin();
	ofClear(0, 0, 0, 0);
	scrolls.draw();
	fboScrolls.end();

	fboTemple.begin();
	ofClear(0, 0, 0, 0);
	temple.draw();
	fboTemple.end();

	eyePlayer.update();
    if(eyePlayer.isFrameNew()){
        ofPixels & pixels = eyePlayer.getPixels();
        eyeTexture.loadData(pixels);
	}

	for (int i = 0; i < iNumScreens; i++) {
		fbos[i].begin();
		ofClear(0, 0, 0, 0);
		post_shaders[i].begin();
		post_shaders[i].setUniform2i("resolution", ofGetWidth(), ofGetHeight());
		post_shaders[i].setUniform1f("time", ofGetElapsedTimef());
		post_shaders[i].setUniformTexture("last1", lastFbos[0].getTexture(), 0);
		post_shaders[i].setUniformTexture("last2", lastFbos[1].getTexture(), 1);
		post_shaders[i].setUniformTexture("last3", lastFbos[2].getTexture(), 2);
		post_shaders[i].setUniformTexture("last4", lastFbos[3].getTexture(), 3);
		post_shaders[i].setUniformTexture("fractals", fboFractals.getTexture(), 4);
		post_shaders[i].setUniformTexture("attractors", fboAttractors.getTexture(), 5);
		post_shaders[i].setUniformTexture("scrolls", fboScrolls.getTexture(), 6);
		post_shaders[i].setUniformTexture("temple", fboTemple.getTexture(), 7);
		post_shaders[i].setUniformTexture("mask", maskImage.getTexture(), 8);
		post_shaders[i].setUniformTexture("camera", vidTexture, 9);
		post_shaders[i].setUniformTexture("eye", eyeTexture, 10);
		post_shaders[i].setUniform1f("bpm", bpm);
		post_shaders[i].setUniform1i("beat", beat);
		post_shaders[i].setUniform1f("bass", bass);
		post_shaders[i].setUniform1f("treble", treble);
		post_shaders[i].setUniform1i("sortPixels", iSortPixels);
		post_shaders[i].setUniform1i("isOnset", iIsOnset);
		post_shaders[i].setUniform1i("postType", postType);
		post_shaders[i].setUniform1f("postVar", postVar);
		ofSetColor(255);
		ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
		post_shaders[i].end();
		fbos[i].end();

		lastFbos[i].begin();
		ofClear(0, 0, 0, 0);
		fbos[i].draw(0, 0, ofGetWidth(), ofGetHeight());
		lastFbos[i].end();
	}

	if (iNumScreens == 1) {
		lastFbos[0].draw(0, 0, ofGetWidth(), ofGetHeight());
	} else if (iNumScreens == 4) {
		lastFbos[0].draw(0, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		lastFbos[1].draw(ofGetWidth() / 2, 0, ofGetWidth() / 2, ofGetHeight() / 2);
		lastFbos[2].draw(0, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);
		lastFbos[3].draw(ofGetWidth() / 2, ofGetHeight() / 2, ofGetWidth() / 2, ofGetHeight() / 2);
	}
	// fboTemple.draw(0, 0, ofGetWidth(), ofGetHeight());
}

//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& message) {

	if(message.status < MIDI_SYSEX) {
		if (message.status == MIDI_NOTE_ON) {
			// 48 - 72
			int key = message.pitch - 48;

			if (key <= 11) {
				changeArtifactType(key);
			}

			if ((key > 11) && (key < 22)) {
				postType = key - 12;
			}

			if (key == 22) {
				if (iNumScreens == 4) {
					iNumScreens = 1;
				} else if (iNumScreens == 1) {
					iNumScreens = 4;
				}
			}

			if (key == 23) {
				if (iSortPixels == 0) {
					iSortPixels = 1;
				} else if (iSortPixels == 1) {
					iSortPixels = 0;
				}
			}

			if (key == 24) {
				bLoadPostShader = true;
			}
		} else if (message.status == MIDI_NOTE_OFF) {
			int key = message.pitch - 48;
			if (key == 23) {
				iSortPixels = 0;
			}
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
	if (key >= '0' && key <= '9') {
		int num = key - '0';
		changeArtifactType(num);
	} else if (key == 's') {
		iSortPixels = 1;
	} else if (key == ' ') {
		reloadShaders();
	}
}

void ofApp::keyReleased(int key) {
	if (key == 's') {
		iSortPixels = 0;
	}
}

void ofApp::changeArtifactType(int type) {
	if (type < 4) {
		int fractalType = type;
		fractals.setFractalType(fractalType);
	}
	
	if ((type >= 4) && (type <= 11)) {
		int attractorType = type - 4;
		attractors.setAttractorType(attractorType);
	}
}

void ofApp::reloadShaders() {
	ofShader testShader;
	for (int i = 0; i < 4; i++) {
		string shaderName = "shaders/post" + ofToString(i+1) + ".frag";
		bool success = testShader.setupShaderFromFile(GL_FRAGMENT_SHADER, shaderName);
		if (success) {
			post_shaders[i].load("shaders/generic.vert", shaderName);
		}
	}
	
	attractors.reloadShaders();
	fractals.reloadShaders();
	scrolls.reloadShaders();
}
