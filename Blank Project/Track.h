#pragma once
#include <vector>
#include "../nclgl/Vector3.h"
#include "../nclgl/window.h"

class Light;
class Camera;

// templated type for object that will be on the track
template <class T>

class Track
{
protected:
	Vector3* startPos;
	Vector3* endPos;

	std::vector<Vector3> trackPoints;

	int numPoints;
	int currentPoint;

	float duration;
	float elapsedTime;

	float progress;
	bool looping;
	
	T* object;

public:

	Track(Vector3* start, Vector3* end, T* obj) {
		startPos = start;
		endPos = end;
		object = obj;
		numPoints = 2;
		currentPoint = 0;
		duration = 3.0f;
		elapsedTime = 0.0f;
		progress = 0.0f;
		looping = false;
		initTrack();
	}

	~Track() {
		delete startPos;
		delete endPos;
	}

	void addPoint(Vector3 point) {
		//remove old end point
		trackPoints.pop_back();
		//add new point
		trackPoints.push_back(point);
		//add end point back
		trackPoints.push_back(*endPos);
		numPoints++;
	}

	Vector3 getPoint(int i) { return trackPoints[i];}

	Vector3 getStart() { return *startPos; }

	Vector3 getEnd() { return *endPos;  }

	int getNumPoints() { return numPoints;  }

	T* getObject() { return object; }

	int getCurrentPoint() { return currentPoint; }

	void setCurrentPoint(int i) { currentPoint = i; }

	vector<Vector3> getTrackPoints() { return trackPoints; }

	void initTrack() {
		// add the start and end points
		trackPoints.push_back(*startPos);
		trackPoints.push_back(*endPos);
	}

	bool isComplete() {return progress >= 1.0f;}

	bool isLooping() {return looping;}

	void toggleLooping() {looping = !looping;}

	void resetTrack() {
		object->SetPosition(*startPos);
		elapsedTime = 0.0f;
		progress = 0.0f;
		currentPoint = 0;
	}


	//traverse track and update object position
	void Track<T>::traverseTrack(float dt) {
		if (progress >= 1.0f) {
			currentPoint++;
			progress = 0.0f;
			elapsedTime = 0.0f;
			progress = 0.0f;
		}
		if (numPoints == 0 || numPoints-1 == currentPoint) {
			return;
		}
		

		// Update elapsed time
		elapsedTime += (dt);

		// Calculate progress as a percentage (from 0 to 1)
		progress = elapsedTime / duration;
		progress = std::min(progress, 1.0f); // Clamp to 1.0 to avoid overshooting

		// Interpolate between start and end positions
		Vector3 newPosition = (getTrackPoints()[getCurrentPoint()]) * (1.0f - progress) + (getTrackPoints()[getCurrentPoint()+1]) * progress;
		std::cout << "new position: " << newPosition << "\n";

		// Update object position
		object->SetPosition(newPosition);
	}
};

// polymorphic class 'DirectionalTrack' that inherits from 'Track'
template <class T>
class DirectionalTrack : public Track<T>
{
protected:

	Vector3 startTarget;
	Vector3 endTarget;

	vector<Vector3> targets;

	int numTargets;
	int currentTarget;

public:
	// Constructor for DirectionalTrack, inherits and initializes Track constructor
	DirectionalTrack(Vector3* start, Vector3* end, Vector3 startTarget, Vector3 endTarget, T* obj):Track<T>(start, end, obj){
		this->startTarget = startTarget;
		this->endTarget = endTarget;
		numTargets = 0;
		currentTarget = 0;
	}

	~DirectionalTrack() {
		delete startTarget;
		delete endTarget;
	}

	void initTargets() {
		targets.push_back(*startTarget);
		targets.push_back(*endTarget);
		numTargets = 2;
	}

	void addTarget(Vector3 target) {
		targets.pop_back();
		targets.push_back(target);
		targets.push_back(*endTarget);
		numTargets++;
	}

	void faceTarget() {
	Vector3 directionToTarget = (startTarget - object->GetPosition());
	directionToTarget.Normalise();
	object->setDirection(directionToTarget);
	}


};

