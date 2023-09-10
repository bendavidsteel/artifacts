#include "scrolls.h"

void Scrolls::setup() {
    numRows = 60;
    numCols = 250;

    brightnessOrder = " `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@";

    ofBuffer buffer = ofBufferFromFile("assets/fungi_from_yuggoth.txt");
	for (auto line : buffer.getLines()){
		fungiLines.push_back(ofToUpper(line));
	}

    fungiLineOffset = 0;
    fungiStartPos = int(ofRandom(10, numCols - 100));

	textStartPos = -9999999;

    intensityShader.load("shaders/scrolls/generic.vert", "shaders/scrolls/intensity.frag");
    fboIntensity.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA16);

    textLines.resize(numRows);
    for (int i = 0; i < numRows; i++) {
        textLines[i].resize(numCols);
    }

    pixels.allocate(ofGetWidth(), ofGetHeight(), OF_IMAGE_COLOR);
}

void Scrolls::update(ofTexture vidTexture, int numAudio, int numPoints, float bass) {
    float time = ofGetElapsedTimef();

    fboIntensity.begin();
    ofClear(0, 0, 0, 255);
    intensityShader.begin();
    intensityShader.setUniform1f("time", time);
    intensityShader.setUniform2i("resolution", ofGetWidth(), ofGetHeight());
    intensityShader.setUniform1i("numAudio", numAudio);
    intensityShader.setUniform1i("numPoints", numPoints);
    intensityShader.setUniform1f("bass", bass);
    intensityShader.setUniformTexture("camera", vidTexture, 0);
    ofSetColor(255);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
	intensityShader.end();
	fboIntensity.end();

    fboIntensity.readToPixels(pixels);

    for (int i = 0; i < numRows; i++) {
        int y = ofMap(i, 0, numRows, 0, ofGetHeight());
        for (int j = 0; j < numCols; j++) {
            int x = ofMap(j, 0, numCols, 0, ofGetWidth());
            ofColor color = pixels.getColor(x, y);
            float brightness = color.getBrightness();
            int index = ofMap(brightness, 0, 255, 0, brightnessOrder.size() - 1);
            char c = brightnessOrder[index];
            textLines[i][j] = c;
        }
    }

    if (int(fungiLineOffset) + textLines.size() >= fungiLines.size()) {
        fungiLineOffset = 0.;
        fungiStartPos = int(ofRandom(10, numCols - 100));
    }

    fungiBufferSize = 5;
    for (int i = 0; i < numRows; i++) {
        string fungiLine = fungiLines[i + int(fungiLineOffset)];
        for (int j = fungiStartPos; j < fungiStartPos + fungiBufferSize; j++) {
            textLines[i][j] = ' ';
        }
        for (int j = fungiStartPos + fungiBufferSize; j < fungiStartPos + fungiBufferSize + fungiLine.size(); j++) {
            textLines[i][j] = fungiLine[j - fungiStartPos - fungiBufferSize];
        }
        for (int j = fungiStartPos + fungiBufferSize + fungiLine.size(); j < fungiStartPos + 2 * fungiBufferSize + fungiLine.size(); j++) {
            textLines[i][j] = ' ';
        }
    }
    fungiLineOffset += 0.2;
}

void Scrolls::draw() {
    // fboIntensity.draw(0, 0, ofGetWidth(), ofGetHeight());
    int lineSep, lineSpeed;
    lineSep = 20;
    lineSpeed = 2;

    for (int i = 0; i < textLines.size(); i++) {
        string line = textLines[i];
        ofDrawBitmapString(line, 0, i * lineSep);
    }
}