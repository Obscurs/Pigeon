#pragma once

// From WinUser.h
constexpr unsigned char WIN_PG_MOUSE_BUTTON_LEFT = 0x01; // VK_LBUTTON
constexpr unsigned char WIN_PG_MOUSE_BUTTON_RIGHT = 0x02; // VK_RBUTTON
constexpr unsigned char WIN_PG_MOUSE_BUTTON_MIDDLE = 0x04; // VK_MBUTTON

constexpr unsigned char WIN_PG_MOUSE_BUTTON_4 = 0x05; // XBUTTON1
constexpr unsigned char WIN_PG_MOUSE_BUTTON_5 = 0x06; // XBUTTON2

constexpr unsigned char WIN_PG_MOUSE_BUTTON_6 = 0x07;
constexpr unsigned char WIN_PG_MOUSE_BUTTON_7 = 0x08;
constexpr unsigned char WIN_PG_MOUSE_BUTTON_8 = 0x09;

constexpr unsigned char WIN_PG_MOUSE_BUTTON_LAST = WIN_PG_MOUSE_BUTTON_8;

// From WinUser.h
constexpr unsigned char WIN_PG_KEY_0 = 0x30;
constexpr unsigned char WIN_PG_KEY_1 = 0x31;
constexpr unsigned char WIN_PG_KEY_2 = 0x32;
constexpr unsigned char WIN_PG_KEY_3 = 0x33;
constexpr unsigned char WIN_PG_KEY_4 = 0x34;
constexpr unsigned char WIN_PG_KEY_5 = 0x35;
constexpr unsigned char WIN_PG_KEY_6 = 0x36;
constexpr unsigned char WIN_PG_KEY_7 = 0x37;
constexpr unsigned char WIN_PG_KEY_8 = 0x38;
constexpr unsigned char WIN_PG_KEY_9 = 0x39;
constexpr unsigned char WIN_PG_KEY_A = 0x41;
constexpr unsigned char WIN_PG_KEY_B = 0x42;
constexpr unsigned char WIN_PG_KEY_C = 0x43;
constexpr unsigned char WIN_PG_KEY_D = 0x44;
constexpr unsigned char WIN_PG_KEY_E = 0x45;
constexpr unsigned char WIN_PG_KEY_F = 0x46;
constexpr unsigned char WIN_PG_KEY_G = 0x47;
constexpr unsigned char WIN_PG_KEY_H = 0x48;
constexpr unsigned char WIN_PG_KEY_I = 0x49;
constexpr unsigned char WIN_PG_KEY_J = 0x4A;
constexpr unsigned char WIN_PG_KEY_K = 0x4B;
constexpr unsigned char WIN_PG_KEY_L = 0x4C;
constexpr unsigned char WIN_PG_KEY_M = 0x4D;
constexpr unsigned char WIN_PG_KEY_N = 0x4E;
constexpr unsigned char WIN_PG_KEY_O = 0x4F;
constexpr unsigned char WIN_PG_KEY_P = 0x50;
constexpr unsigned char WIN_PG_KEY_Q = 0x51;
constexpr unsigned char WIN_PG_KEY_R = 0x52;
constexpr unsigned char WIN_PG_KEY_S = 0x53;
constexpr unsigned char WIN_PG_KEY_T = 0x54;
constexpr unsigned char WIN_PG_KEY_U = 0x55;
constexpr unsigned char WIN_PG_KEY_V = 0x56;
constexpr unsigned char WIN_PG_KEY_W = 0x57;
constexpr unsigned char WIN_PG_KEY_X = 0x58;
constexpr unsigned char WIN_PG_KEY_Y = 0x59;
constexpr unsigned char WIN_PG_KEY_Z = 0x5A;

/* Function keys */
constexpr unsigned char WIN_PG_KEY_SPACE = VK_SPACE;
constexpr unsigned char WIN_PG_KEY_ESCAPE = VK_ESCAPE;
constexpr unsigned char WIN_PG_KEY_ENTER = VK_RETURN;
constexpr unsigned char WIN_PG_KEY_TAB = VK_TAB;
constexpr unsigned char WIN_PG_KEY_BACKSPACE = VK_BACK;
constexpr unsigned char WIN_PG_KEY_INSERT = VK_INSERT;
constexpr unsigned char WIN_PG_KEY_DELETE = VK_DELETE;
constexpr unsigned char WIN_PG_KEY_RIGHT = VK_RIGHT;
constexpr unsigned char WIN_PG_KEY_LEFT = VK_LEFT;
constexpr unsigned char WIN_PG_KEY_DOWN = VK_DOWN;
constexpr unsigned char WIN_PG_KEY_UP = VK_UP;
constexpr unsigned char WIN_PG_KEY_PAGE_UP = VK_PRIOR;
constexpr unsigned char WIN_PG_KEY_PAGE_DOWN = VK_NEXT;
constexpr unsigned char WIN_PG_KEY_HOME = VK_HOME;
constexpr unsigned char WIN_PG_KEY_END = VK_END;
constexpr unsigned char WIN_PG_KEY_CAPS_LOCK = VK_CAPITAL;
constexpr unsigned char WIN_PG_KEY_CONTROL = VK_CONTROL;
constexpr unsigned char WIN_PG_KEY_ALT = VK_MENU;
constexpr unsigned char WIN_PG_KEY_SHIFT = VK_SHIFT;
constexpr unsigned char WIN_PG_KEY_PAUSE = VK_PAUSE;
constexpr unsigned char WIN_PG_KEY_F1 = VK_F1;
constexpr unsigned char WIN_PG_KEY_F2 = VK_F2;
constexpr unsigned char WIN_PG_KEY_F3 = VK_F3;
constexpr unsigned char WIN_PG_KEY_F4 = VK_F4;
constexpr unsigned char WIN_PG_KEY_F5 = VK_F5;
constexpr unsigned char WIN_PG_KEY_F6 = VK_F6;
constexpr unsigned char WIN_PG_KEY_F7 = VK_F7;
constexpr unsigned char WIN_PG_KEY_F8 = VK_F8;
constexpr unsigned char WIN_PG_KEY_F9 = VK_F9;
constexpr unsigned char WIN_PG_KEY_F10 = VK_F10;
constexpr unsigned char WIN_PG_KEY_F11 = VK_F11;
constexpr unsigned char WIN_PG_KEY_F12 = VK_F12;
constexpr unsigned char WIN_PG_KEY_F13 = VK_F13;
constexpr unsigned char WIN_PG_KEY_F14 = VK_F14;
constexpr unsigned char WIN_PG_KEY_F15 = VK_F15;
constexpr unsigned char WIN_PG_KEY_F16 = VK_F16;
constexpr unsigned char WIN_PG_KEY_F17 = VK_F17;
constexpr unsigned char WIN_PG_KEY_F18 = VK_F18;
constexpr unsigned char WIN_PG_KEY_F19 = VK_F19;
constexpr unsigned char WIN_PG_KEY_F20 = VK_F20;
constexpr unsigned char WIN_PG_KEY_F21 = VK_F21;
constexpr unsigned char WIN_PG_KEY_F22 = VK_F22;
constexpr unsigned char WIN_PG_KEY_F23 = VK_F23;
constexpr unsigned char WIN_PG_KEY_F24 = VK_F24;
//constexpr unsigned char WIN_PG_KEY_F25 = VK_F25;
constexpr unsigned char WIN_PG_KEY_KP_0 = VK_NUMPAD0;
constexpr unsigned char WIN_PG_KEY_KP_1 = VK_NUMPAD1;
