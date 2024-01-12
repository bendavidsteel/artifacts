#pragma once

#include "ofMain.h"

struct Rect {
	vector<float> origin;
	vector<float> size;
	vector<float> drift;
	shared_ptr<Rect> left, right;
	bool wireframe;
	ofColor color;
	int stackDepth;
};

class RectTree {

	public:
		RectTree();
		void update();
		void draw();

		Rect createRect(vector<float> origin, vector<float> size, int stackDepth);
		void recurRects();
		void drawRects();
		void drawRect(shared_ptr<Rect> rect);

		shared_ptr<Rect> root;
};
