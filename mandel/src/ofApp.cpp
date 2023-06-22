#include "ofApp.h"
#include "ofConstants.h"

//--------------------------------------------------------------
void ofApp::setup(){
	fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

	renderer.load("generic.vert", "fractals.frag");

}

//--------------------------------------------------------------
void ofApp::update(){ }

//--------------------------------------------------------------
void ofApp::draw() {
	fbo.begin();
	ofClear(255,255,255, 0);
	renderer.begin();
	renderer.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
	renderer.setUniform1f("time", ofGetElapsedTimef());
	renderer.setUniform1i("fractal_type", 1);
	renderer.setUniform1f("scale", 1.0);
	renderer.setUniform1f("transform_1", 0.0);
	renderer.setUniform1f("transform_2", 0.0);
	renderer.setUniform1f("ambient", 0.0);
	renderer.setUniform1f("camera_amp", 0.0);
	renderer.setUniform1f("camera_speed", 0.1);
	ofSetColor(255);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	renderer.end();
	fbo.end();
	fbo.draw(0, 0, ofGetWidth(), ofGetHeight());

	ofDrawBitmapString(ofGetFrameRate(),20,20);
}

void ofApp::exit(){

	
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}