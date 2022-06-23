#include "keyboard.hpp"

#include <memory>
#include "usb/classdriver/keyboard.hpp"
#include "task.hpp"

#include "asmfunc.h"
#include "logger.hpp"
#include "layer.hpp"

namespace {

const char keycode_map[256] = {
  0,    0,    0,    0,    'a',  'b',  'c',  'd', // 0
  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l', // 8
  'm',  'n',  'o',  'p',  'q',  'r',  's',  't', // 16
  'u',  'v',  'w',  'x',  'y',  'z',  '1',  '2', // 24
  '3',  '4',  '5',  '6',  '7',  '8',  '9',  '0', // 32
  '\n', '\b', 0x08, '\t', ' ',  '-',  '=',  '[', // 40
  ']', '\\',  '#',  ';', '\'',  '`',  ',',  '.', // 48
  '/',  0,    0,    0,    0,    0,    0,    0,   // 56
  0,    0,    0,    0,    0,    0,    0,    0,   // 64
  0,    0,    0,    0,    0,    0,    0,    0,   // 72
  0,    0,    0,    0,    '/',  '*',  '-',  '+', // 80
  '\n', '1',  '2',  '3',  '4',  '5',  '6',  '7', // 88
  '8',  '9',  '0',  '.', '\\',  0,    0,    '=', // 96
  0,    0,    0,    0,    0,    0,    0,    0,   // 104
  0,    0,    0,    0,    0,    0,    0,    0,   // 112
  0,    0,    0,    0,    0,    0,    0,    0,   // 120
  0,    0,    0,    0,    0,    0,    0,    0,   // 128
  0,    '\\', 0,    0,    0,    0,    0,    0,   // 136
};

const char keycode_map_shifted[256] = {
  0,    0,    0,    0,    'A',  'B',  'C',  'D', // 0
  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L', // 8
  'M',  'N',  'O',  'P',  'Q',  'R',  'S',  'T', // 16
  'U',  'V',  'W',  'X',  'Y',  'Z',  '!',  '@', // 24
  '#',  '$',  '%',  '^',  '&',  '*',  '(',  ')', // 32
  '\n', '\b', 0x08, '\t', ' ',  '_',  '+',  '{', // 40
  '}',  '|',  '~',  ':',  '"',  '~',  '<',  '>', // 48
  '?',  0,    0,    0,    0,    0,    0,    0,   // 56
  0,    0,    0,    0,    0,    0,    0,    0,   // 64
  0,    0,    0,    0,    0,    0,    0,    0,   // 72
  0,    0,    0,    0,    '/',  '*',  '-',  '+', // 80
  '\n', '1',  '2',  '3',  '4',  '5',  '6',  '7', // 88
  '8',  '9',  '0',  '.', '\\',  0,    0,    '=', // 96
  0,    0,    0,    0,    0,    0,    0,    0,   // 104
  0,    0,    0,    0,    0,    0,    0,    0,   // 112
  0,    0,    0,    0,    0,    0,    0,    0,   // 120
  0,    0,    0,    0,    0,    0,    0,    0,   // 128
  0,    '|',  0,    0,    0,    0,    0,    0,   // 136
};

const char ps2_keycode_map[0x3a] = {
  0,    0,   '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', //0x0
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', // 0xF
  0,    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', 0,   0, // 0x1D
  '|',  'z', 'x', 'c' ,'v', 'b', 'n', 'm', ',', '.', '/', 0,    0,   0,  ' ', // 0x2B
};

const char ps2_keycode_map_shifted[0x3a] = {
  0,    0,   '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', //0x0
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', // 0xF
  0,    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', 0,   0, // 0x1D
  '\\', 'Z', 'X', 'C' ,'V', 'B', 'N', 'M', '<', '>', '?', 0,    0,   0,  ' ', // 0x2B
};

} // namespace

void InitializeKeyboard() {
  usb::HIDKeyboardDriver::default_observer =
    [](uint8_t modifier, uint8_t keycode, bool press) {
      const bool shift = (modifier & (kLShiftBitMask | kRShiftBitMask)) != 0;
      char ascii = keycode_map[keycode];
      if (shift) {
        ascii = keycode_map_shifted[keycode];
      }
      Message msg{Message::kKeyPush};
      msg.arg.keyboard.modifier = modifier;
      msg.arg.keyboard.keycode = keycode;
      msg.arg.keyboard.ascii = ascii;
      msg.arg.keyboard.press = press;
      task_manager->SendMessage(1, msg);
    };
}

void InitializePS2Keyboard() {
  while ((IoIn32(0x64) & 0x02) != 0);
  IoOut8(0x64, 0x60);
  while ((IoIn32(0x64) & 0x02) != 0);
  IoOut8(0x60, 0x47);
}

void KeyboardEvent() {
    static auto mouse = layer_manager->Mouse;
    static int mouse_button = 0;
    static int modif = 0b00;
    Message msg{Message::kKeyPush};
    uint32_t raw = IoIn32(0x60);
    uint32_t key = raw & 0x0000'00ff;

    Log(kDebug, "%x\n", raw);

    msg.arg.keyboard.ascii = 0;
    msg.arg.keyboard.press = 1;
    msg.arg.keyboard.keycode = 0;

    switch (key) {
      case 0x1d: // left ctrl make
        modif = modif | 0b01;
        break;
      case 0x9d: // left ctrl break
        modif = modif & 0b10;
        msg.arg.keyboard.press = 0;
        break;
      case 0x2a: // left shift make
        modif = modif | 0b10;
        break;
      case 0xaa: // left shift break
        modif = modif & 0b01;
        msg.arg.keyboard.press = 0;
        break;
    }
    msg.arg.keyboard.modifier = modif;

    int dx = 0;
    int dy = 0;
    if (modif & 1) {
      switch ((raw & 0x0000ff00) >> 8) {
        case 0x48:
          dy = -10;
          Log(kDebug, "Top\n");
          break;
        case 0x4B:
          dx = -10;
          Log(kDebug, "Left\n");
          break;
        case 0x50:
          dy = 10;
          Log(kDebug, "Buttom\n");
          break;
        case 0x4D:
          dx = 10;
          Log(kDebug, "Right\n");
          break;
      }
      switch (key) {
        case 0x10:
          msg.arg.keyboard.ascii = 'q';
          msg.arg.keyboard.keycode = 0x14;
          __asm__("cli");
          task_manager->SendMessage(1, msg);
          __asm__("sti");

          return;
        case 0x20:
          msg.arg.keyboard.ascii = 'd';
          msg.arg.keyboard.keycode = 0x07;
          __asm__("cli");
          task_manager->SendMessage(1, msg);
          __asm__("sti");

          return;
        case 0x1C:
          mouse_button = 1;
          break;
        case 0x9C:
          mouse_button = 0;
          break;
      }
      mouse->OnInterrupt(mouse_button, dx, dy);
      return;
    }

    if (0x0 <= key && key <= 0x39) { // make
        if (modif & 0b10) {
          msg.arg.keyboard.ascii = ps2_keycode_map_shifted[key];
        } else {
          msg.arg.keyboard.ascii = ps2_keycode_map[key];
        }
    } else if (0x80 <= key && key <= 0xB9) { // break
        if (modif & 0b10) {
          msg.arg.keyboard.ascii = ps2_keycode_map_shifted[key - 0x80];
        } else {
          msg.arg.keyboard.ascii = ps2_keycode_map[key - 0x80];
        }
        msg.arg.keyboard.press = 0;
    } 

    __asm__("cli");
    task_manager->SendMessage(1, msg);
    __asm__("sti");
    return;
}
