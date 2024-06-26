#pragma once

namespace pig
{
	constexpr unsigned char PG_MOUSE_BUTTON_LEFT = 0x01; // VK_LBUTTON
	constexpr unsigned char PG_MOUSE_BUTTON_RIGHT = 0x02; // VK_RBUTTON
	constexpr unsigned char PG_MOUSE_BUTTON_MIDDLE = 0x03; // VK_MBUTTON

	constexpr unsigned char PG_MOUSE_BUTTON_4 = 0x04; // XBUTTON1
	constexpr unsigned char PG_MOUSE_BUTTON_5 = 0x05; // XBUTTON2

	constexpr unsigned char PG_MOUSE_BUTTON_6 = 0x06;
	constexpr unsigned char PG_MOUSE_BUTTON_7 = 0x07;
	constexpr unsigned char PG_MOUSE_BUTTON_8 = 0x08;

	constexpr unsigned char PG_KEY_0 = 0x30;
	constexpr unsigned char PG_KEY_1 = 0x31;
	constexpr unsigned char PG_KEY_2 = 0x32;
	constexpr unsigned char PG_KEY_3 = 0x33;
	constexpr unsigned char PG_KEY_4 = 0x34;
	constexpr unsigned char PG_KEY_5 = 0x35;
	constexpr unsigned char PG_KEY_6 = 0x36;
	constexpr unsigned char PG_KEY_7 = 0x37;
	constexpr unsigned char PG_KEY_8 = 0x38;
	constexpr unsigned char PG_KEY_9 = 0x39;
	constexpr unsigned char PG_KEY_A = 0x41;
	constexpr unsigned char PG_KEY_B = 0x42;
	constexpr unsigned char PG_KEY_C = 0x43;
	constexpr unsigned char PG_KEY_D = 0x44;
	constexpr unsigned char PG_KEY_E = 0x45;
	constexpr unsigned char PG_KEY_F = 0x46;
	constexpr unsigned char PG_KEY_G = 0x47;
	constexpr unsigned char PG_KEY_H = 0x48;
	constexpr unsigned char PG_KEY_I = 0x49;
	constexpr unsigned char PG_KEY_J = 0x4A;
	constexpr unsigned char PG_KEY_K = 0x4B;
	constexpr unsigned char PG_KEY_L = 0x4C;
	constexpr unsigned char PG_KEY_M = 0x4D;
	constexpr unsigned char PG_KEY_N = 0x4E;
	constexpr unsigned char PG_KEY_O = 0x4F;
	constexpr unsigned char PG_KEY_P = 0x50;
	constexpr unsigned char PG_KEY_Q = 0x51;
	constexpr unsigned char PG_KEY_R = 0x52;
	constexpr unsigned char PG_KEY_S = 0x53;
	constexpr unsigned char PG_KEY_T = 0x54;
	constexpr unsigned char PG_KEY_U = 0x55;
	constexpr unsigned char PG_KEY_V = 0x56;
	constexpr unsigned char PG_KEY_W = 0x57;
	constexpr unsigned char PG_KEY_X = 0x58;
	constexpr unsigned char PG_KEY_Y = 0x59;
	constexpr unsigned char PG_KEY_Z = 0x5A;

	/* Function keys */
	constexpr unsigned char PG_KEY_SPACE = 0x80;
	constexpr unsigned char PG_KEY_ESCAPE = 0x81;
	constexpr unsigned char PG_KEY_ENTER = 0x82;
	constexpr unsigned char PG_KEY_TAB = 0x83;
	constexpr unsigned char PG_KEY_BACKSPACE = 0x84;
	constexpr unsigned char PG_KEY_INSERT = 0x84;
	constexpr unsigned char PG_KEY_DELETE = 0x85;
	constexpr unsigned char PG_KEY_RIGHT = 0x86;
	constexpr unsigned char PG_KEY_LEFT = 0x87;
	constexpr unsigned char PG_KEY_DOWN = 0x88;
	constexpr unsigned char PG_KEY_UP = 0x89;
	constexpr unsigned char PG_KEY_PAGE_UP = 0x8A;
	constexpr unsigned char PG_KEY_PAGE_DOWN = 0x8B;
	constexpr unsigned char PG_KEY_HOME = 0x8C;
	constexpr unsigned char PG_KEY_END = 0x8D;
	constexpr unsigned char PG_KEY_CAPS_LOCK = 0x8E;
	constexpr unsigned char PG_KEY_CONTROL = 0x8F;
	constexpr unsigned char PG_KEY_ALT = 0x90;
	constexpr unsigned char PG_KEY_SHIFT = 0x91;
	constexpr unsigned char PG_KEY_PAUSE = 0x92;
	constexpr unsigned char PG_KEY_F1 = 0x11;
	constexpr unsigned char PG_KEY_F2 = 0x12;
	constexpr unsigned char PG_KEY_F3 = 0x13;
	constexpr unsigned char PG_KEY_F4 = 0x14;
	constexpr unsigned char PG_KEY_F5 = 0x15;
	constexpr unsigned char PG_KEY_F6 = 0x16;
	constexpr unsigned char PG_KEY_F7 = 0x17;
	constexpr unsigned char PG_KEY_F8 = 0x18;
	constexpr unsigned char PG_KEY_F9 = 0x19;
	constexpr unsigned char PG_KEY_F10 = 0x1A;
	constexpr unsigned char PG_KEY_F11 = 0x1B;
	constexpr unsigned char PG_KEY_F12 = 0x1C;
	constexpr unsigned char PG_KEY_F13 = 0x1D;
	constexpr unsigned char PG_KEY_F14 = 0x1E;
	constexpr unsigned char PG_KEY_F15 = 0x1F;
	constexpr unsigned char PG_KEY_F16 = 0x20;
	constexpr unsigned char PG_KEY_F17 = 0x21;
	constexpr unsigned char PG_KEY_F18 = 0x22;
	constexpr unsigned char PG_KEY_F19 = 0x23;
	constexpr unsigned char PG_KEY_F20 = 0x24;
	constexpr unsigned char PG_KEY_F21 = 0x25;
	constexpr unsigned char PG_KEY_F22 = 0x26;
	constexpr unsigned char PG_KEY_F23 = 0x27;
	constexpr unsigned char PG_KEY_F24 = 0x28;
	constexpr unsigned char PG_KEY_KP_0 = 0x29;
	constexpr unsigned char PG_KEY_KP_1 = 0x2A;
}
