#pragma once

#include <string_view>
#include <string>
#include <unordered_map>

#define ALL_KEY_BOARDS \
    KEY_NAME(Unknown)     \
    KEY_NAME(Backspace)   \
    KEY_NAME(Tab)         \
    KEY_NAME(Clear)       \
    KEY_NAME(Return)      \
    KEY_NAME(Pause)       \
    KEY_NAME(Escape)      \
    KEY_NAME(Space)       \
    KEY_NAME(Exclaim)     \
    KEY_NAME(Quotedbl)    \
    KEY_NAME(Hash)        \
    KEY_NAME(Dollar)      \
    KEY_NAME(Ampersand)   \
    KEY_NAME(Quote)       \
    KEY_NAME(Leftparen)   \
    KEY_NAME(Rightparen)  \
    KEY_NAME(Asterisk)    \
    KEY_NAME(Plus)        \
    KEY_NAME(Comma)       \
    KEY_NAME(Minus)       \
    KEY_NAME(Period)      \
    KEY_NAME(Slash)       \
    KEY_NAME(Key0)        \
    KEY_NAME(Key1)        \
    KEY_NAME(Key2)        \
    KEY_NAME(Key3)        \
    KEY_NAME(Key4)        \
    KEY_NAME(Key5)        \
    KEY_NAME(Key6)        \
    KEY_NAME(Key7)        \
    KEY_NAME(Key8)        \
    KEY_NAME(Key9)        \
    KEY_NAME(Colon)       \
    KEY_NAME(Semicolon)   \
    KEY_NAME(Less)        \
    KEY_NAME(Equals)      \
    KEY_NAME(Greater)     \
    KEY_NAME(Question)    \
    KEY_NAME(At)          \
    KEY_NAME(Leftbracket) \
    KEY_NAME(Backslash)   \
    KEY_NAME(Rightbracket)\
    KEY_NAME(Caret)       \
    KEY_NAME(Underscore)  \
    KEY_NAME(Backquote)   \
    KEY_NAME(A)           \
    KEY_NAME(B)           \
    KEY_NAME(C)           \
    KEY_NAME(D)           \
    KEY_NAME(E)           \
    KEY_NAME(F)           \
    KEY_NAME(G)           \
    KEY_NAME(H)           \
    KEY_NAME(I)           \
    KEY_NAME(J)           \
    KEY_NAME(K)           \
    KEY_NAME(L)           \
    KEY_NAME(M)           \
    KEY_NAME(N)           \
    KEY_NAME(O)           \
    KEY_NAME(P)           \
    KEY_NAME(Q)           \
    KEY_NAME(R)           \
    KEY_NAME(S)           \
    KEY_NAME(T)           \
    KEY_NAME(U)           \
    KEY_NAME(V)           \
    KEY_NAME(W)           \
    KEY_NAME(X)           \
    KEY_NAME(Y)           \
    KEY_NAME(Z)           \
    KEY_NAME(Delete)      \
    KEY_NAME(KP0)         \
    KEY_NAME(KP1)         \
    KEY_NAME(KP2)         \
    KEY_NAME(KP3)         \
    KEY_NAME(KP4)         \
    KEY_NAME(KP5)         \
    KEY_NAME(KP6)         \
    KEY_NAME(KP7)         \
    KEY_NAME(KP8)         \
    KEY_NAME(KP9)         \
    KEY_NAME(KPPeriod)    \
    KEY_NAME(KPDivide)    \
    KEY_NAME(KPMultiply)  \
    KEY_NAME(KPMinus)     \
    KEY_NAME(KPPlus)      \
    KEY_NAME(KPEnter)     \
    KEY_NAME(KPEquals)    \
    KEY_NAME(Up)          \
    KEY_NAME(Down)        \
    KEY_NAME(Right)       \
    KEY_NAME(Left)        \
    KEY_NAME(Insert)      \
    KEY_NAME(Home)        \
    KEY_NAME(End)         \
    KEY_NAME(Pageup)      \
    KEY_NAME(Pagedown)    \
    KEY_NAME(F1)          \
    KEY_NAME(F2)          \
    KEY_NAME(F3)          \
    KEY_NAME(F4)          \
    KEY_NAME(F5)          \
    KEY_NAME(F6)          \
    KEY_NAME(F7)          \
    KEY_NAME(F8)          \
    KEY_NAME(F9)          \
    KEY_NAME(F10)         \
    KEY_NAME(F11)         \
    KEY_NAME(F12)         \
    KEY_NAME(F13)         \
    KEY_NAME(F14)         \
    KEY_NAME(F15)         \
    KEY_NAME(Numlock)     \
    KEY_NAME(Capslock)    \
    KEY_NAME(Scrolllock)  \
    KEY_NAME(Rshift)      \
    KEY_NAME(Lshift)      \
    KEY_NAME(Rctrl)       \
    KEY_NAME(Lctrl)       \
    KEY_NAME(Ralt)        \
    KEY_NAME(Lalt)        \
    KEY_NAME(Mode)        \
    KEY_NAME(Help)        \
    KEY_NAME(Print)       \
    KEY_NAME(Sysreq)      \
    KEY_NAME(Menu)        \
    KEY_NAME(Power)       \

namespace nes
{
#ifdef KEY_NAME
#undef KEY_NAME
#endif
#define KEY_NAME(key) key,

    // 所有键盘按键的枚举
    enum class KeyCode
    {
        ALL_KEY_BOARDS
    };

#undef KEY_NAME

#define KEY_NAME(key) #key,

    // 所有键盘枚举的字符串
    inline constexpr std::string_view KeyName[] = 
    {
        ALL_KEY_BOARDS
    };

#undef KEY_NAME

#define KEY_NAME(key) {#key, KeyCode::key},

    // 键盘字符串与枚举的映射
    inline const std::unordered_map<std::string, KeyCode> KeyMap = 
    {
        ALL_KEY_BOARDS
    };

#undef KEY_NAME
#undef ALL_KEY_BOARDS

}
