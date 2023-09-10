#include "fractals.h"

void Fractals::setup() {
    shader.load("shaders/fractals/generic.vert", "shaders/fractals/fractals.frag");

    fractalType = 0;
}

void Fractals::update() {}

void Fractals::draw(int numAudio, int numPoints, float cameraRotateSpeed, float cameraDist, float weirdFactor1, float weirdFactor2, float bass) {
    shader.begin();
    shader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform1i("fractal_type", fractalType);
    shader.setUniform1f("scale", 1.0);
    shader.setUniform1f("transform_1", weirdFactor1);
    shader.setUniform1f("transform_2", weirdFactor2);
    shader.setUniform1f("bass", bass);
    shader.setUniform1f("camera_amp", cameraDist);
    shader.setUniform1f("camera_speed", cameraRotateSpeed);
    shader.setUniform1i("numAudio", numAudio);
    shader.setUniform1i("numPoints", numPoints);
    ofSetColor(255);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    shader.end();
}

void Fractals::reloadShaders() {
    shader.load("shaders/fractals/generic.vert", "shaders/fractals/fractals.frag");
}

void Fractals::setFractalType(int fType) {
    fractalType = fType;
}