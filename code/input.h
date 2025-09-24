// EXPLANATION:
// Helps handle game input

#ifndef ASTEROIDS_INPUT_HEADER_GUARD
#define ASTEROIDS_INPUT_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------
#define INPUT_ACTIONS_COUNT 32 // Maximum number of game actions, e.g. confirm, pause, move up
#define INPUT_MAX_MAPS 32 // Maximum number of inputs that can be mapped to an action
#define INPUT_MAX_TOUCH_POINTS 10

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

typedef struct InputMappings {
    KeyboardKey keyMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
    MouseButton mouseMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
    bool virtualButtonMap[INPUT_ACTIONS_COUNT];
} InputMappings;

// Prototypes
// ----------------------------------------------------------------------------
// Input Actions
void InitDefaultInputControls(void); // Sets the default key mapping control scheme
bool IsInputKeyModifier(KeyboardKey key);
bool IsInputActionPressed(InputAction action);
bool IsInputActionDown(InputAction action);
bool IsInputActionMouseDown(InputAction action);

// Touch / Virtual Input
void SetVirtualInput(InputAction action, bool buttonPressed);
int CheckCollisionTouchCircle(Vector2 center, float radius);
int CheckCollisionTouchRec(Rectangle rec);

// Helpers
int UpdateInputTouchPoints(void);
Vector2 GetScaledMousePosition(void);

#endif // ASTEROIDS_INPUT_HEADER_GUARD
