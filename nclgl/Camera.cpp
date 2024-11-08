#include "Camera.h"
#include "Window.h"
#include <algorithm>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


void Camera::UpdateCamera(float dt) {
	pitch -= (Window::GetMouse()->GetRelativePosition().y);
	yaw -= (Window::GetMouse()->GetRelativePosition().x);

	pitch = std::min(90.0f, pitch);
	pitch = std::max(-90.0f, pitch);

	if (yaw < 0.0f) {
		yaw += 360.0f;
	}

	if (yaw > 360.0f) {
		yaw -= 360.0f;
	}

	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));

	Vector3 forward = rotation * Vector3(0, 0, -1);
	Vector3 right = rotation * Vector3(1, 0, 0);

	float speed = 100.0f * dt;

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
		position += forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
		position -= forward * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
		position -= right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		position += right * speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
		position.y += speed;
	}

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
		position.y -= speed;
	}
}


Matrix4 Camera::BuildViewMatrix() {
	
	return Matrix4::Rotation(- pitch, Vector3(1, 0, 0)) * Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * Matrix4::Translation(-position);

}

void Camera::setDirection(Vector3 directionNorm) {
	directionNorm.Normalise(); // Make sure the direction is normalized

	pitch = std::asin(directionNorm.y) * 180.0f / M_PI; // Convert radians to degrees
	yaw = std::atan2(-directionNorm.x, -directionNorm.z) * 180.0f / M_PI;

	// Clamp pitch to avoid gimbal lock
	pitch = std::min(90.0f, std::max(-90.0f, pitch));

	// Normalize yaw within [0, 360] range
	if (yaw < 0.0f) {
		yaw += 360.0f;
	}
	else if (yaw > 360.0f) {
		yaw -= 360.0f;
	}
}

Vector3 Camera::getDirection() {
	Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0)) * Matrix4::Rotation(pitch, Vector3(1, 0, 0));
	Vector3 direction = Vector3(rotation.values[8], rotation.values[9], rotation.values[10]);
	return -direction;
}

