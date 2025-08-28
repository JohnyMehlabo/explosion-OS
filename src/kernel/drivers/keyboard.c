#include "keyboard.h"
#include "stdio.h"
#include "ps2.h"

static bool scroll, number, caps;

void Keyboard_SetLEDs(bool scroll, bool number, bool caps) {
    uint8_t cmd[] = { 0xed, 0x00 };
    
    uint8_t data = 0;
    data = scroll ? data | 0x1 : data;
    data = number ? data | 0x2 : data;
    data = caps ? data | 0x4 : data;
    cmd[1] = data;
    
    PS2_EnqueueCommand(cmd, 2);
}

// TODO: Should switch to scan code 1
void Keyboard_Initialize() {
    Keyboard_SetLEDs(scroll, number, caps);
}

bool keyStateMap[KEY_COUNT] = {0};

#define MAX_CHARACTER_QUEUE_SIZE 256

typedef struct {
    uint8_t characters[MAX_CHARACTER_QUEUE_SIZE];
    uint8_t head, tail;
} CharacterQueue;

CharacterQueue characterQueue;

void Keyboard_PushCharacter(uint8_t character) {
    uint8_t next = (characterQueue.tail + 1) % MAX_CHARACTER_QUEUE_SIZE;
    if (characterQueue.head == next) { // Queue is full, discard
        return;
    }

    characterQueue.characters[characterQueue.tail] = character;

    characterQueue.tail = next;
}

// NOTE: if the queue is empty returns 0
uint8_t Keyboard_GetCharacter() {
    if (characterQueue.head == characterQueue.tail) return 0;

    uint8_t character = characterQueue.characters[characterQueue.head];
    characterQueue.head = (characterQueue.head + 1) % MAX_CHARACTER_QUEUE_SIZE;
    return character;
}

bool Keyboard_CharacterQueueEmpty() {
    return characterQueue.head == characterQueue.tail;
}

void Keyboard_FlushCharacterQueue() {
    characterQueue.head = characterQueue.tail;
}

typedef struct {
    uint8_t lower;
    uint8_t upper;
} KeyChar;

uint8_t Keyboard_KeyCodeToCharacter(KeyCode keyCode) {
    static const KeyChar keycodeToASCII[] = {
        [KEY_A] = {'a', 'A'},
        [KEY_B] = {'b', 'B'},
        [KEY_C] = {'c', 'C'},
        [KEY_D] = {'d', 'D'},
        [KEY_E] = {'e', 'E'},
        [KEY_F] = {'f', 'F'},
        [KEY_G] = {'g', 'G'},
        [KEY_H] = {'h', 'H'},
        [KEY_I] = {'i', 'I'},
        [KEY_J] = {'j', 'J'},
        [KEY_K] = {'k', 'K'},
        [KEY_L] = {'l', 'L'},
        [KEY_M] = {'m', 'M'},
        [KEY_N] = {'n', 'N'},
        [KEY_O] = {'o', 'O'},
        [KEY_P] = {'p', 'P'},
        [KEY_Q] = {'q', 'Q'},
        [KEY_R] = {'r', 'R'},
        [KEY_S] = {'s', 'S'},
        [KEY_T] = {'t', 'T'},
        [KEY_U] = {'u', 'U'},
        [KEY_V] = {'v', 'V'},
        [KEY_W] = {'w', 'W'},
        [KEY_X] = {'x', 'X'},
        [KEY_Y] = {'y', 'Y'},
        [KEY_Z] = {'z', 'Z'},
        [KEY_0] = {'0', '='},
        [KEY_1] = {'1', '!'},
        [KEY_2] = {'2', '"'},
        [KEY_3] = {'3', '\0'},
        [KEY_4] = {'4', '$'},
        [KEY_5] = {'5', '%'},
        [KEY_6] = {'6', '&'},
        [KEY_7] = {'7', '/'},
        [KEY_8] = {'8', '('},
        [KEY_9] = {'9', ')'},
        [KEY_SPACE] = {' ', ' '},
        [KEY_ENTER] = {'\n', '\n'},
        [KEY_BACKSPACE] = {'\b', '\b'},
    };
    
    bool shift = keyStateMap[KEY_LSHIFT] ^ caps;
    return shift ? keycodeToASCII[keyCode].upper : keycodeToASCII[keyCode].lower;
}

// Key event queue API

#define MAX_EVENT_QUEUE_SIZE 256

typedef struct {
    KeyEvent events[MAX_EVENT_QUEUE_SIZE    ];
    uint8_t head, tail;
} EventQueue;

EventQueue eventQueue;

void Keyboard_PushEvent(KeyEvent event) {
    uint8_t next = (eventQueue.tail + 1) % MAX_EVENT_QUEUE_SIZE;
    if (eventQueue.head == next) { // Queue is full, discard
        return;
    }

    eventQueue.events[eventQueue.tail] = event;

    eventQueue.tail = next;
}

// NOTE: if the queue is empty return an error event
KeyEvent Keyboard_PollEvent() {
    static KeyEvent errorKeyEvent = { .code=KEY_INVALID, .type=KEY_EVENT_TYPE_NONE};
    if (eventQueue.head == eventQueue.tail) return errorKeyEvent;

    KeyEvent event = eventQueue.events[eventQueue.head];
    eventQueue.head = (eventQueue.head + 1) % MAX_EVENT_QUEUE_SIZE;
    return event;
}

bool Keyboard_EventQueueEmpty() {
    return eventQueue.head == eventQueue.tail;
}

void Keyboard_FlushEventQueue() {
    eventQueue.head = eventQueue.tail;
}

typedef enum {
    STATE_DEFAULT,
    STATE_E0,
} KeyboardState;

static KeyboardState currentState = STATE_DEFAULT;

void Keyboard_HandleByte(uint8_t byte) {
    switch (currentState)
    {
    case STATE_DEFAULT:
        if (byte == 0xe0) {
            currentState = STATE_E0;
            return;
        }
        if (!(byte & 0x80)) {
            KeyCode keyCode = scancodeMapping[byte];
            keyStateMap[keyCode] = true;

            uint8_t character = Keyboard_KeyCodeToCharacter(keyCode);
            if (character != 0) Keyboard_PushCharacter(character);

            KeyEvent event = {.code=keyCode, .type=KEY_EVENT_TYPE_PRESS};
            Keyboard_PushEvent(event);

            // TODO: Change this to use our keycodes
            if (byte == 0x46) scroll = !scroll;
            if (byte == 0x45) number = !number;
            if (byte == 0x3a) caps = !caps;
            if (byte == 0x3a || byte == 0x45 || byte == 0x46) Keyboard_SetLEDs(scroll, number, caps);
        }
        else {
            KeyCode keyCode = scancodeMapping[byte & ~0x80];
            keyStateMap[keyCode] = false;

            KeyEvent event = {.code=keyCode, .type=KEY_EVENT_TYPE_RELEASE};
            Keyboard_PushEvent(event);
        }
        break;
    
    case STATE_E0:
        currentState = STATE_DEFAULT;
        if (!(byte & 0x80)) 
            puts("Compound key pressed\n");
        else 
            puts("Compound key released\n");
        break;
    default:
        break;
    }
}

bool Keyboard_IsKeyDown(KeyCode keyCode) {
    return keyStateMap[keyCode];
}