#ifndef FPOINT_TWEAKS_H_INCLUDED
#define FPOINT_TWEAKS_H_INCLUDED

#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

//вы стоите на блоке пола. Очевидно, ваши ноги не относятся к тайлу, на котором вы стоите, арифметика чисел с плавающей запятой
//считает иначе, так что вы спотыкаетесь на ровном месте или проваливаетесь.
//Отныне вам нужно притворятся висящим в воздухе где-то в 1/100000 вашего размера от пола.
//а еще вы подопрете стену справа от вас, то окажется, что ваш бок, которым вы оперлись уже внутри нее

constexpr float epsiFraction = numeric_limits<float>::epsilon()*8;

//из этого квадрата можно получить 4 вектора epsi-длины напр. в разные стороны при помощи ф-ии getCornerOfSquare
const FloatRect epsiQuad(-epsiFraction, -epsiFraction, 2*epsiFraction, 2*epsiFraction);

inline float LittleLessThan (float const arg, int mul_arg = 1)
{
    return arg - abs(arg)*epsiFraction*mul_arg;
}
inline float LittleMoreThan (float const arg, int mul_arg = 1)
{
    return arg + abs(arg)*epsiFraction*mul_arg;
}

#endif // FPOINT_TWEAKS_H_INCLUDED
