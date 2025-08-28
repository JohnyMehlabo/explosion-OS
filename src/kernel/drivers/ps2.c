#include "arch/i686/i8042.h"
#include "memory.h"
#include "arch/i686/irq.h"
#include "keyboard.h"

#define MAX_CMD_LEN 4

typedef struct {
    uint8_t bytes[MAX_CMD_LEN];  // command + args
    uint8_t len;
    uint8_t pos;                 // current send position
    uint8_t retries;
    bool waiting_ack;
} PS2Command;

#define MAX_CMD_QUEUE_SIZE 8

typedef struct {
    PS2Command commands[MAX_CMD_QUEUE_SIZE];
    uint8_t head, tail;
    bool busy;
} PS2CommandQueue;

static PS2CommandQueue queue;

void PS2_IRQHandler();

void PS2_Initialize() {
    uint8_t dummy;
    i686_i8042_ReadByte(&dummy); // Read byte to ensure buffer is empty
    i686_IRQ_SetHandler(1, PS2_IRQHandler);    

    Keyboard_Initialize();
}

void PS2_SendNextCommand() {
    if (queue.tail == queue.head) {
        queue.busy = false;
        return;
    }
    
    PS2Command* cmd = &queue.commands[queue.head];
    queue.busy = true;

    i686_i8042_WriteByte(cmd->bytes[cmd->pos++]);
    cmd->waiting_ack = true;
}

bool PS2_EnqueueCommand(uint8_t* bytes, uint8_t len) {
    uint8_t next = (queue.tail + 1) % MAX_CMD_QUEUE_SIZE;
    if (next == queue.head) return false;
    
    memcpy(queue.commands[queue.tail].bytes, bytes, len);
    queue.commands[queue.tail].len = len;
    queue.commands[queue.tail].pos = 0;
    queue.commands[queue.tail].retries = 3;
    queue.commands[queue.tail].waiting_ack = false;

    queue.tail = next;

    if (!queue.busy)
        PS2_SendNextCommand();

    return true;
}

void PS2_IRQHandler() {
    uint8_t byte;
    if (!i686_i8042_ReadByte(&byte)) return;

    if (queue.busy) {
        PS2Command* cmd = &queue.commands[queue.head];

        if (cmd->waiting_ack) {
            if (byte == 0xfa) { // Got ACK
                if (cmd->pos < cmd->len) { // There are bytes left to send
                    i686_i8042_WriteByte(cmd->bytes[cmd->pos++]);
                    cmd->waiting_ack = true;
                } else { // Load new command
                    queue.head = (queue.head + 1) % MAX_CMD_QUEUE_SIZE;
                    PS2_SendNextCommand();
                }
                return;
            } else if (byte == 0xfe) { // Got resend
                if (cmd->retries-- > 0) {
                    cmd->pos--;
                    i686_i8042_WriteByte(cmd->bytes[cmd->pos++]);
                    cmd->waiting_ack = true;
                } else {
                    queue.head = (queue.head + 1) % MAX_CMD_QUEUE_SIZE;
                    PS2_SendNextCommand();
                }
                return;
            }
        }
    }

    // TODO: Byte isnt part of instruction, send to keyboard
    Keyboard_HandleByte(byte);
}
