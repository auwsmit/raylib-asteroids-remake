// EXPLANATION:
// For managing the user interface

#ifndef ASTEROIDS_MENU_HEADER_GUARD
#define ASTEROIDS_MENU_HEADER_GUARD

#include "raylib.h"

// Macros
// ----------------------------------------------------------------------------

#define UI_BUTTON_PADDING 20 // area around text to click
#define UI_TRANSPARENCY 0.10f

// Size of UI elements
#define UI_TITLE_SIZE        150   // title font size
#define UI_TITLE_BUTTON_SIZE 80    // title menu button font size
#define UI_CURSOR_SIZE       20.0f // cursor triangle size
#define UI_FONT_SIZE_CENTER  140   // center of screen font size
#define UI_FONT_SIZE_EDGE    75    // top of screen font size

// UI spacing
#define UI_TITLE_TOP_PADDING 180 // space from the top of the screen
#define UI_TITLE_SPACING     200 // space between the first option and title text
#define UI_BUTTON_SPACING    50  // space between each button
#define UI_EDGE_PADDING      70  // space from screen edges

// Virtual input
#define UI_STICK_RADIUS 100.0f

// Types and Structures
// ----------------------------------------------------------------------------

typedef enum UiMenuState {
    UI_MENU_TITLE, UI_MENU_PAUSE, UI_MENU_GAMEPLAY
} UiMenuState;

typedef enum UiTitleMenuId {
    UI_BID_START, UI_BID_EXIT
} UiTitleMenuId;

typedef enum UiGameplayButtonId {
    UI_BID_PAUSE, UI_BID_SHOOT, UI_BID_THRUST
} UiGameplayButtonId;

typedef enum UiPauseMenuId {
    UI_BID_RESUME, UI_BID_BACKTOTITLE
} UiPauseMenuId;

typedef struct UiButton {
    const char *text;
    int buttonId;
    int fontSize;
    // bool mouseHovered;
    bool clicked;
    Vector2 position;
    Color color;
} UiButton;

typedef struct UiAnalogStick {
    Vector2 centerPos;
    Vector2 stickPos;
    float centerRadius;
    float stickRadius;
    int lastTouchId;
    bool isActive;
} UiAnalogStick;

typedef struct UiMenu {
    UiButton *buttons; // allocate buttons for variable length menus
    unsigned int buttonCount;
} UiMenu;

// Holds data for the title screen menu
typedef struct UiState {
    UiButton title[2]; // Title text
    UiButton *buttons; // non-menu buttons
    UiButton pause;
    UiButton shoot; // virtual input button
    UiButton fly;   // virtual input button
    UiAnalogStick stick;  // virtual analog stick
    UiMenu menus[3]; // title, difficulty, and pause menus
    float keyHeldTime;
    float textFade;            // tracks fade value over time
    float textFadeTimeElapsed; // tracks time for the fade animation
    UiMenuState currentMenu;
    unsigned int selectedId;
    bool firstFrame;
    bool lastSelectWithMouse;
    bool autoScroll;
    bool mouseInUse; // whether or not the UI has precedence over mouse input
} UiState;

extern UiState ui; // global declaration

// Prototypes
// ----------------------------------------------------------------------------

// Initialize
void InitUiState(void); // Initializes the title screen and allocates memory for menu buttons
UiButton InitUiTitle(char *text);
UiButton InitUiButton(char *text, int buttonId, float textPosX, float textPosY, int fontSize);
UiButton *CreateUiMenuButton(char *text, UiMenu *menu, float textPosX, float textPosY, int fontSize); // Initializes a button within a menu
UiButton *CreateUiMenuButtonRelative(char* text, UiMenu *menu, float offsetY, int fontSize); // Initializes a button within a menu relative to the last menu button
void FreeUiState(void); // Frees memory for all menu buttons

// Update / User Input
void UpdateUiFrame(void); // Updates the menu for the current frame
void UpdateUiMenuTraverse(void); // Updates the cursor for movement by user input
// void UpdateUiButtonMouseHover(UiButton *button); // Draw cursor when mouse is over button
//                                                  // Disabled+Unused
void UpdateUiButtonSelect(UiButton *button); // Selects a button by user input
void UpdateUiTouchInput(UiButton *button); // Updates virtual input from button
void UpdateUiAnalogStick(UiAnalogStick *stick);
void ChangeUiMenu(UiMenuState newMenu); // Change from one menu to another

bool IsMouseWithinUiButton(UiButton *button);
int IsTouchWithinUiButton(UiButton *button);

// Draw
void DrawUiFrame(void); // Draws the menu for the current frame
void DrawUiElement(UiButton *button);
void DrawUiCursor(UiButton *selectedButton); // Draw the cursor at the given button
void DrawUiOutline(UiButton *selectedButton); // Draw a highlight box around a given button
void DrawUiAnalogStick (UiAnalogStick *stick); // Draw virtual analog stick
void DrawLives(void);
void DrawCenterText(void); // Draws center text based on game state
                           // TODO replace with something like SetCenterText(char *text, float time)?

#endif // ASTEROIDS_MENU_HEADER_GUARD
