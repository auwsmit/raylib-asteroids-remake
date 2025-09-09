// EXPLANATION:
// Helps handle game input

#ifndef ASTEROIDS_INPUT_HEADER_GUARD
#define ASTEROIDS_INPUT_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------
#define INPUT_ACTIONS_COUNT 32 // Maximum number of game actions, e.g. confirm, pause, move up
#define INPUT_MAX_MAPS 32 // Maximum number of inputs that can be mapped to an action

// These are needed because MOUSE_LEFT_BUTTON is 0, which is the default null mapping value
#define INPUT_MOUSE_NULL 7
#define INPUT_MOUSE_LEFT_BUTTON 8

// Types and Structures
// ----------------------------------------------------------------------------
typedef enum InputAction {
    INPUT_ACTION_FULLSCREEN,
    INPUT_ACTION_CONFIRM,
    INPUT_ACTION_BACK,
    INPUT_ACTION_MENU_UP,
    INPUT_ACTION_MENU_DOWN,
    INPUT_ACTION_PAUSE,

    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,
    INPUT_ACTION_FORWARD,
    INPUT_ACTION_SHOOT,
} InputAction;

typedef struct InputMappings {
    KeyboardKey keyMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
    MouseButton mouseMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_MAPS];
} InputMappings;

// Prototypes
// ----------------------------------------------------------------------------
void InitDefaultInputControls(void); // Sets the default key mapping control scheme
bool IsInputKeyModifier(KeyboardKey key);
bool IsInputActionPressed(InputAction action);
bool IsInputActionDown(InputAction action);
Vector2 GetScaledMousePosition(void);
void HandleToggleFullscreen(void);

#endif // ASTEROIDS_INPUT_HEADER_GUARD
