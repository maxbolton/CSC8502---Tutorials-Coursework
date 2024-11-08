#include "../nclgl/window.h"
#include "Renderer.h"
#include <stdio.h>

using namespace std;

int main() {
    Window w("Max Bolton - CSC8502", 1280, 720, false);
    if (!w.HasInitialised()) {
        return -1;
    }

    Renderer renderer(w);
    if (!renderer.HasInitialised()) {
        return -1;
    }

    w.LockMouseToWindow(true);
    w.ShowOSPointer(false);


    while (w.UpdateWindow() &&
        !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)) {
        
        renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());
        renderer.RenderScene();
        renderer.SwapBuffers();

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			renderer.lightPositive();
		}
        if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_2)) {
        renderer.lightNegative();
        }
        if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_3)) {
			renderer.idkBruh();
        }
    }
    return 0;
}
