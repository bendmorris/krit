declare enum KeyMod {
    None,
    Lshift,
    Rshift,
    Lctrl,
    Rctrl,
    Lalt,
    Ralt,
    Lgui,
    Rgui,
    Num,
    Caps,
    Mode,
    Scroll,

    Ctrl,
    Shift,
    Alt,
    Gui,
}

declare enum KeyCode {
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Num0,
    Return,
    Escape,
    Backspace,
    Tab,
    Space,
    Minus,
    Equals,
    LeftBracket,
    RightBracket,
    Backslash,
    NonUSHash,
    Semicolon,
    Apostrophe,
    Grave,
    Comma,
    Period,
    Slash,
    CapsLock,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    PrintScreen,
    ScrollLock,
    Pause,
    Insert,
    Home,
    PageUp,
    Delete,
    End,
    PageDown,
    Right,
    Left,
    Down,
    Up,
    NumLockClear,
    KPDivide,
    KPMultiply,
    KPMinus,
    KPPlus,
    KPEnter,
    KP1,
    KP2,
    KP3,
    KP4,
    KP5,
    KP6,
    KP7,
    KP8,
    KP9,
    KP0,
    KPPeriod,
    NonUSBackslash,
    Application,
    Power,
    KPEquals,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    Execute,
    Help,
    Menu,
    Select,
    Stop,
    Again,
    Undo,
    Cut,
    Copy,
    Paste,
    Find,
    Mute,
    VolumeUp,
    VolumeDown,
    KPComma,
    KPEqualsAS400,
    International1,
    International2,
    International3,
    International4,
    International5,
    International6,
    International7,
    International8,
    International9,
    Lang1,
    Lang2,
    Lang3,
    Lang4,
    Lang5,
    Lang6,
    Lang7,
    Lang8,
    Lang9,
    AltErase,
    SysReq,
    Cancel,
    Clear,
    Prior,
    Return2,
    Separator,
    Out,
    Oper,
    ClearAgain,
    CrSel,
    ExSel,
    KP00,
    KP000,
    ThousandsSeparator,
    DecimalSeparator,
    CurrencyUnit,
    CurrencySubunit,
    LeftParen,
    RightParen,
    LeftBrace,
    RightBrace,
    KPTab,
    KPBackspace,
    KPA,
    KPB,
    KPC,
    KPD,
    KPE,
    KPF,
    KPXOR,
    KPPower,
    KPPercent,
    KPLess,
    KPGreater,
    KPAmpersand,
    KPDblAmpersand,
    KPVerticalBar,
    KPDblVerticalBar,
    KPColon,
    KPHash,
    KPSpace,
    KPAt,
    KPExclam,
    KPMemStore,
    KPMemRecall,
    KPMemClear,
    KPMemAdd,
    KPMemSubtract,
    KPMemMultiply,
    KPMemDivide,
    KPPlusMinus,
    KPClear,
    KPClearEntry,
    KPBinary,
    KPOctal,
    KPDecimal,
    KPHexadecimal,
    LCtrl,
    LShift,
    LAlt,
    LGUI,
    RCtrl,
    RShift,
    RAlt,
    RGUI,
    Mode,
    AudioNext,
    AudioPrev,
    AudioStop,
    AudioPlay,
    AudioMute,
    MediaSelect,
    WWW,
    Mail,
    Calculator,
    Computer,
    ACSearch,
    ACHome,
    ACBack,
    ACForward,
    ACStop,
    ACRefresh,
    ACBookmarks,
    BrightnessDown,
    BrightnessUp,
    DisplaySwitch,
    KBDIllumToggle,
    KBDIllumDown,
    KBDIllumUp,
    Eject,
    Sleep,
    App1,
    App2,
}

interface KeyEvent {
    code: KeyCode;
    mod: KeyMod;
    down: boolean;
    repeat: boolean;
}

interface KeyContext {
    readonly events: KeyEvent[];

    define(key: KeyCode, action: number): void;
    undefine(key: KeyCode): void;
    keyName(key: KeyCode): string;

    startTextInput(): void;
    stopTextInput(): void;
    textInput(): string;
}
