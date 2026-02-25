// TOUCH CONTROLS FOR ANDROID
// On-screen virtual gamepad implementation
// (c)2025 Otto Matic Android Port
//
// Virtual joystick + 4-button diamond layout

#include "game.h"
#include "touchcontrols.h"

#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "OttoMatic", __VA_ARGS__)
#else
#define LOGI(...) SDL_Log(__VA_ARGS__)
#endif

/****************************/
/*    CONSTANTS             */
/****************************/

// Invalid finger ID sentinel value
#define INVALID_FINGER_ID       ((SDL_FingerID)-1)

// Joystick size and position (relative to screen height)
#define JOYSTICK_RADIUS         0.15f       // Joystick radius as fraction of screen height (20% larger than old 0.125)
#define JOYSTICK_CENTER_X       0.15f
#define JOYSTICK_CENTER_Y       0.70f

// Joystick dead zone
#define JOYSTICK_DEADZONE       0.15f

// Action button size
#define BUTTON_SIZE             0.065f      // Button radius as fraction of screen height

// Diamond layout center on right side of screen
#define DIAMOND_CENTER_X        0.85f
#define DIAMOND_CENTER_Y        0.68f
#define DIAMOND_SPACING         0.10f       // Distance from center to each button

// Pause and debug buttons
#define PAUSE_BUTTON_X          0.50f
#define PAUSE_BUTTON_Y          0.05f

#define DEBUG_BUTTON_X          0.95f
#define DEBUG_BUTTON_Y          0.05f

// Transparency
#define BG_ALPHA                0.15f       // Very transparent background
#define OUTLINE_ALPHA           0.4f        // Subtle outline
#define ICON_ALPHA              0.45f       // Subtle icon
#define PRESSED_ALPHA           0.6f        // More visible when pressed

// Hit area multipliers (touch targets larger than visual size)
#define BUTTON_HIT_MULTIPLIER   1.3f        // 30% larger than visual
#define JOYSTICK_HIT_MULTIPLIER 1.5f        // 50% larger than visual

// Maximum circle segments for drawing
#define MAX_CIRCLE_SEGMENTS     32

/****************************/
/*    TYPES                 */
/****************************/

typedef struct
{
    float   centerX;        // Center X position (0-1)
    float   centerY;        // Center Y position (0-1)
    float   radius;         // Radius (as fraction of screen height)
    int     buttonID;       // Which button this represents
    bool    isRound;        // Round button (true) or rectangular (false)
} TouchButton;

/****************************/
/*    VARIABLES             */
/****************************/

TouchControlState gTouchControls;

static TouchButton gTouchButtons[NUM_TOUCH_BUTTONS];
static SDL_FingerID gJoystickFinger = INVALID_FINGER_ID;
static SDL_FingerID gButtonFingers[NUM_TOUCH_BUTTONS];

static bool gTouchControlsInitialized = false;

// Screen dimensions for touch coordinate conversion
static int gTouchScreenWidth = 640;
static int gTouchScreenHeight = 480;

// Vertex buffer for circle drawing
static GLfloat gCircleVertices[(MAX_CIRCLE_SEGMENTS + 2) * 2];

/****************************/
/*    PROTOTYPES            */
/****************************/

static void InitTouchButton(int buttonID, float x, float y, float radius, bool isRound);
static int HitTestButtons(float touchX, float touchY);
static void ProcessJoystickTouch(float touchX, float touchY);
static bool IsInJoystickArea(float touchX, float touchY);
static void DrawFilledCircle(float cx, float cy, float r, int segments);
static void DrawCircleOutline(float cx, float cy, float r, int segments);
static void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
static void DrawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
static void DrawLine(float x1, float y1, float x2, float y2);
static void DrawJoystick(float centerX, float centerY, float radius, float alpha);
static void DrawActionButton(const TouchButton* button, bool pressed);

/****************************/
/*    IMPLEMENTATION        */
/****************************/

bool IsTouchDevice(void)
{
#ifdef __ANDROID__
    return true;
#else
    // Check if there's a touch device available
    int numTouchDevices = 0;
    SDL_TouchID* touchDevices = SDL_GetTouchDevices(&numTouchDevices);
    bool hasTouchDevice = numTouchDevices > 0;
    SDL_free(touchDevices);
    return hasTouchDevice;
#endif
}

void TouchControls_Init(void)
{
    if (gTouchControlsInitialized)
        return;

    LOGI("TouchControls_Init: Initializing touch controls");

    // Initialize state
    SDL_memset(&gTouchControls, 0, sizeof(gTouchControls));
    gTouchControls.visible = IsTouchDevice();
    gTouchControls.opacity = 1.0f;
    gTouchControls.buttonScale = 1.0f;
    
    // Reset finger tracking
    gJoystickFinger = INVALID_FINGER_ID;
    for (int i = 0; i < NUM_TOUCH_BUTTONS; i++)
    {
        gButtonFingers[i] = INVALID_FINGER_ID;
    }

    // Virtual joystick d-pad entries (not drawn as individual buttons)
    InitTouchButton(TOUCH_BUTTON_DPAD_UP,    JOYSTICK_CENTER_X, JOYSTICK_CENTER_Y - JOYSTICK_RADIUS/2, JOYSTICK_RADIUS/3, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_DOWN,  JOYSTICK_CENTER_X, JOYSTICK_CENTER_Y + JOYSTICK_RADIUS/2, JOYSTICK_RADIUS/3, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_LEFT,  JOYSTICK_CENTER_X - JOYSTICK_RADIUS/2, JOYSTICK_CENTER_Y, JOYSTICK_RADIUS/3, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_RIGHT, JOYSTICK_CENTER_X + JOYSTICK_RADIUS/2, JOYSTICK_CENTER_Y, JOYSTICK_RADIUS/3, false);
    
    // Diamond layout action buttons (right side of screen)
    // Bottom = Jump, Right = Shoot, Top = Switch Weapon, Left = Interact
    InitTouchButton(TOUCH_BUTTON_JUMP,           DIAMOND_CENTER_X, DIAMOND_CENTER_Y + DIAMOND_SPACING, BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_SHOOT,          DIAMOND_CENTER_X + DIAMOND_SPACING, DIAMOND_CENTER_Y, BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_PUNCH_PICKUP,   DIAMOND_CENTER_X - DIAMOND_SPACING, DIAMOND_CENTER_Y, BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_SWITCH_WEAPON,  DIAMOND_CENTER_X, DIAMOND_CENTER_Y - DIAMOND_SPACING, BUTTON_SIZE, true);
    
    InitTouchButton(TOUCH_BUTTON_PAUSE,          PAUSE_BUTTON_X, PAUSE_BUTTON_Y, BUTTON_SIZE * 0.6f, true);
    InitTouchButton(TOUCH_BUTTON_DEBUG_TOGGLE,   DEBUG_BUTTON_X, DEBUG_BUTTON_Y, BUTTON_SIZE * 0.5f, true);

    gTouchControlsInitialized = true;
    
    LOGI("TouchControls_Init: Touch controls initialized, visible=%d", gTouchControls.visible);
}

void TouchControls_Shutdown(void)
{
    gTouchControlsInitialized = false;
    SDL_memset(&gTouchControls, 0, sizeof(gTouchControls));
}

static void InitTouchButton(int buttonID, float x, float y, float radius, bool isRound)
{
    gTouchButtons[buttonID].centerX = x;
    gTouchButtons[buttonID].centerY = y;
    gTouchButtons[buttonID].radius = radius;
    gTouchButtons[buttonID].buttonID = buttonID;
    gTouchButtons[buttonID].isRound = isRound;
}

static int HitTestButtons(float touchX, float touchY)
{
    // touchX and touchY are in normalized coordinates (0-1)
    float aspectRatio = (float)gTouchScreenWidth / (float)gTouchScreenHeight;
    
    // Check action buttons (not joystick d-pad entries)
    for (int i = TOUCH_BUTTON_JUMP; i < NUM_TOUCH_BUTTONS; i++)
    {
        TouchButton* btn = &gTouchButtons[i];
        
        float dx = (touchX - btn->centerX) * aspectRatio;
        float dy = touchY - btn->centerY;
        float dist = sqrtf(dx*dx + dy*dy);
        float scaledRadius = btn->radius * gTouchControls.buttonScale;
        
        if (dist <= scaledRadius * BUTTON_HIT_MULTIPLIER)
        {
            return i;
        }
    }
    
    return TOUCH_BUTTON_NONE;
}

static bool IsInJoystickArea(float touchX, float touchY)
{
    float aspectRatio = (float)gTouchScreenWidth / (float)gTouchScreenHeight;
    float dx = (touchX - JOYSTICK_CENTER_X) * aspectRatio;
    float dy = touchY - JOYSTICK_CENTER_Y;
    float dist = sqrtf(dx*dx + dy*dy);
    float scaledRadius = JOYSTICK_RADIUS * gTouchControls.buttonScale;
    
    return dist <= scaledRadius * JOYSTICK_HIT_MULTIPLIER;
}

static void ProcessJoystickTouch(float touchX, float touchY)
{
    // Convert touch position to joystick direction
    float aspectRatio = (float)gTouchScreenWidth / (float)gTouchScreenHeight;
    float dx = (touchX - JOYSTICK_CENTER_X) * aspectRatio;
    float dy = touchY - JOYSTICK_CENTER_Y;
    
    float dist = sqrtf(dx*dx + dy*dy);
    float scaledRadius = JOYSTICK_RADIUS * gTouchControls.buttonScale;
    
    if (dist < JOYSTICK_DEADZONE * scaledRadius)
    {
        // In dead zone - no movement
        gTouchControls.analogX = 0;
        gTouchControls.analogY = 0;
        gTouchControls.dpadActive = false;
        return;
    }
    
    // Normalize and clamp
    float normalizedDist = MinFloat(dist / scaledRadius, 1.0f);
    float angle = atan2f(dy, dx);
    
    gTouchControls.analogX = cosf(angle) * normalizedDist;
    gTouchControls.analogY = sinf(angle) * normalizedDist;
    gTouchControls.dpadActive = true;
    
    // Set discrete d-pad buttons based on direction
    float threshold = 0.3f;
    
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_UP]    = (gTouchControls.analogY < -threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_DOWN]  = (gTouchControls.analogY > threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_LEFT]  = (gTouchControls.analogX < -threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_RIGHT] = (gTouchControls.analogX > threshold);
}

void TouchControls_BeginFrame(void)
{
    if (!gTouchControlsInitialized || !gTouchControls.visible)
        return;

    // Update screen dimensions before events are processed so HandleEvent
    // uses the current aspect ratio for hit testing.  Query the window
    // directly so that the values are always fresh, even before the first
    // OGL_DrawScene call sets gGameWindowWidth/gGameWindowHeight.
    if (gSDLWindow)
    {
        SDL_GetWindowSizeInPixels(gSDLWindow, &gTouchScreenWidth, &gTouchScreenHeight);
    }
    else if (gGameWindowWidth > 0 && gGameWindowHeight > 0)
    {
        gTouchScreenWidth = gGameWindowWidth;
        gTouchScreenHeight = gGameWindowHeight;
    }

    // Store previous state for edge detection (must happen before events are processed)
    for (int i = 0; i < NUM_TOUCH_BUTTONS; i++)
    {
        gTouchControls.wasPressed[i] = gTouchControls.isPressed[i];
    }
}

void TouchControls_Update(void)
{
    if (!gTouchControlsInitialized || !gTouchControls.visible)
        return;

    // Sync screen dimensions for the upcoming Draw call
    gTouchScreenWidth = gGameWindowWidth;
    gTouchScreenHeight = gGameWindowHeight;

    // Clear action button states (joystick is handled separately)
    for (int i = TOUCH_BUTTON_JUMP; i < NUM_TOUCH_BUTTONS; i++)
    {
        gTouchControls.isPressed[i] = false;
    }
    
    // Clear joystick state if no finger is tracking it
    if (gJoystickFinger == INVALID_FINGER_ID)
    {
        gTouchControls.analogX = 0;
        gTouchControls.analogY = 0;
        gTouchControls.dpadActive = false;
        gTouchControls.isPressed[TOUCH_BUTTON_DPAD_UP] = false;
        gTouchControls.isPressed[TOUCH_BUTTON_DPAD_DOWN] = false;
        gTouchControls.isPressed[TOUCH_BUTTON_DPAD_LEFT] = false;
        gTouchControls.isPressed[TOUCH_BUTTON_DPAD_RIGHT] = false;
    }
    
    // Process touch state from SDL
    int numTouchDevices = 0;
    SDL_TouchID* touchDevices = SDL_GetTouchDevices(&numTouchDevices);
    
    if (touchDevices && numTouchDevices > 0)
    {
        for (int d = 0; d < numTouchDevices; d++)
        {
            int fingerCount = 0;
            SDL_Finger** fingers = SDL_GetTouchFingers(touchDevices[d], &fingerCount);
            
            if (fingers)
            {
                for (int f = 0; f < fingerCount; f++)
                {
                    SDL_Finger* finger = fingers[f];
                    if (!finger) continue;
                    
                    float touchX = finger->x;
                    float touchY = finger->y;
                    
                    // Check if this finger is in the joystick area
                    if (IsInJoystickArea(touchX, touchY))
                    {
                        if (gJoystickFinger == INVALID_FINGER_ID || gJoystickFinger == finger->id)
                        {
                            gJoystickFinger = finger->id;
                            ProcessJoystickTouch(touchX, touchY);
                        }
                    }
                    else
                    {
                        // Only check action buttons for non-joystick fingers;
                        // the joystick finger must never activate buttons even if
                        // it drifts outside the joystick hit area.
                        if (finger->id != gJoystickFinger)
                        {
                            int buttonHit = HitTestButtons(touchX, touchY);
                            if (buttonHit != TOUCH_BUTTON_NONE)
                            {
                                gTouchControls.isPressed[buttonHit] = true;
                                gButtonFingers[buttonHit] = finger->id;
                            }
                        }
                    }
                }
                SDL_free(fingers);
            }
        }
        SDL_free(touchDevices);
    }

    // Handle debug toggle button (cycle through debug modes: 0=off, 1=fps, 2=all)
    if (gTouchControls.isPressed[TOUCH_BUTTON_DEBUG_TOGGLE] &&
        !gTouchControls.wasPressed[TOUCH_BUTTON_DEBUG_TOGGLE])
    {
        if (++gDebugMode > 2)
            gDebugMode = 0;
    }
}

//=============================================================================
// Drawing functions
//=============================================================================

static void DrawFilledCircle(float cx, float cy, float r, int segments)
{
    if (segments > MAX_CIRCLE_SEGMENTS)
        segments = MAX_CIRCLE_SEGMENTS;
    
    // Center vertex
    gCircleVertices[0] = cx;
    gCircleVertices[1] = cy;
    
    // Circle vertices
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * PI * (float)i / (float)segments;
        gCircleVertices[(i + 1) * 2] = cx + r * cosf(angle);
        gCircleVertices[(i + 1) * 2 + 1] = cy + r * sinf(angle);
    }
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, gCircleVertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void DrawCircleOutline(float cx, float cy, float r, int segments)
{
    if (segments > MAX_CIRCLE_SEGMENTS)
        segments = MAX_CIRCLE_SEGMENTS;
    
    for (int i = 0; i < segments; i++)
    {
        float angle = 2.0f * PI * (float)i / (float)segments;
        gCircleVertices[i * 2] = cx + r * cosf(angle);
        gCircleVertices[i * 2 + 1] = cy + r * sinf(angle);
    }
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, gCircleVertices);
    glDrawArrays(GL_LINE_LOOP, 0, segments);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3)
{
    GLfloat vertices[] = {
        x1, y1,
        x2, y2,
        x3, y3
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void DrawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
    GLfloat vertices[] = {
        x1, y1,
        x2, y2,
        x3, y3,
        x4, y4
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableClientState(GL_VERTEX_ARRAY);
}

static void DrawLine(float x1, float y1, float x2, float y2)
{
    GLfloat vertices[] = {
        x1, y1,
        x2, y2
    };
    
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, vertices);
    glDrawArrays(GL_LINES, 0, 2);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void TouchControls_Draw(void)
{
    if (!gTouchControlsInitialized || !gTouchControls.visible)
        return;
    
    // Set up 2D rendering
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, gTouchScreenWidth, gTouchScreenHeight, 0, -1, 1);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Save and set GL state for clean rendering
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glLineWidth(2.0f);
    
    // The 3D renderer may have left colour/normal/texcoord client arrays enabled.
    // Disable them so every primitive below uses the solid colour set via glColor4f
    // (or the GLES bridge equivalent) instead of stale per-vertex data from the
    // last 3D draw call, which would produce unwanted gradients on the HUD circles.
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    
    // Draw virtual joystick
    float joyX = JOYSTICK_CENTER_X * gTouchScreenWidth;
    float joyY = JOYSTICK_CENTER_Y * gTouchScreenHeight;
    float joyR = JOYSTICK_RADIUS * gTouchScreenHeight * gTouchControls.buttonScale;
    DrawJoystick(joyX, joyY, joyR, 1.0f);
    
    // Draw 4 action buttons in diamond layout
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_JUMP],
                     gTouchControls.isPressed[TOUCH_BUTTON_JUMP]);
    
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_SHOOT],
                     gTouchControls.isPressed[TOUCH_BUTTON_SHOOT]);
    
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_PUNCH_PICKUP],
                     gTouchControls.isPressed[TOUCH_BUTTON_PUNCH_PICKUP]);
    
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_SWITCH_WEAPON],
                     gTouchControls.isPressed[TOUCH_BUTTON_SWITCH_WEAPON]);
    
    // Pause button
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_PAUSE],
                     gTouchControls.isPressed[TOUCH_BUTTON_PAUSE]);
    
    // Debug toggle button (extra subtle)
    DrawActionButton(&gTouchButtons[TOUCH_BUTTON_DEBUG_TOGGLE],
                     gTouchControls.isPressed[TOUCH_BUTTON_DEBUG_TOGGLE]);
    
    // Restore GL state fully so the next frame's 3D rendering is not affected
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    OGL_EnableLighting();
    glLineWidth(1.0f);
    SetColor4f(1, 1, 1, 1);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void DrawJoystick(float centerX, float centerY, float radius, float alpha)
{
    // Outer ring background - very transparent
    glColor4f(0.3f, 0.3f, 0.3f, BG_ALPHA * alpha);
    DrawFilledCircle(centerX, centerY, radius, 32);
    
    // Outer ring outline
    glColor4f(0.7f, 0.7f, 0.7f, OUTLINE_ALPHA * alpha);
    DrawCircleOutline(centerX, centerY, radius, 32);
    
    // Thumb indicator
    float thumbX = centerX + gTouchControls.analogX * radius * 0.6f;
    float thumbY = centerY + gTouchControls.analogY * radius * 0.6f;
    float thumbR = radius * 0.25f;
    
    if (gTouchControls.dpadActive)
    {
        // Active: slightly brighter thumb
        glColor4f(0.5f, 0.5f, 0.5f, 0.35f * alpha);
        DrawFilledCircle(thumbX, thumbY, thumbR, 16);
        glColor4f(0.8f, 0.8f, 0.8f, OUTLINE_ALPHA * alpha);
        DrawCircleOutline(thumbX, thumbY, thumbR, 16);
    }
    else
    {
        // Idle: centered thumb
        glColor4f(0.4f, 0.4f, 0.4f, 0.25f * alpha);
        DrawFilledCircle(centerX, centerY, thumbR, 16);
        glColor4f(0.7f, 0.7f, 0.7f, OUTLINE_ALPHA * 0.7f * alpha);
        DrawCircleOutline(centerX, centerY, thumbR, 16);
    }
}

static void DrawActionButton(const TouchButton* button, bool pressed)
{
    float scale = gTouchControls.buttonScale;
    float cx = button->centerX * gTouchScreenWidth;
    float cy = button->centerY * gTouchScreenHeight;
    float r = button->radius * gTouchScreenHeight * scale;
    
    float bgAlpha = pressed ? 0.25f : BG_ALPHA;
    float outAlpha = pressed ? PRESSED_ALPHA : OUTLINE_ALPHA;
    float iconAlpha = pressed ? PRESSED_ALPHA : ICON_ALPHA;
    
    // All buttons: transparent gray background, white outline, white icon
    glColor4f(0.3f, 0.3f, 0.3f, bgAlpha);
    DrawFilledCircle(cx, cy, r, 24);
    
    glLineWidth(2.0f);
    glColor4f(0.8f, 0.8f, 0.8f, outAlpha);
    DrawCircleOutline(cx, cy, r, 24);
    
    // Draw icon based on button type
    float iconSize = r * 0.5f;
    glColor4f(0.9f, 0.9f, 0.9f, iconAlpha);
    
    switch (button->buttonID)
    {
        case TOUCH_BUTTON_JUMP:
            // Up arrow (jump)
            DrawTriangle(cx, cy - iconSize,
                         cx - iconSize * 0.7f, cy + iconSize * 0.5f,
                         cx + iconSize * 0.7f, cy + iconSize * 0.5f);
            break;
            
        case TOUCH_BUTTON_SHOOT:
            // Crosshair (fire weapon)
            glLineWidth(2.0f);
            DrawLine(cx - iconSize, cy, cx + iconSize, cy);
            DrawLine(cx, cy - iconSize, cx, cy + iconSize);
            DrawCircleOutline(cx, cy, iconSize * 0.5f, 12);
            break;
            
        case TOUCH_BUTTON_PUNCH_PICKUP:
            // Square (interact)
            {
                float hs = iconSize * 0.55f;
                DrawQuad(cx - hs, cy - hs,
                         cx + hs, cy - hs,
                         cx + hs, cy + hs,
                         cx - hs, cy + hs);
            }
            break;
            
        case TOUCH_BUTTON_SWITCH_WEAPON:
            // Double arrows (switch weapon - cycling arrows)
            DrawTriangle(cx - iconSize * 0.5f, cy - iconSize * 0.1f,
                         cx + iconSize * 0.5f, cy - iconSize * 0.1f,
                         cx, cy - iconSize * 0.8f);
            DrawTriangle(cx - iconSize * 0.5f, cy + iconSize * 0.1f,
                         cx + iconSize * 0.5f, cy + iconSize * 0.1f,
                         cx, cy + iconSize * 0.8f);
            break;
            
        case TOUCH_BUTTON_PAUSE:
            // Pause bars
            {
                float bw = iconSize * 0.2f;
                float bh = iconSize * 0.6f;
                float gap = iconSize * 0.15f;
                DrawQuad(cx - gap - bw, cy - bh,
                         cx - gap, cy - bh,
                         cx - gap, cy + bh,
                         cx - gap - bw, cy + bh);
                DrawQuad(cx + gap, cy - bh,
                         cx + gap + bw, cy - bh,
                         cx + gap + bw, cy + bh,
                         cx + gap, cy + bh);
            }
            break;

        case TOUCH_BUTTON_DEBUG_TOGGLE:
            // Small "i" info icon
            DrawFilledCircle(cx, cy - iconSize * 0.55f, iconSize * 0.15f, 8);
            DrawQuad(cx - iconSize * 0.15f, cy - iconSize * 0.25f,
                     cx + iconSize * 0.15f, cy - iconSize * 0.25f,
                     cx + iconSize * 0.15f, cy + iconSize * 0.6f,
                     cx - iconSize * 0.15f, cy + iconSize * 0.6f);
            break;
    }
}

bool TouchControls_IsPressed(int buttonID)
{
    if (!gTouchControlsInitialized || buttonID < 0 || buttonID >= NUM_TOUCH_BUTTONS)
        return false;
    return gTouchControls.isPressed[buttonID];
}

bool TouchControls_IsNewPress(int buttonID)
{
    if (!gTouchControlsInitialized || buttonID < 0 || buttonID >= NUM_TOUCH_BUTTONS)
        return false;
    return gTouchControls.isPressed[buttonID] && !gTouchControls.wasPressed[buttonID];
}

void TouchControls_GetAnalog(float* outX, float* outY)
{
    if (outX) *outX = gTouchControls.analogX;
    if (outY) *outY = gTouchControls.analogY;
}

void TouchControls_SetVisible(bool visible)
{
    gTouchControls.visible = visible;
}

bool TouchControls_IsVisible(void)
{
    return gTouchControls.visible && gTouchControlsInitialized;
}

void TouchControls_SetOpacity(float opacity)
{
    gTouchControls.opacity = ClampFloat(opacity, 0.0f, 1.0f);
}

void TouchControls_SetScale(float scale)
{
    gTouchControls.buttonScale = ClampFloat(scale, 0.5f, 2.0f);
}

// Handle touch events from SDL event loop
void TouchControls_HandleEvent(SDL_Event* event)
{
    if (!gTouchControlsInitialized || !gTouchControls.visible)
        return;
    
    switch (event->type)
    {
        case SDL_EVENT_FINGER_DOWN:
        {
            float touchX = event->tfinger.x;
            float touchY = event->tfinger.y;
            SDL_FingerID fingerID = event->tfinger.fingerID;
            
            if (IsInJoystickArea(touchX, touchY))
            {
                if (gJoystickFinger == INVALID_FINGER_ID)
                {
                    gJoystickFinger = fingerID;
                    ProcessJoystickTouch(touchX, touchY);
                }
            }
            else
            {
                int buttonHit = HitTestButtons(touchX, touchY);
                if (buttonHit != TOUCH_BUTTON_NONE)
                {
                    gTouchControls.isPressed[buttonHit] = true;
                    gButtonFingers[buttonHit] = fingerID;
                }
            }
            break;
        }
        
        case SDL_EVENT_FINGER_MOTION:
        {
            float touchX = event->tfinger.x;
            float touchY = event->tfinger.y;
            SDL_FingerID fingerID = event->tfinger.fingerID;
            
            if (fingerID == gJoystickFinger)
            {
                ProcessJoystickTouch(touchX, touchY);
            }
            break;
        }
        
        case SDL_EVENT_FINGER_UP:
        {
            SDL_FingerID fingerID = event->tfinger.fingerID;
            
            if (fingerID == gJoystickFinger)
            {
                gJoystickFinger = INVALID_FINGER_ID;
                gTouchControls.analogX = 0;
                gTouchControls.analogY = 0;
                gTouchControls.dpadActive = false;
                gTouchControls.isPressed[TOUCH_BUTTON_DPAD_UP] = false;
                gTouchControls.isPressed[TOUCH_BUTTON_DPAD_DOWN] = false;
                gTouchControls.isPressed[TOUCH_BUTTON_DPAD_LEFT] = false;
                gTouchControls.isPressed[TOUCH_BUTTON_DPAD_RIGHT] = false;
            }
            else
            {
                // Check if this finger was on a button
                for (int i = 0; i < NUM_TOUCH_BUTTONS; i++)
                {
                    if (gButtonFingers[i] == fingerID)
                    {
                        gTouchControls.isPressed[i] = false;
                        gButtonFingers[i] = INVALID_FINGER_ID;
                        break;
                    }
                }
            }
            break;
        }
    }
}
