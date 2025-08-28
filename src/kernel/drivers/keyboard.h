#pragma once
#include "stdint.h"
#include "stdbool.h"
#include "keycode.h"

void Keyboard_Initialize();
void Keyboard_HandleByte(uint8_t byte);

// Key state map API
bool Keyboard_IsKeyDown(KeyCode keyCode);

// Character queue API
uint8_t Keyboard_GetCharacter();
bool Keyboard_CharacterQueueEmpty();
void Keyboard_FlushCharacterQueue();

// Key event queue API
typedef enum {
    KEY_EVENT_TYPE_NONE = 0,
    KEY_EVENT_TYPE_PRESS,
    KEY_EVENT_TYPE_RELEASE,
} KeyEventType;

typedef struct {
    KeyCode code;
    KeyEventType type;
} KeyEvent;

KeyEvent Keyboard_PollEvent();
bool Keyboard_EventQueueEmpty();
void Keyboard_FlushEventQueue();
