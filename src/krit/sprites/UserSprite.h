#ifndef KRIT_SPRITES_USERSPRITE
#define KRIT_SPRITES_USERSPRITE

namespace krit {

template <typename T, typename U> struct UserSprite: public T {
    U userData;

    template <typename ...Args> UserSprite<T, U>(Args&&... args): T(args...) {}
};

}

#endif
