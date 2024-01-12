#include "rectTree.h"

#include "ColorWheelScheme.h"
#include "ColorWheelSchemes.h"

//--------------------------------------------------------------
RectTree::RectTree(){

    Rect rect;
    int numDims = 3;

    if (numDims == 2) {
        rect.origin = {0, 0};
        rect.size = {ofGetWidth(), ofGetHeight()};
        rect.drift = {0, 0};
    } else if (numDims == 3) {
        rect.origin = {0, 0, 0};
        rect.size = {100, 100, 100};
        rect.drift = {0, 0, 0};
    }
    rect.stackDepth = 0;
    root = make_shared<Rect>(rect);
}


//--------------------------------------------------------------
void RectTree::update(){
    recurRects();
}

void RectTree::recurRects() {
    std::stack<shared_ptr<Rect>> rects;
    rects.push(root);

    while (!rects.empty()) {
        shared_ptr<Rect> rect = rects.top();
        rects.pop();
        if (rect->left == nullptr && rect->right == nullptr) {
            if (rect->stackDepth > 10) {
                continue;
            }
            if (ofRandom(1) > 0.1) {
                continue;
            }
            float dirRand = ofRandom(1);
            int numDims = rect->size.size();
            for (int i = 0; i < numDims; i++) {
                if ((dirRand > float(i) / numDims) && (dirRand < float(i + 1) / numDims)) {
                    float share = ofRandom(0.1, 0.9);
                    vector<float> leftOrigin = rect->origin;
                    vector<float> leftSize = rect->size;
                    vector<float> rightOrigin = rect->origin;
                    vector<float> rightSize = rect->size;
                    leftSize[i] *= share;
                    rightSize[i] *= (1 - share);
                    rightOrigin[i] += leftSize[i];
                    rect->left = make_shared<Rect>(createRect(leftOrigin, leftSize, rect->stackDepth + 1));
                    rect->right = make_shared<Rect>(createRect(rightOrigin, rightSize, rect->stackDepth + 1));
                }
            }
        } else {
            if (rect->left != nullptr) {
                rects.push(rect->left);
            }
            if (rect->right != nullptr) {
                rects.push(rect->right);
            }
        }
    }
}

Rect RectTree::createRect(vector<float> origin, vector<float> size, int stackDepth) {
    Rect rect;
    rect.origin = origin;
    rect.size = size;
    rect.drift.resize(size.size());
    rect.stackDepth = stackDepth;

    float noiseFactor = 0.1;
    float noise = ofNoise(origin[0] * noiseFactor, origin[1] * noiseFactor, origin[2] * noiseFactor);
    if (noise > 0.6) {
        rect.wireframe = true;
    } else {
        rect.wireframe = false;
    }

    int colorScheme = 1;
    int numColors = 5;
	ofColor primaryColor = ofColor(0, 0, 0);
	shared_ptr<ofxColorTheory::ColorWheelScheme> scheme = ofxColorTheory::ColorWheelSchemes::SCHEMES.at(colorScheme);
    scheme->setPrimaryColor(primaryColor);
    vector<ofColor> colors = scheme->interpolate(numColors);
    rect.color = colors.at(int(noise * numColors));

    return rect;
}

//--------------------------------------------------------------
void RectTree::draw(){
    // ofClear(0);
    // ofBackground(0);
    // ofSetColor(255);

    if (root->size.size() == 2) {
        drawRects();
    } else if (root->size.size() == 3) {
        drawRects();
    }
}

void RectTree::drawRects() {
    std::stack<shared_ptr<Rect>> rects;
    rects.push(root);

    while (!rects.empty()) {
        shared_ptr<Rect> rect = rects.top();
        rects.pop();
        if (rect->left == nullptr && rect->right == nullptr) {
            drawRect(rect);
        } else {
            if (rect->left != nullptr) {
                rects.push(rect->left);
            }
            if (rect->right != nullptr) {
                rects.push(rect->right);
            }
        }
    }
}

void RectTree::drawRect(shared_ptr<Rect> rect) {
    if (rect->size.size() == 2) {
        ofNoFill();
        ofDrawRectangle(rect->origin[0], rect->origin[1], rect->size[0], rect->size[1]);
        ofFill();
    } else if (rect->size.size() == 3) {
        if (rect->wireframe) {
            ofNoFill();
        }
        ofSetColor(rect->color);
        for (int i = 0; i < rect->drift.size(); i++) {
            if (ofRandom(1) < 0.1) {
                rect->drift[i] += rect->size[i] * ofRandom(-0.01, 0.01);
            }
            rect->drift[i] = ofClamp(rect->drift[i], rect->size[i] * -0.2, rect->size[i] * 0.2);
        }
        ofDrawBox(
            rect->origin[0] + (rect->size[0] / 2) + rect->drift[0], rect->origin[1] + (rect->size[1] / 2) + rect->drift[1], rect->origin[2] + (rect->size[2] / 2) + rect->drift[2], 
            rect->size[0], rect->size[1], rect->size[2]
        );
        // ofDrawBox(rect->origin[0], rect->origin[1], rect->origin[2], rect->size[0], rect->size[1], rect->size[2]);
        if (rect->wireframe) {
            ofFill();
        }
    }
}
