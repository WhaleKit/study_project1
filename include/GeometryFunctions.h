#ifndef GEOMETRYFUNCTIONS_H_INCLUDED
#define GEOMETRYFUNCTIONS_H_INCLUDED

#include "fpoint_tweaks.h"
#include "Tileset2d.h"

using namespace std;
using namespace sf;

template<typename T>
T minByAbs(T arg1, T arg2 )
{
    return min(arg1, arg2, [](T t1, T t2)->bool{return abs(t1)<abs(t2);} );
}

float vectorTan (Vector2f const& vec_arg)
{
    return vec_arg.y/vec_arg.x;
}


//разработчики SFML проделали отличную работу, но
//некоторые моменты вводят меня в недоумение
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
Vector2f getCornerCoords(FloatRect const& rect_arg, CornerOfRect corner_arg)
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
        assert(false); //"похоже, у квадрата появилась 5-я сторона"
    }
}
CornerOfRect OppositeRectCorner (CornerOfRect arg)
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
        assert(false); //"похоже, у квадрата появилась 5-я сторона"
    }
}
inline IntRect HorizontalFlip(IntRect const& ir_arg)
{
    return IntRect(ir_arg.left+ir_arg.width ,ir_arg.top, -ir_arg.width ,ir_arg.height);
}



//функция работает, только если вектор от начала вектора-аргумента до точки и вектор-аргумент находятся в одной четверти
//меньше нуля - точка ниже, больше нуля - выше, 0 - на нем
Int8 DotPositionRelativeToVector (Vector2f dotCoords_arg, Vector2f vectorStartCoords_arg, Vector2f vectorItself_arg)
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


//данная функция меняет скорость и положение тела, но не его габариты
//возвращает true, если тело столкнулось с опорой снизу, false в противном случае
bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg)
{

    const Vector2f origMoveVector(bodySpeed_arg.x * time_arg.asMicroseconds(), bodySpeed_arg.y * time_arg.asMicroseconds() );
        Vector2f moveVector(bodySpeed_arg.x * time_arg.asMicroseconds(), bodySpeed_arg.y * time_arg.asMicroseconds());

        //использовалось для отслеживания багов с провалом сквозь землю и прохождением сквозь стены
        //зная аргументты я мог позже воспроизвести его и посмотреть выполнение пошагово
        //дело в том, что отладка во время самой игры затруднена из-за того, что часы не останавливаются, да прокликивать
        //f8 300 раз, чтобы добраться до того места, где баг воспроизводится... Проще поймать состояние логгером
        //и потом воспроизводить в тесте
#ifdef LOGGING_ENABLED
{
    static vector<tuple<FloatRect, Vector2f, Time>> log(30);

    static unsigned num=0;

    log[num] = make_tuple(body_arg, bodySpeed_arg, time_arg);
    num = (num+1)%30;

    static int left=10;

    if (body_arg.left>2220)
    {
        --left;
        if (left==0)
        {
            cout << "\n\n\n\n\nNumber is " << num <<endl;

            tuple<FloatRect, Vector2f, Time> & current = log[num];
            cout << -20<< ")top: "<< get<0>(current).top << " left: " << get<0>(current).left;
            cout << "\n   speed: (" << get<1>(current).x <<", "
                                << get<1>(current).y << ")";
            cout << "\n   time = " << get<2>(current).asMicroseconds();
            cout << "\n   move = (" << get<1>(current).x*get<2>(current).asMicroseconds()
                                    <<", " << get<1>(current).y*get<2>(current).asMicroseconds() << ")" << endl;

            for (unsigned i = num+1; i!=num; i = (i+1)%30)
            {
                tuple<FloatRect, Vector2f, Time> & current = log[i];
                cout << int( ( int(i) - int(num) )>0? ( int(i)- int(num)%30) : (30+(i- int(num)%30))  ) -20
                        << ")top: "<< get<0>(current).top << " left: " << get<0>(current).left;
                cout << "\n   speed: (" << get<1>(current).x <<", "
                                << get<1>(current).y << ")";
                cout << "\n   time = " << get<2>(current).asMicroseconds();
            cout << "\n   move = (" << get<1>(current).x*get<2>(current).asMicroseconds()
                                    <<", " << get<1>(current).y*get<2>(current).asMicroseconds() << ")" << endl;
            }

        }
    }
    else
        left = 10;
}
#endif // LOGGING_ENABLED


    if ( (moveVector.x == 0)  && (moveVector.y==0) )
    {
        return false;
    }

    static auto DotCrossesTheHorBoundary = [&map_arg] ( float xcoord, float xShift  ) -> bool
        {
            return map_arg.XCoordToIndex(xcoord) != map_arg.XCoordToIndex(xcoord+xShift);
        };

    static auto DotCrossesTheVerBoundary = [&map_arg] ( float ycoord, float yShift ) ->bool
        {
            return map_arg.YCoordToIndex(ycoord) != map_arg.YCoordToIndex(ycoord+yShift);
        };

    bool crossedVerticalBoundary, crossedHorizontalBoundary;
    {
        float xcoord;
        if (moveVector.x>0)
        {
           xcoord = (body_arg.left+body_arg.width - abs(body_arg.left+body_arg.width)*epsiFraction*2 );
           crossedHorizontalBoundary = DotCrossesTheHorBoundary(xcoord, moveVector.x*(1+epsiFraction) + abs(body_arg.left+body_arg.width)*epsiFraction );
        }
        else if(moveVector.x!=0)
        {
            xcoord = (body_arg.left + abs(body_arg.left)*epsiFraction*2 );
            crossedHorizontalBoundary = DotCrossesTheHorBoundary(xcoord, moveVector.x*(1+epsiFraction) - abs(body_arg.left)*epsiFraction);
        }
        else
            crossedHorizontalBoundary = false;

        float ycoord;
        if (moveVector.y >0)
        {
            ycoord = (body_arg.top + body_arg.height) - abs(body_arg.top + body_arg.height)*epsiFraction*2;
            crossedVerticalBoundary = DotCrossesTheVerBoundary(ycoord, moveVector.y*(1+epsiFraction) + abs(body_arg.top + body_arg.height)*epsiFraction*2);
        }
        else if (moveVector.y!=0)
        {
            ycoord = body_arg.top + abs(body_arg.top)*epsiFraction*2;
            crossedVerticalBoundary = DotCrossesTheVerBoundary(ycoord, moveVector.y*(1+epsiFraction) - abs(body_arg.top)*epsiFraction*2);
        }
        else
            crossedVerticalBoundary = false;
    }
    if (!(crossedHorizontalBoundary || crossedVerticalBoundary))
    {
        body_arg.top += moveVector.y;
        body_arg.left+= moveVector.x;
        return false;
    }

    if (moveVector.x > map_arg.tileSize || moveVector.y > map_arg.tileSize)
    {
        //если скорость слишком большая/тайлы маленькие - разбиваем движение на части и производим по частям
        //нам же не нужно, чтобы тело на скорости пролетело сквозь тайлы?
        bool grounded = false;

        int stageCount = ceil( max( abs(moveVector.x) , abs(moveVector.y) )/ static_cast<float>(map_arg.tileSize));
        for (int i = 0; i<stageCount; i++)
        {
            //вложенность рекурси - не обльше 1, т.к. я разбил движение на гарантированно достаточно малые куски
            //другое дело, что этих кусков может быть много
            if (MoveTroughtTilesAndCollide(map_arg, body_arg, bodySpeed_arg, time_arg/static_cast<Int64>(stageCount) ))
                grounded=true;
            if (bodySpeed_arg==Vector2f(0,0))
                break;
        }
        return grounded;
    }

    bool collidedVertically = false, collidedHorizontally = false;

    //не очень хорошо, с точки зрения читаемости, что функция, которая все завершает, находится в начале
    static auto finish = [&]()mutable-> bool
        {
            bool grounded = false;
            if (collidedHorizontally)
            {
                if(moveVector.x>0)
                {
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex( LittleLessThan(body_arg.left+body_arg.width,2) ) +1 );
                    moveVector.x = wallX - LittleMoreThan(body_arg.left+body_arg.width) ;

                    //moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
                }
                else
                {
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex( LittleMoreThan(body_arg.left) ) );
                    moveVector.x = wallX - LittleLessThan(body_arg.left);

                    //moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
                }
                bodySpeed_arg.x=0;
            }
            if (collidedVertically)
            {
                if (moveVector.y>0)
                {
                    grounded = true;
                    float floorY =  map_arg.YIndexToCoord( map_arg.YCoordToIndex( LittleLessThan(body_arg.top+body_arg.height,2)) + 1 );
                    moveVector.y = (floorY) - LittleMoreThan(body_arg.top+body_arg.height);

                    //moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
                }
                else
                {
                    float ceilingY = map_arg.YCoordToIndex( map_arg.YIndexToCoord(LittleMoreThan(body_arg.top,2) ));
                    moveVector.y = ceilingY - LittleMoreThan(body_arg.top);

                    //moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
                }
                bodySpeed_arg.y=0;
            }
            body_arg.left += moveVector.x;
            body_arg.top  += moveVector.y;
            return grounded;
        };

    //эти макросы будут вызываться, когда я обнаружу столкновение
    //я не смог сделать все через лямбды, т.к. тут еще и возврат из функции при опр. условиях

#define FINISH()  return finish();

#define COLLIDE_VERT() \
{                           \
    collidedVertically=true;\
    crossedVerticalBoundary = false;\
    if (crossedHorizontalBoundary&&crossedVerticalBoundary)\
    {\
        FINISH()\
    }\
}

#define COLLIDE_HOR()  \
{       \
    collidedHorizontally=true;\
    crossedHorizontalBoundary = false;\
    if (crossedHorizontalBoundary&&crossedVerticalBoundary)\
    {\
        FINISH()\
    }\
}

    // тут первая логическая часть
    //обнаруживаем столкновения об угол углом коллизии
    if(  (moveVector.x!=0) && (moveVector.y!=0) )
    {
        Vector2f upperMovingDot, lowerMovingDot,
                   upperEpsiVec, lowerEpsiVec;

        bool movingRight = moveVector.x>0
            ,movingDown  = moveVector.y>0;

        CornerOfRect movingDirection;
        if (movingRight)
        {
            if(movingDown)
                movingDirection = CornerOfRect::RightDown;
            else
                movingDirection = CornerOfRect::RightUp;
        }
        else
        {
            if (movingDown)
                movingDirection = CornerOfRect::LeftDown;
            else
                movingDirection = CornerOfRect::LeftUp;
        }

        //если прямоугольник движется вниз влево или вверх вправо - то углы, которые сбоку от движения - это верхний левый или нижний правый
        //с bool-ами == работает как NXOR
        if ( movingRight == movingDown )
        {
            upperMovingDot = getCornerCoords(body_arg, CornerOfRect::RightUp);
            upperEpsiVec   = abs(upperMovingDot)*getCornerCoords( epsiQuad, CornerOfRect::LeftDown );

            lowerMovingDot = getCornerCoords(body_arg, CornerOfRect::LeftDown);
            lowerEpsiVec   = abs(lowerMovingDot)*getCornerCoords( epsiQuad, CornerOfRect::RightUp );
        }
        else
        {
            upperMovingDot = getCornerCoords(body_arg, CornerOfRect::LeftUp    );
            upperEpsiVec   = abs(upperMovingDot)*getCornerCoords( epsiQuad, CornerOfRect::RightDown);

            lowerMovingDot = getCornerCoords(body_arg, CornerOfRect::RightDown );
            lowerEpsiVec   = abs(lowerMovingDot)*getCornerCoords( epsiQuad, CornerOfRect::LeftUp   );
        }
        upperMovingDot+=upperEpsiVec;
        lowerMovingDot+=lowerEpsiVec;
        //ищем удары лбом об углы (высокие персонажи мне спасибо не скажут)
        if(     DotCrossesTheHorBoundary(upperMovingDot.x, moveVector.x - upperEpsiVec.x)
            &&  DotCrossesTheVerBoundary(upperMovingDot.y, moveVector.y = upperEpsiVec.y)
           )
        {
            Vector2f tileCorner = getCornerCoords( map_arg.getTileRectByIndex(map_arg.CoordsToIndex(upperMovingDot)), movingDirection);
            Vector2u tileWeMovingTrought;

            if (movingDown)
                tileWeMovingTrought = map_arg.CoordsToIndex(tileCorner+moveVector) - Vector2u(0, 1);
            else
                tileWeMovingTrought = map_arg.CoordsToIndex(tileCorner) - Vector2u(0, 1);

            if (map_arg.at(tileWeMovingTrought)!=nullptr && map_arg.at(tileWeMovingTrought)->solid_m
                &&DotPositionRelativeToVector(tileCorner, upperMovingDot, moveVector)<0  )
            {
                if (movingDown)
                {
                    COLLIDE_HOR()
                }
                else
                {
                    COLLIDE_VERT()
                }
            }
        }
        //теперь ищем столкновения нижним углом
        if(     DotCrossesTheHorBoundary(lowerMovingDot.x, moveVector.x - lowerEpsiVec.x)
           &&   DotCrossesTheVerBoundary(lowerMovingDot.y, moveVector.y - lowerEpsiVec.x)
           )
        {
            Vector2f tileCorner = getCornerCoords( map_arg.getTileRectByIndex(map_arg.CoordsToIndex(lowerMovingDot)), movingDirection);
            Vector2u tileWeMovingTrought;
            if (movingDown)
                tileWeMovingTrought = map_arg.CoordsToIndex(lowerMovingDot)+Vector2u(0, 1);
            else
                tileWeMovingTrought = map_arg.CoordsToIndex(lowerMovingDot+moveVector)+Vector2u(0,1);

            if (map_arg.at(tileWeMovingTrought)!=nullptr && map_arg.at(tileWeMovingTrought)->solid_m
                && DotPositionRelativeToVector(tileCorner, lowerMovingDot, moveVector)>0)
            {
                if (movingDown)
                {
                    COLLIDE_VERT()
                }
                else
                {
                    COLLIDE_HOR()
                }
            }
        }
    }//и-и-и, закончили искать столкновения углом

    //это - врорая часть
    //теперь ищем прямые столкновения


    //горизонтальные
    if (crossedHorizontalBoundary)
    {
        float XcoordOfBody = (moveVector.x>0)? (body_arg.left+body_arg.width) : (body_arg.left);

        Vector2u upperTile = map_arg.CoordsToIndex( Vector2f(XcoordOfBody+moveVector.x, LittleMoreThan(body_arg.top+moveVector.y) ) );

        Vector2u downTile  = map_arg.CoordsToIndex( Vector2f(XcoordOfBody+moveVector.x
                                                             ,LittleLessThan(body_arg.top+body_arg.height+moveVector.y) ));

        if ( !(DotCrossesTheVerBoundary( body_arg.top+abs(body_arg.top)*epsiFraction, moveVector.y-abs(body_arg.top)*epsiFraction )
               &&moveVector.y<0 )
            && map_arg.at(upperTile)!=nullptr && map_arg.at(upperTile)->solid_m )
        {
            COLLIDE_HOR()
            goto EndHorizontalCollizionSearch;
            //поскольку мы уже нашли горизонтадьную коллизию, ловить тут больше нечего
        }
        for (Uint16 i = upperTile.y+1; i<downTile.y; ++i)
        {
            if( map_arg.at(Vector2u(upperTile.x, i))!=nullptr && map_arg.at(Vector2u(upperTile.x, i))->solid_m )
            {
                COLLIDE_HOR()
                goto EndHorizontalCollizionSearch;
            }
        }
        if(!(DotCrossesTheVerBoundary( body_arg.top+body_arg.height - abs(body_arg.top+body_arg.height)*epsiFraction
                                     ,moveVector.y + abs(body_arg.top+body_arg.height)*epsiFraction )
             && moveVector.y >0  )
           && map_arg.at(downTile)!=nullptr && map_arg.at(downTile)->solid_m)
        {
            COLLIDE_HOR()
            goto EndHorizontalCollizionSearch;
        }

    }
    EndHorizontalCollizionSearch:


    //теперь все то же самое, но мы ищем прямые вертикальные коллизии
    if(crossedVerticalBoundary)
    {
        float ycoordOfBody = moveVector.y>0? (body_arg.top+body_arg.height) : (body_arg.top);

        Vector2u leftTile  = map_arg.CoordsToIndex( LittleMoreThan(body_arg.left+moveVector.x),ycoordOfBody+moveVector.y );
        Vector2u rightTile = map_arg.CoordsToIndex( LittleLessThan(body_arg.left+body_arg.width+moveVector.x)
                                                   ,ycoordOfBody+moveVector.y );

        // если верхний/нижний блок находится выше персонажа, то это дело других частей - тех что ищут
        // "прыжки на углы" и "удары углом"
        if (!(DotCrossesTheHorBoundary(body_arg.left+abs(body_arg.left)*epsiFraction, moveVector.x-abs(body_arg.left)*epsiFraction )
              && moveVector.x<0 )
            && map_arg.at(leftTile)!=nullptr && map_arg.at(leftTile)->solid_m)
        {
            COLLIDE_VERT()
            goto EndVerticalCollizionSearch;
        }
        for (Uint16 i = leftTile.x+1; i<rightTile.x; ++i)
        {
            if (map_arg.at(i, leftTile.y)!=nullptr && map_arg.at(i, leftTile.y)->solid_m)
            {
                COLLIDE_VERT()
                goto EndVerticalCollizionSearch;
            }
        }
        if ( !(DotCrossesTheHorBoundary(body_arg.left+body_arg.width-abs(body_arg.left+body_arg.width)*epsiFraction
                                        ,moveVector.x+abs(body_arg.left+body_arg.width)*epsiFraction)
                 &&moveVector.x>0)
            &&map_arg.at(rightTile)!=nullptr && map_arg.at(rightTile)->solid_m )
        {
            COLLIDE_VERT()
        }

    }
    EndVerticalCollizionSearch:

    //это третья часть
    //теперь обрабатываем "прыжки на угол"
    if (!(collidedHorizontally||collidedVertically)
        &&crossedHorizontalBoundary&&crossedVerticalBoundary)
    {
        CornerOfRect movingDirection;
        if (moveVector.x>0)
        {
            if(moveVector.y>0)
                movingDirection = CornerOfRect::RightDown;
            else
                movingDirection = CornerOfRect::RightUp;
        }
        else
        {
            if (moveVector.y>0)
                movingDirection = CornerOfRect::LeftDown;
            else
                movingDirection = CornerOfRect::LeftUp;
        }
        Vector2f movingBodyCorner = getCornerCoords(body_arg, movingDirection);

        Vector2u tileIndex = map_arg.CoordsToIndex(movingBodyCorner+moveVector);

        if( map_arg.at(tileIndex)!=nullptr && map_arg.at(tileIndex)->solid_m )
        {

            Vector2f tileCorner = getCornerCoords(map_arg.getTileRectByIndex(tileIndex), OppositeRectCorner(movingDirection) );

            if( (DotPositionRelativeToVector(tileCorner, movingBodyCorner, moveVector)>0)
                    == (moveVector.y>0) )
            {
                COLLIDE_HOR()
            }
            else
            {
                COLLIDE_VERT()
            }
        }
    }


    //эта функция
    FINISH()

#undef COLLIDE_HOR
#undef COLLIDE_VERT
#undef FINISH
}

//<s>возвращает true или false если выполнение достигает оператора return и мусор в противном случае</s>
//возвращает true если тело имеет опору под ногами
//(с самого начала не стоило передавать эту функцию moveTroughtTileAndCollide)
bool StandingOnTheSolidGround (Tileset2d const& map_arg, FloatRect const& body_arg, const float footingDistance_arg )
{
    //определим наличие опоры под ногами
    bool thereIsFooting = false;
    {
        Vector2f dot = getCornerCoords(body_arg, CornerOfRect::LeftDown);
        dot += Vector2f(abs(dot.x)*epsiFraction, footingDistance_arg);
        Vector2u leftTile = map_arg.CoordsToIndex(dot);

        dot = getCornerCoords(body_arg, CornerOfRect::RightDown) + Vector2f(-epsiFraction, 2*epsiFraction);
        dot += Vector2f(-abs(dot.x)*epsiFraction, footingDistance_arg);
        Uint16 rightTileX = map_arg.CoordsToIndex(dot).x;

        for (int x = leftTile.x; x<=rightTileX; x++)
        {
            if (map_arg.at(x, leftTile.y)!=nullptr && map_arg.at(x, leftTile.y)->solid_m )
            {
                thereIsFooting = true;
                break;
            }
        }
    }
    return thereIsFooting;
}




#endif // GEOMETRYFUNCTIONS_H_INCLUDED
