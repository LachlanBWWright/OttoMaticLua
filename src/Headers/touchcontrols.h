// TOUCH CONTROLS FOR ANDROID
// On-screen virtual gamepad for touch devices
// (c)2025 Otto Matic Android Port

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Touch control button IDs
enum
{
    TOUCH_BUTTON_NONE = -1,
    
    // D-pad directions
    TOUCH_BUTTON_DPAD_UP,
    TOUCH_BUTTON_DPAD_DOWN,
    TOUCH_BUTTON_DPAD_LEFT,
    TOUCH_BUTTON_DPAD_RIGHT,
    
    // Action buttons
    TOUCH_BUTTON_JUMP,
    TOUCH_BUTTON_SHOOT,
    TOUCH_BUTTON_PUNCH_PICKUP,
    TOUCH_BUTTON_PREV_WEAPON,
    TOUCH_BUTTON_NEXT_WEAPON,
    TOUCH_BUTTON_PAUSE,
    TOUCH_BUTTON_DEBUG_TOGGLE,
    
    NUM_TOUCH_BUTTONS
};

// Touch control state structure
typedef struct
{
    bool        isPressed[NUM_TOUCH_BUTTONS];
    bool        wasPressed[NUM_TOUCH_BUTTONS];  // For detecting new presses
    
    // Analog stick state (for d-pad)
    float       analogX;        // -1.0 to 1.0
    float       analogY;        // -1.0 to 1.0
    bool        dpadActive;     // true if touch is on d-pad area
    
    // Visibility and layout
    bool        visible;
    float       opacity;
    float       buttonScale;
} TouchControlState;

// Initialize touch controls system
void TouchControls_Init(void);

// Shutdown touch controls system
void TouchControls_Shutdown(void);

// Update touch controls (call each frame before UpdateInput)
void TouchControls_Update(void);

// Draw touch controls overlay (call after main rendering)
void TouchControls_Draw(void);

// Check if a touch control button is pressed
bool TouchControls_IsPressed(int buttonID);

// Check if a touch control button was just pressed this frame
bool TouchControls_IsNewPress(int buttonID);

// Get analog stick values (-1.0 to 1.0)
void TouchControls_GetAnalog(float* outX, float* outY);

// Set touch controls visibility
void TouchControls_SetVisible(bool visible);

// Check if touch controls are visible/active
bool TouchControls_IsVisible(void);

// Configure touch control layout and opacity
void TouchControls_SetOpacity(float opacity);
void TouchControls_SetScale(float scale);

// Handle SDL touch events
void TouchControls_HandleEvent(SDL_Event* event);

// Global touch control state
extern TouchControlState gTouchControls;

// Check if we are on a touch device (Android)
bool IsTouchDevice(void);

#ifdef __cplusplus
}
#endif
