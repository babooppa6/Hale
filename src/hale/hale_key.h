#ifndef HALE_KEY_H
#define HALE_KEY_H

namespace hale {

enum KeyModifiers
{
    KeyM_None        = 0,

    KeyM_Shift       = 0x01,
    KeyM_Alt         = 0x02,
    KeyM_AltGr       = 0x04,
    KeyM_Ctrl        = 0x08,
    KeyM_WinLeft     = 0x10,
    KeyM_WinRight    = 0x20
};

struct Key
{
    u8 modifiers;
    union {
        ch32 codepoint;
        u8 key;
    };
};

enum KeyEventType
{
    KeyT_KeyDown = 0,
    KeyT_KeyUp = 1,
    KeyT_Text = 2
};

struct KeyEvent
{
    KeyEventType type;
    Key key;
};

}

#endif // HALE_KEY_H

