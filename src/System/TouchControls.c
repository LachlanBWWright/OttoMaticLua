// TOUCH CONTROLS FOR ANDROID
// On-screen virtual gamepad implementation
// (c)2025 Otto Matic Android Port
//
// This file uses vertex arrays for OpenGL ES 1.1 compatibility

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

// Button sizes and positions (relative to screen size)
#define DPAD_SIZE               0.25f       // D-pad diameter as fraction of screen height
#define DPAD_MARGIN             0.05f       // Margin from screen edge
#define BUTTON_SIZE             0.10f       // Action button diameter
#define BUTTON_MARGIN           0.03f       // Margin between buttons
#define BUTTON_ALPHA            0.6f        // Default opacity

// D-pad dead zone
#define DPAD_DEADZONE           0.15f

// Button layout positions (as fractions of screen dimensions)
// D-pad is on the left side
#define DPAD_CENTER_X           0.15f
#define DPAD_CENTER_Y           0.70f

// Action buttons on the right side
#define JUMP_BUTTON_X           0.88f
#define JUMP_BUTTON_Y           0.60f

#define SHOOT_BUTTON_X          0.78f
#define SHOOT_BUTTON_Y          0.70f

#define PUNCH_BUTTON_X          0.88f
#define PUNCH_BUTTON_Y          0.80f

#define PREV_WEAPON_X           0.70f
#define PREV_WEAPON_Y           0.15f

#define NEXT_WEAPON_X           0.85f
#define NEXT_WEAPON_Y           0.15f

#define PAUSE_BUTTON_X          0.50f
#define PAUSE_BUTTON_Y          0.05f

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
static SDL_FingerID gDpadFinger = -1;   // Track which finger is on the d-pad
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
static void ProcessDpadTouch(float touchX, float touchY);
static bool IsInDpadArea(float touchX, float touchY);
static void DrawFilledCircle(float cx, float cy, float r, int segments);
static void DrawCircleOutline(float cx, float cy, float r, int segments);
static void DrawTriangle(float x1, float y1, float x2, float y2, float x3, float y3);
static void DrawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4);
static void DrawLine(float x1, float y1, float x2, float y2);
static void DrawDpad(float centerX, float centerY, float size, float alpha);
static void DrawButton(const TouchButton* button, bool pressed, float alpha);

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
    gTouchControls.opacity = BUTTON_ALPHA;
    gTouchControls.buttonScale = 1.0f;
    
    // Reset finger tracking
    gDpadFinger = -1;
    for (int i = 0; i < NUM_TOUCH_BUTTONS; i++)
    {
        gButtonFingers[i] = -1;
    }

    // Initialize button positions
    // D-pad buttons (virtual - used for hit testing)
    InitTouchButton(TOUCH_BUTTON_DPAD_UP,    DPAD_CENTER_X, DPAD_CENTER_Y - DPAD_SIZE/3, DPAD_SIZE/4, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_DOWN,  DPAD_CENTER_X, DPAD_CENTER_Y + DPAD_SIZE/3, DPAD_SIZE/4, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_LEFT,  DPAD_CENTER_X - DPAD_SIZE/3, DPAD_CENTER_Y, DPAD_SIZE/4, false);
    InitTouchButton(TOUCH_BUTTON_DPAD_RIGHT, DPAD_CENTER_X + DPAD_SIZE/3, DPAD_CENTER_Y, DPAD_SIZE/4, false);
    
    // Action buttons
    InitTouchButton(TOUCH_BUTTON_JUMP,        JUMP_BUTTON_X,   JUMP_BUTTON_Y,   BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_SHOOT,       SHOOT_BUTTON_X,  SHOOT_BUTTON_Y,  BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_PUNCH_PICKUP, PUNCH_BUTTON_X, PUNCH_BUTTON_Y,  BUTTON_SIZE, true);
    InitTouchButton(TOUCH_BUTTON_PREV_WEAPON, PREV_WEAPON_X,   PREV_WEAPON_Y,   BUTTON_SIZE * 0.7f, true);
    InitTouchButton(TOUCH_BUTTON_NEXT_WEAPON, NEXT_WEAPON_X,   NEXT_WEAPON_Y,   BUTTON_SIZE * 0.7f, true);
    InitTouchButton(TOUCH_BUTTON_PAUSE,       PAUSE_BUTTON_X,  PAUSE_BUTTON_Y,  BUTTON_SIZE * 0.6f, true);

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
    
    // Check action buttons (not d-pad)
    for (int i = TOUCH_BUTTON_JUMP; i < NUM_TOUCH_BUTTONS; i++)
    {
        TouchButton* btn = &gTouchButtons[i];
        
        float dx = (touchX - btn->centerX) * aspectRatio;
        float dy = touchY - btn->centerY;
        float dist = sqrtf(dx*dx + dy*dy);
        float scaledRadius = btn->radius * gTouchControls.buttonScale;
        
        if (dist <= scaledRadius)
        {
            return i;
        }
    }
    
    return TOUCH_BUTTON_NONE;
}

static bool IsInDpadArea(float touchX, float touchY)
{
    float aspectRatio = (float)gTouchScreenWidth / (float)gTouchScreenHeight;
    float dx = (touchX - DPAD_CENTER_X) * aspectRatio;
    float dy = touchY - DPAD_CENTER_Y;
    float dist = sqrtf(dx*dx + dy*dy);
    float scaledSize = (DPAD_SIZE / 2.0f) * gTouchControls.buttonScale;
    
    return dist <= scaledSize;
}

static void ProcessDpadTouch(float touchX, float touchY)
{
    // Convert touch position to d-pad direction
    float aspectRatio = (float)gTouchScreenWidth / (float)gTouchScreenHeight;
    float dx = (touchX - DPAD_CENTER_X) * aspectRatio;
    float dy = touchY - DPAD_CENTER_Y;
    
    float dist = sqrtf(dx*dx + dy*dy);
    float scaledSize = (DPAD_SIZE / 2.0f) * gTouchControls.buttonScale;
    
    if (dist < DPAD_DEADZONE * scaledSize)
    {
        // In dead zone - no movement
        gTouchControls.analogX = 0;
        gTouchControls.analogY = 0;
        gTouchControls.dpadActive = false;
        return;
    }
    
    // Normalize and clamp
    float normalizedDist = MinFloat(dist / scaledSize, 1.0f);
    float angle = atan2f(dy, dx);
    
    gTouchControls.analogX = cosf(angle) * normalizedDist;
    gTouchControls.analogY = sinf(angle) * normalizedDist;
    gTouchControls.dpadActive = true;
    
    // Set discrete d-pad buttons based on direction
    // Using 8-directional snapping
    float threshold = 0.3f;
    
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_UP]    = (gTouchControls.analogY < -threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_DOWN]  = (gTouchControls.analogY > threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_LEFT]  = (gTouchControls.analogX < -threshold);
    gTouchControls.isPressed[TOUCH_BUTTON_DPAD_RIGHT] = (gTouchControls.analogX > threshold);
}

void TouchControls_Update(void)
{
    if (!gTouchControlsInitialized || !gTouchControls.visible)
        return;
    
    // Update screen dimensions
    gTouchScreenWidth = gGameWindowWidth;
    gTouchScreenHeight = gGameWindowHeight;
    
    // Store previous state for edge detection
    for (int i = 0; i < NUM_TOUCH_BUTTONS; i++)
    {
        gTouchControls.wasPressed[i] = gTouchControls.isPressed[i];
    }
    
    // Clear action button states (d-pad is handled separately)
    for (int i = TOUCH_BUTTON_JUMP; i < NUM_TOUCH_BUTTONS; i++)
    {
        gTouchControls.isPressed[i] = false;
    }
    
    // Clear d-pad state if no finger is tracking it
    if (gDpadFinger == -1)
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
                    
                    // Check if this finger is in the d-pad area
                    if (IsInDpadArea(touchX, touchY))
                    {
                        if (gDpadFinger == -1 || gDpadFinger == finger->id)
                        {
                            gDpadFinger = finger->id;
                            ProcessDpadTouch(touchX, touchY);
                        }
                    }
                    else
                    {
                        // Check action buttons
                        int buttonHit = HitTestButtons(touchX, touchY);
                        if (buttonHit != TOUCH_BUTTON_NONE)
                        {
                            gTouchControls.isPressed[buttonHit] = true;
                            gButtonFingers[buttonHit] = finger->id;
                        }
                    }
                }
                SDL_free(fingers);
            }
        }
        SDL_free(touchDevices);
    }
}

//=============================================================================
// Drawing functions using vertex arrays (OpenGL ES 1.1 compatible)
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
    
    // Disable depth testing and enable blending
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float alpha = gTouchControls.opacity;
    float scale = gTouchControls.buttonScale;
    
    // Draw D-pad
    float dpadX = DPAD_CENTER_X * gTouchScreenWidth;
    float dpadY = DPAD_CENTER_Y * gTouchScreenHeight;
    float dpadSize = DPAD_SIZE * gTouchScreenHeight * scale;
    DrawDpad(dpadX, dpadY, dpadSize, alpha);
    
    // Draw action buttons
    DrawButton(&gTouchButtons[TOUCH_BUTTON_JUMP], 
               gTouchControls.isPressed[TOUCH_BUTTON_JUMP], alpha);
    
    DrawButton(&gTouchButtons[TOUCH_BUTTON_SHOOT],
               gTouchControls.isPressed[TOUCH_BUTTON_SHOOT], alpha);
    
    DrawButton(&gTouchButtons[TOUCH_BUTTON_PUNCH_PICKUP],
               gTouchControls.isPressed[TOUCH_BUTTON_PUNCH_PICKUP], alpha);
    
    DrawButton(&gTouchButtons[TOUCH_BUTTON_PREV_WEAPON],
               gTouchControls.isPressed[TOUCH_BUTTON_PREV_WEAPON], alpha);
    DrawButton(&gTouchButtons[TOUCH_BUTTON_NEXT_WEAPON],
               gTouchControls.isPressed[TOUCH_BUTTON_NEXT_WEAPON], alpha);
    
    DrawButton(&gTouchButtons[TOUCH_BUTTON_PAUSE],
               gTouchControls.isPressed[TOUCH_BUTTON_PAUSE], alpha);
    
    // Restore GL state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void DrawDpad(float centerX, float centerY, float size, float alpha)
{
    float radius = size / 2.0f;
    
    // Draw outer circle (background)
    glColor4f(0.2f, 0.2f, 0.2f, alpha * 0.5f);
    DrawFilledCircle(centerX, centerY, radius, 32);
    
    // Draw outline
    glLineWidth(2.0f);
    glColor4f(0.8f, 0.8f, 0.8f, alpha);
    DrawCircleOutline(centerX, centerY, radius, 32);
    
    // Draw cross/arrows
    float arrowSize = radius * 0.3f;
    float offset = radius * 0.5f;
    
    // Up arrow
    bool upPressed = gTouchControls.isPressed[TOUCH_BUTTON_DPAD_UP];
    glColor4f(upPressed ? 1.0f : 0.7f, upPressed ? 1.0f : 0.7f, upPressed ? 1.0f : 0.7f, alpha);
    DrawTriangle(centerX, centerY - offset - arrowSize,
                 centerX - arrowSize * 0.6f, centerY - offset + arrowSize * 0.5f,
                 centerX + arrowSize * 0.6f, centerY - offset + arrowSize * 0.5f);
    
    // Down arrow
    bool downPressed = gTouchControls.isPressed[TOUCH_BUTTON_DPAD_DOWN];
    glColor4f(downPressed ? 1.0f : 0.7f, downPressed ? 1.0f : 0.7f, downPressed ? 1.0f : 0.7f, alpha);
    DrawTriangle(centerX, centerY + offset + arrowSize,
                 centerX - arrowSize * 0.6f, centerY + offset - arrowSize * 0.5f,
                 centerX + arrowSize * 0.6f, centerY + offset - arrowSize * 0.5f);
    
    // Left arrow
    bool leftPressed = gTouchControls.isPressed[TOUCH_BUTTON_DPAD_LEFT];
    glColor4f(leftPressed ? 1.0f : 0.7f, leftPressed ? 1.0f : 0.7f, leftPressed ? 1.0f : 0.7f, alpha);
    DrawTriangle(centerX - offset - arrowSize, centerY,
                 centerX - offset + arrowSize * 0.5f, centerY - arrowSize * 0.6f,
                 centerX - offset + arrowSize * 0.5f, centerY + arrowSize * 0.6f);
    
    // Right arrow
    bool rightPressed = gTouchControls.isPressed[TOUCH_BUTTON_DPAD_RIGHT];
    glColor4f(rightPressed ? 1.0f : 0.7f, rightPressed ? 1.0f : 0.7f, rightPressed ? 1.0f : 0.7f, alpha);
    DrawTriangle(centerX + offset + arrowSize, centerY,
                 centerX + offset - arrowSize * 0.5f, centerY - arrowSize * 0.6f,
                 centerX + offset - arrowSize * 0.5f, centerY + arrowSize * 0.6f);
    
    // Draw analog position indicator if active
    if (gTouchControls.dpadActive)
    {
        float indicatorX = centerX + gTouchControls.analogX * radius * 0.6f;
        float indicatorY = centerY + gTouchControls.analogY * radius * 0.6f;
        glColor4f(1.0f, 1.0f, 1.0f, alpha);
        DrawFilledCircle(indicatorX, indicatorY, radius * 0.15f, 16);
    }
}

static void DrawButton(const TouchButton* button, bool pressed, float alpha)
{
    float scale = gTouchControls.buttonScale;
    float cx = button->centerX * gTouchScreenWidth;
    float cy = button->centerY * gTouchScreenHeight;
    float r = button->radius * gTouchScreenHeight * scale;
    
    // Set color based on button type
    float baseR = 0.3f, baseG = 0.3f, baseB = 0.3f;
    
    switch (button->buttonID)
    {
        case TOUCH_BUTTON_JUMP:         baseR = 0.2f; baseG = 0.7f; baseB = 0.2f; break;  // Green
        case TOUCH_BUTTON_SHOOT:        baseR = 0.2f; baseG = 0.4f; baseB = 0.8f; break;  // Blue
        case TOUCH_BUTTON_PUNCH_PICKUP: baseR = 0.8f; baseG = 0.2f; baseB = 0.2f; break;  // Red
        case TOUCH_BUTTON_PREV_WEAPON:
        case TOUCH_BUTTON_NEXT_WEAPON:  baseR = 0.8f; baseG = 0.7f; baseB = 0.2f; break;  // Yellow
        case TOUCH_BUTTON_PAUSE:        baseR = 0.5f; baseG = 0.5f; baseB = 0.5f; break;  // Gray
    }
    
    if (pressed)
    {
        baseR = MinFloat(baseR * 1.5f, 1.0f);
        baseG = MinFloat(baseG * 1.5f, 1.0f);
        baseB = MinFloat(baseB * 1.5f, 1.0f);
    }
    
    // Draw button background
    glColor4f(baseR, baseG, baseB, alpha * 0.7f);
    DrawFilledCircle(cx, cy, r, 24);
    
    // Draw button outline
    glLineWidth(pressed ? 4.0f : 2.0f);
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    DrawCircleOutline(cx, cy, r, 24);
    
    // Draw button label/icon
    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    float iconSize = r * 0.5f;
    
    switch (button->buttonID)
    {
        case TOUCH_BUTTON_JUMP:
            // Draw up arrow
            DrawTriangle(cx, cy - iconSize,
                         cx - iconSize * 0.7f, cy + iconSize * 0.5f,
                         cx + iconSize * 0.7f, cy + iconSize * 0.5f);
            break;
            
        case TOUCH_BUTTON_SHOOT:
            // Draw crosshair
            glLineWidth(3.0f);
            DrawLine(cx - iconSize, cy, cx + iconSize, cy);
            DrawLine(cx, cy - iconSize, cx, cy + iconSize);
            DrawCircleOutline(cx, cy, iconSize * 0.5f, 12);
            break;
            
        case TOUCH_BUTTON_PUNCH_PICKUP:
            // Draw fist/hand icon (simple box)
            DrawQuad(cx - iconSize * 0.6f, cy - iconSize * 0.6f,
                     cx + iconSize * 0.6f, cy - iconSize * 0.6f,
                     cx + iconSize * 0.6f, cy + iconSize * 0.6f,
                     cx - iconSize * 0.6f, cy + iconSize * 0.6f);
            break;
            
        case TOUCH_BUTTON_PREV_WEAPON:
            // Draw left arrow
            DrawTriangle(cx - iconSize, cy,
                         cx + iconSize * 0.3f, cy - iconSize * 0.7f,
                         cx + iconSize * 0.3f, cy + iconSize * 0.7f);
            break;
            
        case TOUCH_BUTTON_NEXT_WEAPON:
            // Draw right arrow
            DrawTriangle(cx + iconSize, cy,
                         cx - iconSize * 0.3f, cy - iconSize * 0.7f,
                         cx - iconSize * 0.3f, cy + iconSize * 0.7f);
            break;
            
        case TOUCH_BUTTON_PAUSE:
            // Draw pause bars
            DrawQuad(cx - iconSize * 0.5f, cy - iconSize * 0.6f,
                     cx - iconSize * 0.1f, cy - iconSize * 0.6f,
                     cx - iconSize * 0.1f, cy + iconSize * 0.6f,
                     cx - iconSize * 0.5f, cy + iconSize * 0.6f);
            
            DrawQuad(cx + iconSize * 0.1f, cy - iconSize * 0.6f,
                     cx + iconSize * 0.5f, cy - iconSize * 0.6f,
                     cx + iconSize * 0.5f, cy + iconSize * 0.6f,
                     cx + iconSize * 0.1f, cy + iconSize * 0.6f);
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
            
            if (IsInDpadArea(touchX, touchY))
            {
                if (gDpadFinger == -1)
                {
                    gDpadFinger = fingerID;
                    ProcessDpadTouch(touchX, touchY);
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
            
            if (fingerID == gDpadFinger)
            {
                ProcessDpadTouch(touchX, touchY);
            }
            break;
        }
        
        case SDL_EVENT_FINGER_UP:
        {
            SDL_FingerID fingerID = event->tfinger.fingerID;
            
            if (fingerID == gDpadFinger)
            {
                gDpadFinger = -1;
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
                        gButtonFingers[i] = -1;
                        break;
                    }
                }
            }
            break;
        }
    }
}
