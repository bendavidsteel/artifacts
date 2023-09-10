#pragma once

#include "ofMain.h"

class Scrolls {
    public:
        void setup();
        void update(ofTexture vidTexture, int numAudio, int numPoints, float bass);
        void draw();

    private:
        vector<string> textLines;
		vector<ofColor> textColors;
		vector<bool> textHighlight;
		int textStartPos;
		float textType;
		float textPos;

        string brightnessOrder;

        ofPixels pixels;

        int numRows;
        int numCols;

        ofShader intensityShader;
        ofFbo fboIntensity;

        vector<string> fungiLines;
        int fungiStartPos;
        int fungiBufferSize;
        float fungiLineOffset;
};