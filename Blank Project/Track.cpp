#include "Track.h"


//traverse track and update object position
template <class T>
void Track<T>::traverseTrack() {
	if (numPoints == 0) {
		return;
	}
	//get current time
	float time = (float)Window::GetTimeInSeconds();
	//calculate current point
	int currentPoint = (int)(time / duration) % numPoints;
	//calculate next point
	int nextPoint = (currentPoint + 1) % numPoints;
	//calculate interpolation factor
	float factor = fmod(time, duration) / duration;
	//calculate new position
	Vector3 newPos = trackPoints[currentPoint] * (1 - factor) + trackPoints[nextPoint] * factor;
	//set object position
	object->SetPosition(newPos);

}