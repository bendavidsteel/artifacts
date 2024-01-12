#include "temple.h"

//--------------------------------------------------------------
void Temple::setup(){
    ofEnableLighting();
    ofEnableDepthTest();
    light.setup();
    light.enable();
    light.setPosition(0,800,200);

    cam.setFarClip(ofGetWidth()*100);
	cam.setNearClip(0.1);
    cam.disableOrtho();
    cam.setFov(20);

    numZones = 10;
    numBoxesPerZone = 4;
}

//--------------------------------------------------------------
void Temple::update(float bass, float bpm, int beat){
    int radius = ofGetHeight()/2;
    int size = 100;
    int separation = size + 200;
    // float remainder = std::fmod(100 * time, separation);

    int columnWidth = 200;

    int x_gap = 5;
    int y_gap = 20;
    int z_gap = 100;
    // move all boxes forward
    for (int i = 0; i < boxPositions.size(); i++) {
        boxPositions[i].z += (size + z_gap) / bpm;
        if (ofRandom(1) < 0.1) {
            rectTrees[i].update();
        }
    }

    // add new boxes at the edge of view
    float boxStart = -1 * (size + z_gap) * numZones;
    if ((boxPositions.size() == 0) || (boxPositions.back().z > boxStart + (size + z_gap))) {
        // ofVec3f pos1 = ofVec3f(x_gap + size / 2, y_gap + size / 2, boxStart);
        // ofVec3f pos2 = ofVec3f(x_gap + size / 2, -y_gap - size / 2, boxStart);
        // ofVec3f pos3 = ofVec3f(-x_gap - size / 2, y_gap + size / 2, boxStart);
        // ofVec3f pos4 = ofVec3f(-x_gap - size / 2, -y_gap - size / 2, boxStart);
        ofVec3f pos1 = ofVec3f(x_gap, y_gap, boxStart);
        ofVec3f pos2 = ofVec3f(x_gap, -y_gap - size, boxStart);
        ofVec3f pos3 = ofVec3f(-x_gap - size, y_gap, boxStart);
        ofVec3f pos4 = ofVec3f(-x_gap - size, -y_gap - size, boxStart);

        boxPositions.push_back(pos1);
        rectTrees.emplace_back(RectTree());

        boxPositions.push_back(pos2);
        rectTrees.emplace_back(RectTree());

        boxPositions.push_back(pos3);
        rectTrees.emplace_back(RectTree());

        boxPositions.push_back(pos4);
        rectTrees.emplace_back(RectTree());
    }

    // remove boxes that have moved behind the camera
    if (boxPositions.front().z > size) {
        for (int j = 0; j < numBoxesPerZone; j++) {
            boxPositions.erase(boxPositions.begin());
            rectTrees.erase(rectTrees.begin());
        }
    }
}

//--------------------------------------------------------------
void Temple::draw(){
    float time = ofGetElapsedTimef();

    cam.setPosition(0, 0, 0);

    cam.lookAt(ofVec3f(10 * sin(time), 10 * cos(time), -1000));

    cam.begin();
    // ofSetColor(125,125,125);
    ofSetBackgroundColor(0);

    // drawSmallBoxes(time);
    drawRoom(time);

    cam.end();
}

void Temple::drawRoom(float time) {
    
    for (int i = 0; i < numZones; i++) {
        for (int j = 0; j < 4; j++) {
            if (i*4 + j < boxPositions.size()) {
                ofSetColor(255);
                // ofVec3f pos1 = boxPositions[i*4 + j];
                // shader.begin();
                ofPushMatrix();
                ofTranslate(boxPositions[i*4 + j]);
                // // shader.setUniform3f("positionOffset", pos1.x, pos1.y, pos1.z);
                // shader.setUniform3f("positionOffset", 0, 0, -100);
                rectTrees[i*4 + j].draw();
                // ofRotateDeg(30, 1, 0, 0);
                // ofDrawBox(0, 0, 0, 100);

                ofPopMatrix();
                
                // shader.end();
            }
        }
    }
}

void Temple::drawSmallBoxes(float time) {
    int radius = ofGetHeight()/4;
    int separation = 500;
    float remainder = std::fmod(100 * time, separation);
    int size = 100;

    for (int i = separation; i > -100 * separation; i -= separation) {
        float z = i + remainder;
        ofDrawBox(radius, radius, z, size);
        ofDrawBox(radius, -radius, z, size);
        ofDrawBox(-radius, radius, z, size);
        ofDrawBox(-radius, -radius, z, size);
    }
}
