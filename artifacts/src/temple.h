#pragma once

#include "ofMain.h"

#include "rectTree.h"

class Temple {

	public:
		void setup();
		void update(float bass, float bpm, int beat);
		void draw();

		void drawRoom(float time);
		void drawSmallBoxes(float time);

		ofLight light;
		ofCamera cam;

		vector<ofVec3f> boxPositions;
		vector<ofVec3f> boxSizes;
		vector<RectTree> rectTrees;

		int numZones;
		int numBoxesPerZone;
};
