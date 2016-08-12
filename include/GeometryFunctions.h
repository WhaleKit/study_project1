#ifndef GEOMETRYFUNCTIONS_H_INCLUDED
#define GEOMETRYFUNCTIONS_H_INCLUDED

#include "fpoint_tweaks.h"
#include "Tileset2d.h"
#include <assert.h>

using namespace std;
using namespace sf;

template<typename T>
T minByAbs(T arg1, T arg2 )
{
    return min(arg1, arg2, [](T t1, T t2)->bool{return abs(t1)<abs(t2);} );
}

inline float vectorTan (Vector2f const& vec_arg)
{
    return vec_arg.y/vec_arg.x;
}


//������������ SFML ��������� �������� ������, ��
//��������� ������� ������ ���� � ����������
template <typename T>
Vector2<T> operator *(Vector2<T> const& firts, Vector2<T> const& second)
{
    return Vector2<T>(firts.x*second.x, firts.y*second.y);
}
template <typename T>
Vector2<T> abs (Vector2<T> const& vec_arg)
{
    return Vector2<T>( abs(vec_arg.x), abs(vec_arg.y) );
}


enum class CornerOfRect{LeftUp, LeftDown, RightUp, RightDown};

inline Vector2f getCornerCoords(FloatRect const& rect_arg, CornerOfRect corner_arg)
{
    switch (corner_arg)
    {
    case CornerOfRect::LeftUp:
        return Vector2f( rect_arg.left,                 rect_arg.top                );
    case CornerOfRect::LeftDown:
        return Vector2f( rect_arg.left,                 rect_arg.top+rect_arg.height);
    case CornerOfRect::RightUp:
        return Vector2f( rect_arg.left+rect_arg.width,  rect_arg.top                );
    case CornerOfRect::RightDown:
        return Vector2f( rect_arg.left+rect_arg.width,  rect_arg.top+rect_arg.height);
    default:
        assert(false); //"������, � �������� ��������� 5-� �������"
    }
}
inline CornerOfRect OppositeRectCorner (CornerOfRect arg)
{
    switch(arg)
    {
    case CornerOfRect::LeftUp:
        return CornerOfRect::RightDown;

    case CornerOfRect::LeftDown:
        return CornerOfRect::RightUp;

    case CornerOfRect::RightUp:
        return CornerOfRect::LeftDown;

    case CornerOfRect::RightDown:
        return CornerOfRect::LeftUp;
    default:
        assert(false); //"������, � �������� ��������� 5-� �������"
    }
}
inline IntRect HorizontalFlip(IntRect const& ir_arg)
{
    return IntRect(ir_arg.left+ir_arg.width ,ir_arg.top, -ir_arg.width ,ir_arg.height);
}



//������� ��������, ������ ���� ������ �� ������ �������-��������� �� ����� � ������-�������� ��������� � ����� ��������
//������ ���� - ����� ����, ������ ���� - ����, 0 - �� ���
inline Int8 DotPositionRelativeToVector (Vector2f dotCoords_arg, Vector2f vectorStartCoords_arg, Vector2f vectorItself_arg)
{
    float TanDiff = vectorTan(vectorItself_arg) - vectorTan( dotCoords_arg - vectorStartCoords_arg );

    if ( vectorItself_arg.x < 0 )
        TanDiff*= -1;

    if (TanDiff<0)
        return 1;

    if (TanDiff>0)
        return -1;
    //TanDiff =0
    return 0;
}

//#define LOGGING_ENABLED


//������ ������� ������ �������� � ��������� ����, �� �� ��� ��������
//���������� true, ���� ���� ����������� � ������ �����, false � ��������� ������
bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg);



//<s>���������� true ��� false ���� ���������� ��������� ��������� return � ����� � ��������� ������</s>
//���������� true ���� ���� ����� ����� ��� ������
//(� ������ ������ �� ������ ���������� ��� ������� moveTroughtTileAndCollide)
bool StandingOnTheSolidGround (Tileset2d const& map_arg, FloatRect const& body_arg, const float footingDistance_arg );


//���������, ������������ �� ������� AB � CD
bool LineSegmentsIntersects (Vector2f const A_arg, Vector2f const B_arg, Vector2f const C_arg, Vector2f const D_arg);


#endif // GEOMETRYFUNCTIONS_H_INCLUDED
