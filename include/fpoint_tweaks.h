#ifndef FPOINT_TWEAKS_H_INCLUDED
#define FPOINT_TWEAKS_H_INCLUDED

#include <SFML/Graphics.hpp>

using namespace std;
using namespace sf;

//�� ������ �� ����� ����. ��������, ���� ���� �� ��������� � �����, �� ������� �� ������, ���������� ����� � ��������� �������
//������� �����, ��� ��� �� ������������ �� ������ ����� ��� ��������������.
//������ ��� ����� ����������� ������� � ������� ���-�� � 1/100000 ������ ������� �� ����.
//� ��� �� ��������� ����� ������ �� ���, �� ��������, ��� ��� ���, ������� �� �������� ��� ������ ���

constexpr float epsiFraction = numeric_limits<float>::epsilon()*8;

//�� ����� �������� ����� �������� 4 ������� epsi-����� ����. � ������ ������� ��� ������ �-�� getCornerOfSquare
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
