// EXPLANATION:
// Helps handle game input

#ifndef ASTEROIDS_INPUT_HEADER_GUARD
#define ASTEROIDS_INPUT_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------
#define INPUT_ACTIONS_COUNT 16 // Maximum number of game actions, e.g. confirm, pause, move up
#define INPUT_MAX_MAPS 24 // Maximum number of inputs that can be mapped to an action
#define INPUT_MAX_TOUCH_POINTS 8

// These are needed because MOUSE_LEFT_BUTTON is 0, which is the default null mapping value
#define INPUT_MOUSE_LEFT_BUTTON 7
#define INPUT_MOUSE_NULL 8

// Types and Structures
// ----------------------------------------------------------------------------
typedef enum InputAction {
    INPUT_ACTION_FULLSCREEN,
    INPUT_ACTION_CONFIRM,
    INPUT_ACTION_CANCEL,
    INPUT_ACTION_MENU_UP,
    INPUT_ACTION_MENU_DOWN,
    INPUT_ACTION_PAUSE,

    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,
    INPUT_ACTION_THRUST,
    INPUT_ACTION_SHOOT,
} InputAction;

typedef struct TouchState {
    Vector2 position;
    bool pressedPreviousFrame;
    bool pressedCurrentFrame;
    int currentButton; // TODO this should probably just be a bool
                       // ... actually the touch screen input UI stuff just needs a rewrite
} TouchState;

typedef struct InputState {
    KeyboardKey keyMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
    MouseButton mouseMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
    // GamepadButton gamepadButtonMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS]; // TODO
    bool touchButtonPressed[INPUT_ACTIONS_COUNT]; // for touch screen buttons
    TouchState touchPoints[INPUT_MAX_TOUCH_POINTS];
} InputState;

// Prototypes
// ----------------------------------------------------------------------------
// Input Actions
void InitDefaultInputControls(void); // Sets the default key mapping control scheme
bool IsInputKeyModifier(KeyboardKey key);
bool IsInputActionPressed(InputAction action);
bool IsInputActionMousePressed(InputAction action);
bool IsInputActionDown(InputAction action);
bool IsInputActionMouseDown(InputAction action);

// Touch / Virtual Input
void SetTouchInput(InputAction action, bool isButtonPressed);
void SetTouchPointButton(int index, int buttonIdx); // Set a touch point's current button id (currently used for touch screen analog stick, which probably needs a redesign/rewrite)
bool IsTouchTapped(int index); // Check if a touch point was tapped (works like IsKeyPressed)
bool IsTouchPressingButton(int index); // Check if a touch point is pressing any button
int CheckCollisionTouchCircle(Vector2 center, float radius); // Check if any touch points are within a circle, returns index to touch point or -1
int CheckCollisionTouchRec(Rectangle rec); // Check if any touch points are within a rectangle, returns index to touch point or -1

// Helpers
// int CheckAvailableGamepads(void); // TODO
int UpdateInputTouchPoints(void); // Update touch point info for each frame
Vector2 GetScaledMousePosition(void); // Scale mouse position to the game camera

#endif // ASTEROIDS_INPUT_HEADER_GUARD
