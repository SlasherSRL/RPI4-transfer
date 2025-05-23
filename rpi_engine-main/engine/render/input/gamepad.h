#pragma once
//------------------------------------------------------------------------------
/**
    @file

    @copyright
    (C) 2022 Individual contributors, see AUTHORS file
*/
//------------------------------------------------------------------------------


namespace Input
{
    class Buttons
    {
    public:
        enum ButtonName
        {
            A,
            B,
            X,
            Y,
            LB,
            RB,
            Select,
            Start,
            LS,
            RS,
            D_PADLeft,
            D_PADRight,
            D_PADUp,
            D_PADDown,
            LT,
            RT,

            NumButtons,
            InvalidButton
        };

        static constexpr ButtonName GLFW_BUTTON_TO_ENUM[] = {
            A,        // 0
            B,        // 1
            X,        // 2
            Y,        // 3
            LB,       // 4
            RB,       // 5
             Select,   // 6 (Back)
            Start, // 7
            LS,       // 8
            RS,       // 9
            D_PADUp,     // 10
            D_PADRight,  // 11
            D_PADDown,   // 12
            D_PADLeft    // 13
            // Extend as needed based on your controller
        };

        struct Joystick
        {
            float x = 0.0f; // -1.0 to 1.0
            float y = 0.0f;
        };
    };
   
    struct Gamepad
    {
        bool pressed[Buttons::NumButtons] = {};
        bool released[Buttons::NumButtons] = {};
        Buttons::Joystick leftStick;
        Buttons::Joystick rightStick;
        float leftTrigger = 0.0f;
        float rightTrigger = 0.0f;
        void UpdateGamePad();
    };
   

} // namespace Input
