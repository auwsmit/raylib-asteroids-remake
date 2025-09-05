// EXPLANATION:
// Helps handle game input

#ifndef ASTEROIDS_INPUT_HEADER_GUARD
#define ASTEROIDS_INPUT_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------
#define INPUT_ACTIONS_COUNT 32 // Maximum number of game actions, e.g. confirm, pause, move up
#define INPUT_MAX_KEYS 32 // Maximum number of keys that can be assigned to an action

// Types and Structures
// ----------------------------------------------------------------------------
typedef enum InputAction
{
    INPUT_ACTION_FULLSCREEN,
    INPUT_ACTION_CONFIRM,
    INPUT_ACTION_BACK,
    INPUT_ACTION_MENU_UP,
    INPUT_ACTION_MENU_DOWN,
    INPUT_ACTION_PAUSE,

    INPUT_ACTION_FORWARD,
    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,
} InputAction;

typedef struct InputKeyMaps
{
    KeyboardKey keyMaps[INPUT_ACTIONS_COUNT][INPUT_MAX_KEYS];
} InputKeyMaps;

// Prototypes
// ----------------------------------------------------------------------------
void InitDefaultInputControls(void); // Sets the default key mapping control scheme
bool IsInputKeyModifier(KeyboardKey key);
bool IsInputActionPressed(InputAction action);
bool IsInputActionDown(InputAction action);
void HandleToggleFullscreen(void);

#endif // ASTEROIDS_INPUT_HEADER_GUARD
