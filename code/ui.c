// EXPLANATION:
// For managing the user interface
// See ui.h for more documentation/descriptions

#include "ui.h"

#include "raylib.h"
#include "raymath.h" // needed for Vector math

#include "config.h"
#include "input.h"
#include "asteroids.h"

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr)[0]))

void InitUiState(void)
{
    UiState uiDefaults = {
        .currentMenu = UI_MENU_TITLE,
        .selectedId = UI_BID_START,
        .firstFrame = true,
    };

    // Title menu buttons
    UiMenu *titleMenu = &uiDefaults.menus[UI_MENU_TITLE];

    uiDefaults.title[0] = InitUiTitle("Asteroids");
    uiDefaults.title[1] = InitUiTitle("Remake");
#if !defined(PLATFORM_WEB)
    UiButton *start =
#endif
        InitUiMenuButtonRelative("Start", UI_TITLE_BUTTON_SIZE, &uiDefaults.title[1], UI_TITLE_SPACING, titleMenu);
#if !defined(PLATFORM_WEB)
    InitUiMenuButtonRelative("Exit", UI_TITLE_BUTTON_SIZE, start, UI_BUTTON_SPACING, titleMenu);
#endif

    // Pause button + menu
    UiMenu *pauseMenu = &uiDefaults.menus[UI_MENU_PAUSE];
    char *pauseText = "Pause";
    char *resumeText = "Resume";
    char *toTitleText = "Back to Title";
    const int pauseTextLength = MeasureText(pauseText, UI_FONT_SIZE_EDGE);
    uiDefaults.pause =
        InitUiButton(pauseText, UI_FONT_SIZE_EDGE,
                     (float)VIRTUAL_WIDTH/4 - pauseTextLength/2,
                     (float)VIRTUAL_HEIGHT - UI_FONT_SIZE_EDGE - UI_EDGE_PADDING);

    InitUiMenuButtonRelative(resumeText, UI_FONT_SIZE_EDGE, &uiDefaults.pause, -UI_FONT_SIZE_EDGE, pauseMenu);
    InitUiMenuButtonRelative(toTitleText, UI_FONT_SIZE_EDGE, &uiDefaults.pause, -UI_FONT_SIZE_EDGE*2 - UI_BUTTON_SPACING, pauseMenu);

    ui = uiDefaults;
}

UiButton InitUiTitle(char *text)
{
    static UiButton *previousTitleLine = 0;

    int fontSize = UI_TITLE_SIZE;
    int textWidth = MeasureText(text, fontSize);
    float titlePosX = (VIRTUAL_WIDTH - (float)textWidth)/2;
#if !defined(PLATFORM_WEB) // different spacing for web
        float titlePosY = UI_TITLE_TOP_PADDING;
#else
        float titlePosY = UI_TITLE_TOP_PADDING + UI_TITLE_BUTTON_SIZE;
#endif
    if (previousTitleLine != 0)
        titlePosY += UI_TITLE_SIZE + 10;

    UiButton title = InitUiButton(text, fontSize, titlePosX, titlePosY);
    previousTitleLine = &title;

    return title;
}

UiButton InitUiButton(char *text, int fontSize, float textPosX, float textPosY)
{

    UiButton button = { text, fontSize, false, { textPosX, textPosY }, RAYWHITE };

    return button;
}

UiButton *InitUiMenuButton(char *text, int fontSize, float textPosX, float textPosY, UiMenu *menu)
{
    UiButton button = { text, fontSize, false, { textPosX, textPosY }, RAYWHITE };
    menu->buttonCount++;
    menu->buttons = MemRealloc(menu->buttons, menu->buttonCount*sizeof(UiButton));
    menu->buttons[menu->buttonCount - 1] = button;

    return &menu->buttons[menu->buttonCount - 1];
}

UiButton *InitUiMenuButtonRelative(char* text, int fontSize, UiButton *originButton, float offsetY, UiMenu *menu)
{
    float originWidth = (float)MeasureText(originButton->text, originButton->fontSize);
    float originPosX = (originButton->position.x + originWidth/2);
    float textPosX = originPosX - MeasureText(text, fontSize)/2;
    float textPosY = originButton->position.y + originButton->fontSize;

    return InitUiMenuButton(text, fontSize, textPosX, textPosY + offsetY, menu);
}

void FreeUiState(void)
{
    for (unsigned int i = 0; i < ARRAY_SIZE(ui.menus); i++)
        MemFree(ui.menus[i].buttons);
}

void UpdateUiFrame(void)
{
    if (ui.currentMenu != UI_MENU_GAMEPLAY)
    {
        if (IsInputActionPressed(INPUT_ACTION_BACK) && ui.currentMenu != UI_MENU_TITLE)
        {
            ChangeUiMenu(UI_MENU_TITLE);
            PlaySound(game.beeps[BEEP_MENU]);
        }

        UiButton *selectedButton = &ui.menus[ui.currentMenu].buttons[ui.selectedId];
        UpdateUiButtonSelect(selectedButton);
        UpdateUiMenuTraverse();
    }
    else if (!game.isPaused)
    {
        UpdateUiButtonMouseHover(&ui.pause);
        UpdateUiButtonSelect(&ui.pause);
    }

    // Update pause fade animation
    static float fadeLength = 1.5f; // Fade in and out at this rate in seconds
    static bool fadingOut = false;
    float fadeIncrement = (1.0f/fadeLength)*GetFrameTime();

    if (ui.textFade >= 1.0f)
        fadingOut = true;
    else if (ui.textFade <= 0.0f)
        fadingOut = false;
    if (fadingOut)
        fadeIncrement *= -1;

    ui.textFade += fadeIncrement;
}

void UpdateUiMenuTraverse(void)
{
    if (ui.currentMenu == UI_MENU_GAMEPLAY)
        return;
    UiMenu *menu = &ui.menus[ui.currentMenu];

    UiTitleMenuId prevId = ui.selectedId; // used to play beep

    // Move cursor via mouse
    bool mouseMoved = (Vector2Length(GetMouseDelta()) > 0);
    if (mouseMoved || (ui.firstFrame && ui.lastSelectWithMouse))
    {
        Vector2 mousePos = GetScaledMousePosition();

        for (unsigned int i = 0; i < menu->buttonCount; i++)
        {
            UiButton *currentButton = 0;
            currentButton = &menu->buttons[i];

            if (IsMouseWithinUiButton(mousePos, currentButton))
            {
                ui.selectedId = i;
                ui.autoScroll = false;
                ui.lastSelectWithMouse = true;
            }
        }
    }

    // Move cursor via keyboard
    bool isInputUp = IsInputActionDown(INPUT_ACTION_MENU_UP);
    bool isInputDown = IsInputActionDown(INPUT_ACTION_MENU_DOWN);
    const float autoScrollInitPause = 0.6f;

    bool initialKeyPress = (!ui.autoScroll && ui.keyHeldTime == 0);
    bool heldLongEnoughToRepeat = (ui.autoScroll && ui.keyHeldTime >= 0.1f);
    if (initialKeyPress || heldLongEnoughToRepeat)
    {
        if (isInputUp)
        {
            if (ui.selectedId > 0)
                ui.selectedId--;
            else
                ui.selectedId = menu->buttonCount - 1;
            ui.keyHeldTime = 0;
            ui.lastSelectWithMouse = false;
        }
        if (isInputDown)
        {
            if ((unsigned int)ui.selectedId < menu->buttonCount - 1)
                ui.selectedId++;
            else
                ui.selectedId = 0;
            ui.keyHeldTime = 0.0f;
            ui.lastSelectWithMouse = false;
        }
    }

    // Update auto-scroll timer when holding keys
    if (isInputUp || isInputDown)
    {
        ui.keyHeldTime += GetFrameTime();
        if (ui.keyHeldTime >= autoScrollInitPause)
        {
            ui.autoScroll = true;
        }
    }
    else
    {
        ui.keyHeldTime = 0;
        ui.autoScroll = false;
    }

    if (ui.selectedId != prevId && !ui.firstFrame)
        PlaySound(game.beeps[BEEP_MENU]);

    ui.firstFrame = false;
}

void UpdateUiButtonMouseHover(UiButton *button)
{
    bool mouseMoved = (Vector2Length(GetMouseDelta()) > 0);
    if (!mouseMoved) return;

    Vector2 mousePos = GetScaledMousePosition();

    if (IsMouseWithinUiButton(mousePos, button))
    {
        if (!button->mouseHovered)
            PlaySound(game.beeps[BEEP_MENU]);
        button->mouseHovered = true;
    }
    else
    {
        button->mouseHovered = false;
    }
}

void UpdateUiButtonSelect(UiButton *button)
{
    Vector2 mousePos = GetScaledMousePosition();

    // Select pause button
    if (ui.currentMenu == UI_MENU_GAMEPLAY && IsGestureDetected(GESTURE_TAP) &&
         (!IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && IsMouseWithinUiButton(mousePos, button)))
    {
        ChangeUiMenu(UI_MENU_PAUSE);
        PlaySound(game.beeps[BEEP_MENU]);
    }

    // Select a menu button
    else if (IsInputActionPressed(INPUT_ACTION_CONFIRM) ||
        (IsGestureDetected(GESTURE_TAP) &&
         (!IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && IsMouseWithinUiButton(mousePos, button))))
    {
        if (ui.currentMenu == UI_MENU_GAMEPLAY && !game.isPaused)
            return; // not a menu

        if (ui.currentMenu == UI_MENU_PAUSE && !ui.firstFrame)
        {
            if (ui.selectedId == UI_BID_RESUME)
            {
                game.isPaused = false;
                ui.currentMenu = UI_MENU_GAMEPLAY;
            }
            else if (ui.selectedId == UI_BID_BACKTOTITLE)
            {
                ChangeUiMenu(UI_MENU_TITLE);
            }
        }

        else if (ui.currentMenu == UI_MENU_TITLE)
        {
            if (ui.selectedId == UI_BID_EXIT)
                game.gameShouldExit = true;
            else if (ui.selectedId == UI_BID_START)
                ChangeUiMenu(UI_MENU_GAMEPLAY);
        }

        PlaySound(game.beeps[BEEP_MENU]);
    }
}

bool IsMouseWithinUiButton(Vector2 mousePos, UiButton *button)
{
    int padding = 20; // extra clickable area around the text
    int buttonWidth = MeasureText(button->text, button->fontSize);
    if ((mousePos.x >= button->position.x - padding) &&
        (mousePos.x <= button->position.x + buttonWidth + padding) &&
        (mousePos.y >= button->position.y - padding) &&
        (mousePos.y <= button->position.y + button->fontSize + padding))
        return true;
    else
        return false;
}

void ChangeUiMenu(UiMenuState newMenu)
{
    if (newMenu == UI_MENU_TITLE)
    {
        // Clear old game state if returning from gameplay
        if (game.currentScreen == SCREEN_GAMEPLAY)
        {
            FreeGameState();
            InitGameState();
            game.currentScreen = SCREEN_TITLE;
        }

        ui.selectedId = UI_BID_START;
    }

    else if (newMenu == UI_MENU_PAUSE)
    {
        game.isPaused = true;
        ui.selectedId = UI_BID_RESUME;
    }

    else if (newMenu == UI_MENU_GAMEPLAY)
    {
        // game.currentMode = (GameMode)ui.selectedId;
        game.currentScreen = SCREEN_GAMEPLAY;
    }

    ui.currentMenu = newMenu;
    ui.firstFrame = true;
}

void DrawUiFrame(void)
{
    if (game.currentScreen == SCREEN_TITLE)
    {
        // Draw stars
        for (unsigned int i = 0; i < STAR_AMOUNT; i++)
            DrawCircleV(game.stars[i], 1.0f, WHITE);

        // Draw title menu
        for (unsigned int i = 0; i < ARRAY_SIZE(ui.title); i++)
            DrawUiElement(&ui.title[i]);
    }

    if (ui.currentMenu != UI_MENU_GAMEPLAY)
    {
        UiMenu *menu = &ui.menus[ui.currentMenu];
        for (unsigned int i = 0; i < menu->buttonCount; i++)
            DrawUiElement(&menu->buttons[i]);

        UiButton *selectedButton = &ui.menus[ui.currentMenu].buttons[ui.selectedId];
        DrawUiCursor(selectedButton);
    }
    else if (game.currentScreen == SCREEN_GAMEPLAY)
    {
        // Draw pause button
        DrawUiElement(&ui.pause);
        if (ui.pause.mouseHovered)
            DrawUiCursor(&ui.pause);
    }

    if (game.currentScreen == SCREEN_GAMEPLAY)
    {
        // DrawUiScores();

        DrawLives();

        // Draw level indicator
        const char *levelText = TextFormat("Level: %i", game.level);
        unsigned int textLength = MeasureText(levelText, UI_FONT_SIZE_EDGE);
        DrawText(levelText,
                 VIRTUAL_WIDTH - textLength - UI_EDGE_PADDING, UI_EDGE_PADDING,
                 UI_FONT_SIZE_EDGE, RAYWHITE);


        // Fade animation
        Color fadeColor = Fade(RAYWHITE, ui.textFade);

        // Draw pause message
        if (game.isPaused)
        {
            char *text = "PAUSED";
            int textOffset = MeasureText(text, UI_FONT_SIZE_CENTER)/2;
            DrawText(text, VIRTUAL_WIDTH/2 - textOffset,
                     VIRTUAL_HEIGHT/2 - UI_FONT_SIZE_CENTER/2,
                     UI_FONT_SIZE_CENTER, fadeColor);
        }
        else if (game.lives <= 0)
        {
            char *text = "GAME OVER";
            int textOffset = MeasureText(text, UI_FONT_SIZE_CENTER)/2;
            DrawText(text, VIRTUAL_WIDTH/2 - textOffset,
                     VIRTUAL_HEIGHT/2 - UI_FONT_SIZE_CENTER/2,
                     UI_FONT_SIZE_CENTER, RAYWHITE);
        }
        // else if (game.currentMode == MODE_DEMO) // Draw demo mode message
        // {
        //     text = "DEMO MODE";
        //     int textOffset = MeasureText(text, UI_FONT_SIZE_CENTER)/2;
        //     DrawText(text, VIRTUAL_WIDTH/2 - textOffset,
        //              VIRTUAL_HEIGHT/2 - UI_FONT_SIZE_CENTER/2,
        //              UI_FONT_SIZE_CENTER, fadeColor);
        // }

    }

    // Debug:
    // DrawText(TextFormat("cursor selected: %i", menu->selectedId), 0, 40, 40, WHITE);
    // DrawText(TextFormat("%2i rock total", game.rockCount), 0, 25, 20, RAYWHITE);
    // DrawText(TextFormat("%2i eliminated", game.eliminatedCount), 0, 50, 20, RAYWHITE);
    // DrawText(TextFormat("%2i remaining", game.rockCount - game.eliminatedCount), 0, 75, 20, RAYWHITE);
}

void DrawUiElement(UiButton *button)
{
    DrawText(button->text, (int)button->position.x, (int)button->position.y,
             button->fontSize, RAYWHITE);
}

void DrawUiCursor(UiButton *selectedButton)
{
    float size = UI_CURSOR_SIZE;

    Vector2 selectPointPos; // the corner/vertice pointing towards the right
    Vector2 cursorOffset = (Vector2){-50.0f, (float)selectedButton->fontSize/2};
    selectPointPos = Vector2Add(selectedButton->position, cursorOffset);

    DrawTriangle(Vector2Add(selectPointPos, (Vector2){ -size*2, size }),
                 selectPointPos,
                 Vector2Add(selectPointPos, (Vector2){ -size*2, -size }),
                 RAYWHITE);
}

void DrawLives(void)
{
    float scale = 0.75f;
    float spacing = game.ship.width/6;
    Vector2 lifeTriangle[3] = {
        Vector2Scale(game.shipTriangle[0], scale),
        Vector2Scale(game.shipTriangle[1], scale),
        Vector2Scale(game.shipTriangle[2], scale),
    };

    for (unsigned int i = 0; i < 3; i++)
    {
        lifeTriangle[i].x += UI_EDGE_PADDING - game.ship.width*scale/2 - spacing;
        lifeTriangle[i].y += UI_EDGE_PADDING + game.ship.length*scale/2;
    }

    for (unsigned int i = 0; i < game.lives; i++)
    {
        for (unsigned int j = 0; j < 3; j++)
        {
            lifeTriangle[j].x += spacing + game.ship.width*scale;
        }
        DrawTriangle(lifeTriangle[0], lifeTriangle[1], lifeTriangle[2], GRAY);
    }
}
