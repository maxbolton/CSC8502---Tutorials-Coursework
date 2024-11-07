#pragma once
#include <vector>
#include "../nclgl/Vector3.h"




// templated type for object that will be on the track
template <class T>

class Track
{
protected:
	Vector3* startPos;
	Vector3* endPos;

	std::vector<Vector3> trackPoints;
	int numPoints;

	int duration;
	
	T* object;

public:

	Track(Vector3* start, Vector3* end, T* obj) {
		startPos = start;
		endPos = end;
		object = obj;
		numPoints = 0;
		duration = 3;
		populateTrack();
	}

	~Track() {
		delete startPos;
		delete endPos;
	}

	void addPoint(Vector3 point) {
		trackPoints.push_back(point);
		numPoints++;
	}

	Vector3 getPoint(int i) {
		return trackPoints[i];
	}

	Vector3 getStart() {
		return *startPos;
	}

	Vector3 getEnd() {
		return *endPos;
	}

	int getNumPoints() {
		return numPoints;
	}

	T* getObject() {
		return object;
	}

	void populateTrack() {
		// populate the track with points
		// for now just add the start and end points
		addPoint(*startPos);
		addPoint(*endPos);
	}

	void traverseTrack();
	void occilateObject();

};

