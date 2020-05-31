#ifndef KRIT_INPUT_KEY
#define KRIT_INPUT_KEY

#include "krit/UpdateContext.h"
#include <cstring>
#include <unordered_map>
#include <utility>
#include <vector>
// FIXME: leaky abstraction
#include <SDL.h>

namespace krit {

class InputContext;

enum Key {
    KeyA = SDL_SCANCODE_A,
    KeyB = SDL_SCANCODE_B,
    KeyC = SDL_SCANCODE_C,
    KeyD = SDL_SCANCODE_D,
    KeyE = SDL_SCANCODE_E,
    KeyF = SDL_SCANCODE_F,
    KeyG = SDL_SCANCODE_G,
    KeyH = SDL_SCANCODE_H,
    KeyI = SDL_SCANCODE_I,
    KeyJ = SDL_SCANCODE_J,
    KeyK = SDL_SCANCODE_K,
    KeyL = SDL_SCANCODE_L,
    KeyM = SDL_SCANCODE_M,
    KeyN = SDL_SCANCODE_N,
    KeyO = SDL_SCANCODE_O,
    KeyP = SDL_SCANCODE_P,
    KeyQ = SDL_SCANCODE_Q,
    KeyR = SDL_SCANCODE_R,
    KeyS = SDL_SCANCODE_S,
    KeyT = SDL_SCANCODE_T,
    KeyU = SDL_SCANCODE_U,
    KeyV = SDL_SCANCODE_V,
    KeyW = SDL_SCANCODE_W,
    KeyX = SDL_SCANCODE_X,
    KeyY = SDL_SCANCODE_Y,
    KeyZ = SDL_SCANCODE_Z,
    Key1 = SDL_SCANCODE_1,
    Key2 = SDL_SCANCODE_2,
    Key3 = SDL_SCANCODE_3,
    Key4 = SDL_SCANCODE_4,
    Key5 = SDL_SCANCODE_5,
    Key6 = SDL_SCANCODE_6,
    Key7 = SDL_SCANCODE_7,
    Key8 = SDL_SCANCODE_8,
    Key9 = SDL_SCANCODE_9,
    Key0 = SDL_SCANCODE_0,
    KeyReturn = SDL_SCANCODE_RETURN,
    KeyEscape = SDL_SCANCODE_ESCAPE,
    KeyBackspace = SDL_SCANCODE_BACKSPACE,
    KeyTab = SDL_SCANCODE_TAB,
    KeySpace = SDL_SCANCODE_SPACE,
    KeyMinus = SDL_SCANCODE_MINUS,
    KeyEquals = SDL_SCANCODE_EQUALS,
    KeyLeftBracket = SDL_SCANCODE_LEFTBRACKET,
    KeyRightBracket = SDL_SCANCODE_RIGHTBRACKET,
    KeyBackslash = SDL_SCANCODE_BACKSLASH,
    KeyNonUSHash = SDL_SCANCODE_NONUSHASH,
    KeySemicolon = SDL_SCANCODE_SEMICOLON,
    KeyApostrophe = SDL_SCANCODE_APOSTROPHE,
    KeyGrave = SDL_SCANCODE_GRAVE,
    KeyComma = SDL_SCANCODE_COMMA,
    KeyPeriod = SDL_SCANCODE_PERIOD,
    KeySlash = SDL_SCANCODE_SLASH,
    KeyCapsLock = SDL_SCANCODE_CAPSLOCK,
    KeyF1 = SDL_SCANCODE_F1,
    KeyF2 = SDL_SCANCODE_F2,
    KeyF3 = SDL_SCANCODE_F3,
    KeyF4 = SDL_SCANCODE_F4,
    KeyF5 = SDL_SCANCODE_F5,
    KeyF6 = SDL_SCANCODE_F6,
    KeyF7 = SDL_SCANCODE_F7,
    KeyF8 = SDL_SCANCODE_F8,
    KeyF9 = SDL_SCANCODE_F9,
    KeyF10 = SDL_SCANCODE_F10,
    KeyF11 = SDL_SCANCODE_F11,
    KeyF12 = SDL_SCANCODE_F12,
    KeyPrintScreen = SDL_SCANCODE_PRINTSCREEN,
    KeyScrollLock = SDL_SCANCODE_SCROLLLOCK,
    KeyPause = SDL_SCANCODE_PAUSE,
    KeyInsert = SDL_SCANCODE_INSERT,
    KeyHome = SDL_SCANCODE_HOME,
    KeyPageUp = SDL_SCANCODE_PAGEUP,
    KeyDelete = SDL_SCANCODE_DELETE,
    KeyEnd = SDL_SCANCODE_END,
    KeyPageDown = SDL_SCANCODE_PAGEDOWN,
    KeyRight = SDL_SCANCODE_RIGHT,
    KeyLeft = SDL_SCANCODE_LEFT,
    KeyDown = SDL_SCANCODE_DOWN,
    KeyUp = SDL_SCANCODE_UP,
    KeyNumLockClear = SDL_SCANCODE_NUMLOCKCLEAR,
    KeyKPDivide = SDL_SCANCODE_KP_DIVIDE,
    KeyKPMultiply = SDL_SCANCODE_KP_MULTIPLY,
    KeyKPMinus = SDL_SCANCODE_KP_MINUS,
    KeyKPPlus = SDL_SCANCODE_KP_PLUS,
    KeyKPEnter = SDL_SCANCODE_KP_ENTER,
    KeyKP1 = SDL_SCANCODE_KP_1,
    KeyKP2 = SDL_SCANCODE_KP_2,
    KeyKP3 = SDL_SCANCODE_KP_3,
    KeyKP4 = SDL_SCANCODE_KP_4,
    KeyKP5 = SDL_SCANCODE_KP_5,
    KeyKP6 = SDL_SCANCODE_KP_6,
    KeyKP7 = SDL_SCANCODE_KP_7,
    KeyKP8 = SDL_SCANCODE_KP_8,
    KeyKP9 = SDL_SCANCODE_KP_9,
    KeyKP0 = SDL_SCANCODE_KP_0,
    KeyKPPeriod = SDL_SCANCODE_KP_PERIOD,
    KeyNonUSBackslash = SDL_SCANCODE_NONUSBACKSLASH,
    KeyApplication = SDL_SCANCODE_APPLICATION,
    KeyPower = SDL_SCANCODE_POWER,
    KeyKPEquals = SDL_SCANCODE_KP_EQUALS,
    KeyF13 = SDL_SCANCODE_F13,
    KeyF14 = SDL_SCANCODE_F14,
    KeyF15 = SDL_SCANCODE_F15,
    KeyF16 = SDL_SCANCODE_F16,
    KeyF17 = SDL_SCANCODE_F17,
    KeyF18 = SDL_SCANCODE_F18,
    KeyF19 = SDL_SCANCODE_F19,
    KeyF20 = SDL_SCANCODE_F20,
    KeyF21 = SDL_SCANCODE_F21,
    KeyF22 = SDL_SCANCODE_F22,
    KeyF23 = SDL_SCANCODE_F23,
    KeyF24 = SDL_SCANCODE_F24,
    KeyExecute = SDL_SCANCODE_EXECUTE,
    KeyHelp = SDL_SCANCODE_HELP,
    KeyMenu = SDL_SCANCODE_MENU,
    KeySelect = SDL_SCANCODE_SELECT,
    KeyStop = SDL_SCANCODE_STOP,
    KeyAgain = SDL_SCANCODE_AGAIN,
    KeyUndo = SDL_SCANCODE_UNDO,
    KeyCut = SDL_SCANCODE_CUT,
    KeyCopy = SDL_SCANCODE_COPY,
    KeyPaste = SDL_SCANCODE_PASTE,
    KeyFind = SDL_SCANCODE_FIND,
    KeyMute = SDL_SCANCODE_MUTE,
    KeyVolumeUp = SDL_SCANCODE_VOLUMEUP,
    KeyVolumeDown = SDL_SCANCODE_VOLUMEDOWN,
    KeyKPComma = SDL_SCANCODE_KP_COMMA,
    KeyKPEqualsAS400 = SDL_SCANCODE_KP_EQUALSAS400,
    KeyInternational1 = SDL_SCANCODE_INTERNATIONAL1,
    KeyInternational2 = SDL_SCANCODE_INTERNATIONAL2,
    KeyInternational3 = SDL_SCANCODE_INTERNATIONAL3,
    KeyInternational4 = SDL_SCANCODE_INTERNATIONAL4,
    KeyInternational5 = SDL_SCANCODE_INTERNATIONAL5,
    KeyInternational6 = SDL_SCANCODE_INTERNATIONAL6,
    KeyInternational7 = SDL_SCANCODE_INTERNATIONAL7,
    KeyInternational8 = SDL_SCANCODE_INTERNATIONAL8,
    KeyInternational9 = SDL_SCANCODE_INTERNATIONAL9,
    KeyLang1 = SDL_SCANCODE_LANG1,
    KeyLang2 = SDL_SCANCODE_LANG2,
    KeyLang3 = SDL_SCANCODE_LANG3,
    KeyLang4 = SDL_SCANCODE_LANG4,
    KeyLang5 = SDL_SCANCODE_LANG5,
    KeyLang6 = SDL_SCANCODE_LANG6,
    KeyLang7 = SDL_SCANCODE_LANG7,
    KeyLang8 = SDL_SCANCODE_LANG8,
    KeyLang9 = SDL_SCANCODE_LANG9,
    KeyAltErase = SDL_SCANCODE_ALTERASE,
    KeySysReq = SDL_SCANCODE_SYSREQ,
    KeyCancel = SDL_SCANCODE_CANCEL,
    KeyClear = SDL_SCANCODE_CLEAR,
    KeyPrior = SDL_SCANCODE_PRIOR,
    KeyReturn2 = SDL_SCANCODE_RETURN2,
    KeySeparator = SDL_SCANCODE_SEPARATOR,
    KeyOut = SDL_SCANCODE_OUT,
    KeyOper = SDL_SCANCODE_OPER,
    KeyClearAgain = SDL_SCANCODE_CLEARAGAIN,
    KeyCrSel = SDL_SCANCODE_CRSEL,
    KeyExSel = SDL_SCANCODE_EXSEL,
    KeyKP00 = SDL_SCANCODE_KP_00,
    KeyKP000 = SDL_SCANCODE_KP_000,
    KeyThousandsSeparator = SDL_SCANCODE_THOUSANDSSEPARATOR,
    KeyDecimalSeparator = SDL_SCANCODE_DECIMALSEPARATOR,
    KeyCurrencyUnit = SDL_SCANCODE_CURRENCYUNIT,
    KeyCurrencySubunit = SDL_SCANCODE_CURRENCYSUBUNIT,
    KeyLeftParen = SDL_SCANCODE_KP_LEFTPAREN,
    KeyRightParen = SDL_SCANCODE_KP_RIGHTPAREN,
    KeyLeftBrace = SDL_SCANCODE_KP_LEFTBRACE,
    KeyRightBrace = SDL_SCANCODE_KP_RIGHTBRACE,
    KeyKPTab = SDL_SCANCODE_KP_TAB,
    KeyKPBackspace = SDL_SCANCODE_KP_BACKSPACE,
    KeyKPA = SDL_SCANCODE_KP_A,
    KeyKPB = SDL_SCANCODE_KP_B,
    KeyKPC = SDL_SCANCODE_KP_C,
    KeyKPD = SDL_SCANCODE_KP_D,
    KeyKPE = SDL_SCANCODE_KP_E,
    KeyKPF = SDL_SCANCODE_KP_F,
    KeyKPXOR = SDL_SCANCODE_KP_XOR,
    KeyKPPower = SDL_SCANCODE_KP_POWER,
    KeyKPPercent = SDL_SCANCODE_KP_PERCENT,
    KeyKPLess = SDL_SCANCODE_KP_LESS,
    KeyKPGreater = SDL_SCANCODE_KP_GREATER,
    KeyKPAmpersand = SDL_SCANCODE_KP_AMPERSAND,
    KeyKPDblAmpersand = SDL_SCANCODE_KP_DBLAMPERSAND,
    KeyKPVerticalBar = SDL_SCANCODE_KP_VERTICALBAR,
    KeyKPDblVerticalBar = SDL_SCANCODE_KP_DBLVERTICALBAR,
    KeyKPColon = SDL_SCANCODE_KP_COLON,
    KeyKPHash = SDL_SCANCODE_KP_HASH,
    KeyKPSpace = SDL_SCANCODE_KP_SPACE,
    KeyKPAt = SDL_SCANCODE_KP_AT,
    KeyKPExclam = SDL_SCANCODE_KP_EXCLAM,
    KeyKPMemStore = SDL_SCANCODE_KP_MEMSTORE,
    KeyKPMemRecall = SDL_SCANCODE_KP_MEMRECALL,
    KeyKPMemClear = SDL_SCANCODE_KP_MEMCLEAR,
    KeyKPMemAdd = SDL_SCANCODE_KP_MEMADD,
    KeyKPMemSubtract = SDL_SCANCODE_KP_MEMSUBTRACT,
    KeyKPMemMultiply = SDL_SCANCODE_KP_MEMMULTIPLY,
    KeyKPMemDivide = SDL_SCANCODE_KP_MEMDIVIDE,
    KeyKPPlusMinus = SDL_SCANCODE_KP_PLUSMINUS,
    KeyKPClear = SDL_SCANCODE_KP_CLEAR,
    KeyKPClearEntry = SDL_SCANCODE_KP_CLEARENTRY,
    KeyKPBinary = SDL_SCANCODE_KP_BINARY,
    KeyKPOctal = SDL_SCANCODE_KP_OCTAL,
    KeyKPDecimal = SDL_SCANCODE_KP_DECIMAL,
    KeyKPHexadecimal = SDL_SCANCODE_KP_HEXADECIMAL,
    KeyLCtrl = SDL_SCANCODE_LCTRL,
    KeyLShift = SDL_SCANCODE_LSHIFT,
    KeyLAlt = SDL_SCANCODE_LALT,
    KeyLGUI = SDL_SCANCODE_LGUI,
    KeyRCtrl = SDL_SCANCODE_RCTRL,
    KeyRShift = SDL_SCANCODE_RSHIFT,
    KeyRAlt = SDL_SCANCODE_RALT,
    KeyRGUI = SDL_SCANCODE_RGUI,
    KeyMode = SDL_SCANCODE_MODE,
    KeyAudioNext = SDL_SCANCODE_AUDIONEXT,
    KeyAudioPrev = SDL_SCANCODE_AUDIOPREV,
    KeyAudioStop = SDL_SCANCODE_AUDIOSTOP,
    KeyAudioPlay = SDL_SCANCODE_AUDIOPLAY,
    KeyAudioMute = SDL_SCANCODE_AUDIOMUTE,
    KeyMediaSelect = SDL_SCANCODE_MEDIASELECT,
    KeyWWW = SDL_SCANCODE_WWW,
    KeyMail = SDL_SCANCODE_MAIL,
    KeyCalculator = SDL_SCANCODE_CALCULATOR,
    KeyComputer = SDL_SCANCODE_COMPUTER,
    KeyACSearch = SDL_SCANCODE_AC_SEARCH,
    KeyACHome = SDL_SCANCODE_AC_HOME,
    KeyACBack = SDL_SCANCODE_AC_BACK,
    KeyACForward = SDL_SCANCODE_AC_FORWARD,
    KeyACStop = SDL_SCANCODE_AC_STOP,
    KeyACRefresh = SDL_SCANCODE_AC_REFRESH,
    KeyACBookmarks = SDL_SCANCODE_AC_BOOKMARKS,
    KeyBrightnessDown = SDL_SCANCODE_BRIGHTNESSDOWN,
    KeyBrightnessUp = SDL_SCANCODE_BRIGHTNESSUP,
    KeyDisplaySwitch = SDL_SCANCODE_DISPLAYSWITCH,
    KeyKBDIllumToggle = SDL_SCANCODE_KBDILLUMTOGGLE,
    KeyKBDIllumDown = SDL_SCANCODE_KBDILLUMDOWN,
    KeyKBDIllumUp = SDL_SCANCODE_KBDILLUMUP,
    KeyEject = SDL_SCANCODE_EJECT,
    KeySleep = SDL_SCANCODE_SLEEP,
    KeyApp1 = SDL_SCANCODE_APP1,
    KeyApp2 = SDL_SCANCODE_APP2,
};

struct KeyManager {
    std::unordered_map<int, Action> keyMappings;
    std::unordered_map<int, bool> active;

    KeyManager(std::vector<InputEvent> *events): events(events) {}

    void update() {
        for (auto &it : this->active) {
            this->keyEvent(static_cast<Key>(it.first), InputActive);
        }
    }

    void define(Key keyCode, Action action) {
        this->keyMappings.insert(std::make_pair(keyCode, action));
    }

    void undefine(Key keyCode) {
        this->keyMappings.erase(keyCode);
    }

    void registerDown(Key keyCode) {
        if (this->keyMappings.count(keyCode) && !this->active.count(keyCode)) {
            this->active.insert(std::make_pair(keyCode, true));
            this->keyEvent(keyCode, InputStart);
        }
    }

    void registerUp(Key keyCode) {
        if (this->keyMappings.count(keyCode) && this->active.count(keyCode)) {
            this->active.erase(keyCode);
            this->keyEvent(keyCode, InputFinish);
        }
    }

    private:
        std::vector<InputEvent> *events;

        void keyEvent(Key key, InputEventType eventType) {
            auto found = this->keyMappings.find(static_cast<int>(key));
            if (found != this->keyMappings.end()) {
                this->events->emplace_back(eventType, found->second, LevelData(eventType != InputFinish));
            }
        }
};

}

#endif
