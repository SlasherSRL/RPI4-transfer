//------------------------------------------------------------------------------
//  @file gamepad.cc
//  @copyright (C) 2022 Individual contributors, see AUTHORS file
//------------------------------------------------------------------------------
#include "config.h"
#include "GLFW/glfw3.h"
#include "gamepad.h"
float ClampAxis(float val) {
    if (val > 1.0f) return 1.0f;
    if (val < -1.0f) return -1.0f;
    return val;
}
namespace Input
{
	void Gamepad::UpdateGamePad()
	{
        static bool prevPressed[Buttons::NumButtons] = {};

        if (!glfwJoystickPresent(GLFW_JOYSTICK_1)) return;

        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);

        const int maxMapped = sizeof(Buttons::GLFW_BUTTON_TO_ENUM) / sizeof(Buttons::ButtonName);
        for (int i = 0; i < buttonCount && i < maxMapped; ++i)
        {
            Buttons::ButtonName mapped = Buttons::GLFW_BUTTON_TO_ENUM[i];
            pressed[mapped] = (buttons[i] == GLFW_PRESS);
        }

        // Calculate "released"
        for (int i = 0; i < Buttons::NumButtons; ++i)
        {
            released[i] = prevPressed[i] && !pressed[i];
            prevPressed[i] = pressed[i];
        }

        // Joystick axes
        int axisCount;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axisCount);

        if (axisCount >= 2) {
            leftStick.x = ClampAxis(axes[0]);
            leftStick.y = ClampAxis(axes[1]);
        }
        if (axisCount >= 4) {
            rightStick.x = ClampAxis(axes[2]);
            rightStick.y = ClampAxis(axes[3]);
        }
        if (axisCount >= 6) { // Assuming leftTrigger and rightTrigger are the 5th and 6th axis
            leftTrigger = (axes[4] + 1.0f) / 2.0f;  // Convert to 0.0 to 1.0
            rightTrigger = (axes[5] + 1.0f) / 2.0f; // Convert to 0.0 to 1.0
        }
        pressed[Buttons::LT] = leftTrigger > 0.5f;
        pressed[Buttons::RT] = rightTrigger > 0.5f;
	}
} // namespace Input
