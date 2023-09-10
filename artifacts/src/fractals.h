#pragma once

#include "ofMain.h"

class Fractals {
    public:
        void setup();
        void update();
        void draw(int numAudio, int numPoints, float cameraRotateSpeed, float cameraDist, float weirdFactor1, float weirdFactor2, float bass);
        void reloadShaders();
        void setFractalType(int fractalType);

    private:
        ofShader shader;
        int fractalType;
};