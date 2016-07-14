#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cmath>
#include <utility>
#include <vector>
#include <cassert>
#include <limits>

#include <algorithm>
#include <numeric>

using namespace std;
using namespace sf;

//вы стоите на блоке пола. Очевидно, ваши ноги не относятся к тайлу, на котором вы стоите, арифметика чисел с плавающей запятой
//считает иначе,
//так что вы спотыкаетесь на ровном месте. Отныне вам нужно притворятся висящим в воздухе где-то в 1/100000 вашего размера от пола.

//а еще вы подопрете стену справа от вас, то окажется, что ваш бок, которым вы обперлись уже внутри нее
constexpr float epsiFraction = numeric_limits<float>::epsilon()*2;
//однако, совершенно непоятно, отчего разрабам sfml не сделали FloatRect constexpr-совместимым
const FloatRect epsiQuad(0, 0, epsiFraction, epsiFraction);

template<typename T>
T minByAbs(T arg1, T arg2 )
{
    return min(arg1, arg2, [](T t1, T t2)->bool{return abs(t1)<abs(t2);} );
}

float vectorTan (Vector2f const& vec_arg)
{
    return vec_arg.y/vec_arg.x;
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


class Entity
{
public:
    Entity (){}
    Entity ( vector<Entity*> * where_arg)
        :environment_m(where_arg)
    {   }

    virtual ~Entity(){}

    virtual void Update(sf::Time frameTime_arg)=0;

    virtual sf::Drawable* getDrawableComponent()=0;//должен возвращать nullptr, если не предназначем для рисования
    //хотя это не очень-то эффективно, если я буду делать много энтити не предназначенных для рисования, лучше
    //использовать флаг drawable, или пересмотреть иерархию классов

    vector<Entity*> *environment_m;
};

float floorLevel = 1200;



class Tile
{
public:
    Tile(bool solid_arg)
        :solid_m(solid_arg)
        {    }
    Tile(){}

    bool solid_m = true;
    sf::RectangleShape* drawableComponent;
};

class Tileset2d
{
public:
    Tileset2d (Uint16 width_arg, Uint16 height_arg)
        :   content_m(height_arg*width_arg, nullptr),
            height_m(height_arg),
            width_m(width_arg)
    {    }

    //reference to pointer
    Tile*& at (Vector2u index2d_arg)
    {
        return at(index2d_arg.x, index2d_arg.y);
    }
    Tile*& at (Uint16 x_arg, Uint16 y_arg)
    {
        return content_m[y_arg*width_m + x_arg];
    }
    Tile*& at (Uint32 index_arg)
    {
        return content_m[index_arg];
    }
    Tile* const& at (Vector2u index2d_arg) const
    {
        return at(index2d_arg.x, index2d_arg.y);
    }
    Tile* const & at (Uint16 x_arg, Uint16 y_arg) const
    {
        if (x_arg<width_m && x_arg>=0 && y_arg<height_m && y_arg>=0)
            return content_m[y_arg*width_m + x_arg];
        else
            return nullptr;
    }
    inline Uint16 getWidth () const
    {
        return width_m;
    }
    Uint16 getHeight() const
    {
        return height_m;
    }
    inline Uint16 XCoordToIndex(float xCoord_arg) const
    {
        return static_cast<float>(xCoord_arg - startOffset_m.x)/tileSize;
    }
    inline Uint16 YCoordToIndex(float yCoord_arg) const
    {
        return static_cast<float>(yCoord_arg - startOffset_m.y)/tileSize;
    }
    inline Vector2u CoordsToIndex(float x_arg, float y_arg) const
    {
        return  Vector2u(XCoordToIndex(x_arg), YCoordToIndex(y_arg)) ;
    }
    inline Vector2u CoordsToIndex(Vector2f coords_arg) const
    {
        return CoordsToIndex(coords_arg.x, coords_arg.y);
    }
    inline Vector2f IndexToCoord( Vector2u index_arg) const
    {
        return IndexToCoord(index_arg.x, index_arg.y);
    }
    inline Vector2f IndexToCoord( Uint16 x_arg, Uint16 y_arg) const
    {
        return Vector2f(XIndexToCoord(x_arg), YIndexToCoord(y_arg));
    }
    inline float XIndexToCoord (Uint16 x_arg) const
    {
        return x_arg*tileSize + startOffset_m.x;
    }
    inline float YIndexToCoord (Uint16 y_arg) const
    {
        return y_arg*tileSize + startOffset_m.y;
    }
    inline FloatRect getTileRectByIndex(Uint16 x, Uint16 y) const
    {
        return getTileRectByIndex(Vector2u(x,y));
    }
    inline FloatRect getTileRectByIndex(Vector2u index_coord) const
    {
        return FloatRect(index_coord.x*tileSize, index_coord.y*tileSize, tileSize, tileSize);
    }

    Uint8 tileSize = 100;
private:
    Vector2f startOffset_m = Vector2f(0,0); //я не могу позволить себе сделать Tileset2d прибитым гвоздями к началу координат
    vector<Tile*> content_m;
    Uint16 height_m;
    Uint16 width_m;
};




//функция работает, только если вектор от начала вектора-аргумента до точки и вектор-аргумент находятся в одной четверти
//-1 точка ниже, 1 - выше, 0 - на нем
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



//данная функция меняет скорость и положение тела, но не нго габариты
//возвращает true, если к концу работы под ногамии у тела есть опора, false - если таковой нет
/*
bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg)
{
    const Vector2f origMoveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );
    Vector2f moveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );

    if (bodySpeed_arg==Vector2f(0,0) || bodySpeed_arg == Vector2f(0,0))
    {
        return false;
    }

    {
        float xcoord = (moveVector.x>0) ? (body_arg.left+body_arg.width) : (body_arg.left);
        float ycoord = (moveVector.y>0) ? (body_arg.top+body_arg.height) : (body_arg.top);
        if (map_arg.XCoordToIndex(xcoord)==map_arg.XCoordToIndex(xcoord + moveVector.x) &&
            map_arg.YCoordToIndex(ycoord)==map_arg.YCoordToIndex(ycoord + moveVector.y))
        {
            //если тело не пересекает границ тайлов, значит нет может быть столкновений с ними.
            body_arg.top += moveVector.x;
            body_arg.left +=moveVector.y;
            return false;
        }
    }

    bool grounded=false; //имеется в виду "приземлился" а не "наказан"

    //проверка, что вектор перемещения короче стороны тайла
    if (moveVector.x > map_arg.tileSize || moveVector.y > map_arg.tileSize)
    {
        //если скорость слишком большая/тайлы маленькие - разбиваем движение на части и производим по частям
        int stageCount = ceil( max( abs(moveVector.x) , abs(moveVector.y) )/ static_cast<float>(map_arg.tileSize));
        for (int i = 0; i<stageCount; i++)
        {
            //вложенность рекурси - не обльше 1, т.к. я разбил движение на гарантированно достаточно малые куски
            if (MoveTroughtTilesAndCollide(map_arg, body_arg, bodySpeed_arg, time_arg/static_cast<Int64>(stageCount) ))
                grounded=true;
            if (bodySpeed_arg==Vector2f(0,0))
                break;
        }
        return grounded;
    }

    //проверка, чтобы тело не пролетело сквозь угол тайла
    //для этого нужно смотреть, чтобы он своим углом не зацепился за угол тайла
    if (bodySpeed_arg.y != 0 && bodySpeed_arg.x!=0)
    {
        if (bodySpeed_arg.y>0) //движемся вниз
        {
            if ( bodySpeed_arg.x>0 ) //движемся вниз вправо
            {
                {
                    //проверяем верхний правый угол
                    Vector2f posUpCorner = getCornerCoords(body_arg, CornerOfRect::RightUp);
                    Vector2u currentUpCornerTile = map_arg.CoordsToIndex(posUpCorner),
                            newUpCornerTile = map_arg.CoordsToIndex(posUpCorner+moveVector);
                    if ( (currentUpCornerTile.x!=newUpCornerTile.x) && (currentUpCornerTile.y != newUpCornerTile.y) )
                    {
                        //т.е. угол сдвинулся в новый блок не просто вниз/вбое, а по диагонали
                        if (map_arg.at(currentUpCornerTile.x+1, newUpCornerTile.y)!= nullptr
                            && map_arg.at(currentUpCornerTile.x+1, newUpCornerTile.y)->solid_m
                            && (DotPositionRelativeToVector( map_arg.IndexToCoord(newUpCornerTile), posUpCorner, moveVector )<0))
                        {
                            //кажется у нас коллизия!
                            //делаем полученное перемещение таким, чтобы при его выполнении тело втыкалось в угол, и не преодолевало его
                            moveVector.x = map_arg.IndexToCoord(newUpCornerTile).x - posUpCorner.x;
                        }
                    }
                }
                {
                    //теперь -левый нижний угол
                    Vector2f posDLCorner = getCornerCoords(body_arg, CornerOfRect::LeftDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(posDLCorner),
                            newCornerTileIndex = map_arg.CoordsToIndex(posDLCorner+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y+1);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(newCornerTileIndex), posDLCorner, moveVector )>0)
                        {
                            //столкновение снизу
                            moveVector.y = minByAbs(map_arg.IndexToCoord(newCornerTileIndex).y - posDLCorner.y, moveVector.y );
                            moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                            grounded=true;
                        }
                    }
                }
            }
            else //движемся вниз влево
            {
                {//проследим за верхним левым углом
                    Vector2f cornerULCoord = getCornerCoords(body_arg, CornerOfRect::LeftUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerULCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerULCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x-1, cornerTileIndex.y);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(newCornerTileIndex.x+1, newCornerTileIndex.y)
                                                           , cornerULCoord, moveVector )<0)
                        {
                            //столкновение сбоку
                            moveVector.x = minByAbs (moveVector.x, map_arg.XIndexToCoord(cornerTileIndex.x));
                        }
                    }
                }
                {//за нижним правым
                    Vector2f cornerDRCoord = getCornerCoords(body_arg, CornerOfRect::RightDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerDRCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerDRCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y+1);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(newCornerTileIndex.x+1, newCornerTileIndex.y)
                                                           , cornerDRCoord, moveVector )>0)
                        {
                            //столкновение снизу
                            moveVector.y = minByAbs(moveVector.y, map_arg.YCoordToIndex(newCornerTileIndex.y) - cornerDRCoord.y);
                            moveVector.x = minByAbs( moveVector.x, origMoveVector.x*(origMoveVector.y/moveVector.y) );
                            grounded=true;
                        }
                    }
                }
            }
        }
        else //
        {
            if ( bodySpeed_arg.x>0 ) //движемся вверх вправо
            {
                {//левый верхний
                    Vector2f cornerULCoord = getCornerCoords(body_arg, CornerOfRect::LeftUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerULCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerULCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex.x+1, cornerTileIndex.y)
                                                           , cornerULCoord, moveVector )<0)
                        {
                            //столкновение об потолок
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord(cornerTileIndex.y) - cornerULCoord.y );
                        }
                    }
                }
                {//нижний правый
                    Vector2f cornerDRCoord = getCornerCoords(body_arg, CornerOfRect::RightDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerDRCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerDRCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x+1, cornerTileIndex.y);
                        if (tl!=nullptr && tl->solid_m
                            &&DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex.x+1, cornerTileIndex.y)
                                                          , cornerDRCoord, moveVector )>0)
                        {
                            //столкновение об правую стенку
                            moveVector.x = minByAbs(moveVector.x
                                                    , map_arg.XIndexToCoord(cornerTileIndex.x+1) - cornerDRCoord.x);
                        }
                    }
                }
            }
            else //движемся вверх влево
            {
                {//верхний правый
                    Vector2f cornerURCoord = getCornerCoords(body_arg, CornerOfRect::RightUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerURCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerURCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            &&DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex)
                                                          , cornerURCoord, moveVector )<0)
                        {
                            //столкновение об потолок
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord(cornerTileIndex.y) - cornerURCoord.y );
                        }
                    }
                }
                {//нижний левый
                    Vector2f cornerDLCoord = getCornerCoords(body_arg, CornerOfRect::LeftDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerDLCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerDLCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            &&DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex)
                                                          , cornerDLCoord, moveVector )>0)
                        {
                            //сбоку
                            moveVector.x = minByAbs(moveVector.x, map_arg.XIndexToCoord(cornerTileIndex.x) - cornerDLCoord.x);
                        }
                    }
                }
            }
        }
    }//конец блока, обрабатывающего столкновения об углы при движении по диагонали
    //кода много, но выполняется только 1/4 часть, а то и меньше

    //поиск столкновений об грани блоков
    {
        // гориз. составляющая
        {
            Vector2u upperTile, downTile;

            if (moveVector.x>0)
            {
                upperTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::RightUp  )+moveVector);
                downTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::RightDown)+moveVector);
            }
            else
            {
                upperTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::LeftUp  )+moveVector);
                downTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::LeftDown)+moveVector);
            }
            if (upperTile!=downTile)
            {
                if (  map_arg.YIndexToCoord(upperTile.y-1) > body_arg.top
                    && map_arg.at (upperTile)!=nullptr && map_arg.at(upperTile)->solid_m )
                {
                    moveVector.x = minByAbs(moveVector.x,
                                            map_arg.XIndexToCoord( (moveVector.x>0)? (upperTile.x) : (upperTile.x-1))
                                            - (moveVector.x>0 ) ?  (body_arg.left+body_arg.width) : body_arg.left);
                }

                for (Uint16 i = upperTile.y+1; i < downTile.y; ++i) //значение y возрастает "вниз"
                {
                    Vector2u currentTileI = Vector2u (upperTile.x, i);
                    if (map_arg.at (currentTileI)!=nullptr && map_arg.at(currentTileI)->solid_m )
                        moveVector.x = minByAbs(moveVector.x,
                                                map_arg.XIndexToCoord( (moveVector.x>0)? (currentTileI.x) : (currentTileI.x-1))
                                                    - (moveVector.x>0 ) ?  (body_arg.left+body_arg.width) : body_arg.left
                                                );
                }

                if (  map_arg.YIndexToCoord(downTile.y) < (body_arg.top+body_arg.height)
                    && map_arg.at (downTile)!=nullptr && map_arg.at(downTile)->solid_m )
                {
                    moveVector.x = minByAbs(moveVector.x,
                                    map_arg.XIndexToCoord( (moveVector.x>0)? (downTile.x) : (downTile.x-1))
                                    - (moveVector.x>0 ) ?  (body_arg.left+body_arg.width) : body_arg.left);
                }

            }
            else
            {
                if ( (map_arg.YCoordToIndex(body_arg.top) == upperTile.y)
                        && (map_arg.at(upperTile)!=nullptr) && map_arg.at(upperTile)->solid_m)
                {
                    moveVector.x = minByAbs(moveVector.x,
                                            map_arg.XIndexToCoord( (moveVector.x>0)? (upperTile.x) : (upperTile.x-1))
                                            - (moveVector.x>0 ) ?  (body_arg.left+body_arg.width) : body_arg.left );
                }
            }
        }//конец обработки гориз. составляющей

        //вертикальная составляющая
        {
            Vector2u leftTileI, rightTileI;
            if (moveVector.y>0)
            {//вниз
                leftTileI =  map_arg.CoordsToIndex(  Vector2f(body_arg.left,                body_arg.top+body_arg.height)+moveVector );
                rightTileI=  map_arg.CoordsToIndex(  Vector2f(body_arg.left+body_arg.width, body_arg.top+body_arg.height)+moveVector );
            }
            else
            {//вверх
                leftTileI =  map_arg.CoordsToIndex(  Vector2f(body_arg.left,                body_arg.top)+moveVector );
                rightTileI=  map_arg.CoordsToIndex(  Vector2f(body_arg.left+body_arg.width, body_arg.top)+moveVector );
            }
            if (leftTileI!=rightTileI)
            {
                if ( map_arg.XIndexToCoord(leftTileI.x+1) >= body_arg.left
                    && map_arg.at(leftTileI)!=nullptr && map_arg.at(leftTileI)->solid_m )
                {
                    if (moveVector.y>0)
                    {
                        //столкновение снизу
                        grounded=true;

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y ) - (body_arg.top+body_arg.height));
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        //удар об потолок
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y+1 ) - (body_arg.top));
                    }
                }
                for (Uint16 i = leftTileI.x+1; i<rightTileI.x; i++)
                {
                    Vector2u currentTileI (i, leftTileI.y);
                    if (map_arg.at (currentTileI)!=nullptr && map_arg.at(currentTileI)->solid_m )
                    {
                        if (moveVector.y>0)
                        {
                            //на под
                            grounded=true;
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( currentTileI.y ) - (body_arg.top+body_arg.height) );
                            moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                        }
                        else
                        {
                            //об потолок
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( currentTileI.y+1 ) - (body_arg.top));
                        }
                    }
                }
                if (map_arg.XIndexToCoord(rightTileI.x) <= (body_arg.left+body_arg.width)
                    && map_arg.at(rightTileI)!=nullptr && map_arg.at(rightTileI)->solid_m )
                {
                    if (moveVector.y>0)
                    {
                        //столкновение снизу
                        grounded=true;

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( rightTileI.y ) - (body_arg.top+body_arg.height) );
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( rightTileI.y+1 ) - (body_arg.top) );
                    }
                }
            }
            else
            {
                if ( (map_arg.XCoordToIndex(body_arg.left) == leftTileI.x)
                        && (map_arg.at(leftTileI)!=nullptr) && map_arg.at(leftTileI)->solid_m)
                {
                    if (moveVector.y>0)
                    {
                        grounded=true;

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y ) - (body_arg.top+body_arg.height) );
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y-1 ) - (body_arg.top) );
                    }
                }
            }

        }
    }//конец кода ищущего прямые столкновения


    //код обрабатывающий "прыжки на углы" по диагонали, если тело не столкнулось ни с чем до сих пор
    float xcoord = (moveVector.x>0) ? (body_arg.left+body_arg.width) : (body_arg.left);
    float ycoord = (moveVector.y>0) ? (body_arg.top+body_arg.height) : (body_arg.top);

    if (origMoveVector == moveVector
        && map_arg.XCoordToIndex(xcoord)!=map_arg.XCoordToIndex(xcoord + moveVector.x)
        && map_arg.YCoordToIndex(ycoord)!=map_arg.YCoordToIndex(ycoord + moveVector.y) )
    {


        CornerOfRect direction;
        if (moveVector.x>0)
            if (moveVector.y>0)
                direction = CornerOfRect::RightDown;
            else
                direction = CornerOfRect::RightUp;
        else
            if (moveVector.y>0)
                direction = CornerOfRect::LeftDown;
            else
                direction = CornerOfRect::LeftUp;

        //угол коллизии тела, которй падает на тайл
        Vector2f movingDot = getCornerCoords(body_arg, direction);

        Vector2u tileIndex = map_arg.CoordsToIndex(movingDot+moveVector);


        //угол тайла на который падает тело
        Vector2f tileCorner= getCornerCoords( map_arg.getTileRectByIndex(tileIndex), OppositeRectCorner(direction) );
        if (map_arg.at(tileIndex)!=nullptr && (map_arg.at(tileIndex)->solid_m))
        {
            if ( DotPositionRelativeToVector( tileCorner, movingDot, moveVector ) > 0 )
            {
                if (moveVector.y>0)
                {
                    //столкновение об стенку блока
                    moveVector.x = tileCorner.x - movingDot.x;
                }
                else
                {
                    //об потолок блока
                    moveVector.y = tileCorner.y - movingDot.y;
                }
            }
            else
            {
                if (moveVector.y>0)
                {
                    //на пол
                    grounded = true;
                    moveVector.y = minByAbs(moveVector.y, tileCorner.y - movingDot.y);
                    moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );

                }
                else
                {
                    //об стенку
                    moveVector.x = tileCorner.x - movingDot.x;
                }
            }
        }
    }

    if (moveVector.x != origMoveVector.x)
    {
        bodySpeed_arg.x = 0;
    }
    if (moveVector.y != origMoveVector.y)
    {
        bodySpeed_arg.y = 0;
    }

    body_arg.left += moveVector.x;
    body_arg.top += moveVector.y;
    return grounded;
}

*/


bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg)
{

    const Vector2f origMoveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );
    Vector2f moveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );

    if (bodySpeed_arg==Vector2f(0,0) || bodySpeed_arg == Vector2f(0,0))
    {
        return false;
    }


    static auto DotCrossesTheHorBoundary = [&map_arg, &moveVector] ( float xcoord ) ->bool
        {
            return map_arg.XCoordToIndex(xcoord) != map_arg.XCoordToIndex(xcoord+moveVector.x);
        };
    static auto DotCrossesTheVerBoundary = [&map_arg, &moveVector] ( float ycoord ) ->bool
        {
            return map_arg.YCoordToIndex(ycoord) != map_arg.YCoordToIndex(ycoord+moveVector.y);
        };

    bool crossedVerticalBoundary, crossedHorizontalBoundary;
    {
        float xcoord = moveVector.x>0? (body_arg.left+body_arg.width) : body_arg.left;
        crossedHorizontalBoundary = DotCrossesTheHorBoundary(xcoord);
        float ycoord = moveVector.y>0? (body_arg.top + body_arg.height) : body_arg.top;
        crossedVerticalBoundary = DotCrossesTheVerBoundary(ycoord);
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
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex(body_arg.left+body_arg.width)+1 );
                    moveVector.x = wallX*(1-epsiFraction) - (body_arg.left+body_arg.width) ;

                    moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
                }
                else
                {
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex(body_arg.left) );
                    moveVector.x = wallX*(1+epsiFraction) - (body_arg.left);

                    moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
                }
                bodySpeed_arg.x=0;
            }
            if (collidedVertically)
            {
                if (moveVector.y>0)
                {
                    grounded = true;
                    float floorY =  map_arg.YIndexToCoord( map_arg.YCoordToIndex(body_arg.top+body_arg.height) + 1 );
                    moveVector.y = floorY - (body_arg.top+body_arg.height);

                    moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
                }
                else
                {
                    float ceilingY = map_arg.YCoordToIndex( map_arg.YIndexToCoord(body_arg.top));
                    moveVector.y = ceilingY - body_arg.top;

                    moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
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

    //обнаруживаем столкновения об угол углом коллизии
    if(  (moveVector.x!=0) && (moveVector.y!=0) )
    {
        Vector2f upperMovingDot, lowerMovingDot;

        bool movingRight = moveVector.x>0,
                movingDown = moveVector.y>0;

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

        //если бы я мог прилепить сюда бумажку со схемами, этот код был бы куда понятнее
        if ( movingRight == movingDown )
        {
            upperMovingDot = getCornerCoords(body_arg, CornerOfRect::RightUp);
            lowerMovingDot = getCornerCoords(body_arg, CornerOfRect::LeftDown);
        }
        else
        {
            upperMovingDot = getCornerCoords(body_arg, CornerOfRect::LeftUp);
            lowerMovingDot = getCornerCoords(body_arg, CornerOfRect::RightDown);
        }
        //ищем удары лбом об углы
        if( DotCrossesTheHorBoundary(upperMovingDot.x) && DotCrossesTheVerBoundary(upperMovingDot.y) )
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
        if( DotCrossesTheHorBoundary(lowerMovingDot.x) && DotCrossesTheVerBoundary(lowerMovingDot.y) )
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
    //теперь ищем прямые столкновения


    //горизонтальные
    if (crossedHorizontalBoundary)
    {
        float XcoordOfBody = (moveVector.x>0)? (body_arg.left+body_arg.width) : (body_arg.left);

        Vector2u upperTile = map_arg.CoordsToIndex( Vector2f(XcoordOfBody+moveVector.x,
                                                             (body_arg.top+moveVector.y))*(1+epsiFraction) );
        Vector2u downTile  = map_arg.CoordsToIndex( Vector2f(XcoordOfBody+moveVector.x,
                                                             (body_arg.top+body_arg.height+moveVector.y)*(1-epsiFraction) ));

        if ( /*(moveVector.y<0) && */!DotCrossesTheVerBoundary( body_arg.top )
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
        if(/*moveVector.y>0 && */!DotCrossesTheHorBoundary(body_arg.top+body_arg.height)
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

        Vector2u leftTile  = map_arg.CoordsToIndex( (body_arg.left+moveVector.x)*(1+epsiFraction),
                                                   ycoordOfBody+moveVector.y );
        Vector2u rightTile = map_arg.CoordsToIndex( (body_arg.left+body_arg.width+moveVector.x)*(1-epsiFraction),
                                                   ycoordOfBody+moveVector.y );

        if (/*moveVector.x<0 &&*/ !DotCrossesTheHorBoundary(body_arg.left) &&
            map_arg.at(leftTile)!=nullptr && map_arg.at(leftTile)->solid_m)
        {
            //COLLIDE_VERT()
            {
                collidedVertically=true;
                crossedVerticalBoundary = false;
                if (crossedHorizontalBoundary&&crossedVerticalBoundary)
                {
                    FINISH()
                }
            }
            goto EndVerticalCollizionSearch;
        }
        for (Uint16 i = leftTile.x+1; i<rightTile.x; ++i)
        {
            if (map_arg.at(i, leftTile.y)!=nullptr && map_arg.at(i, leftTile.y)->solid_m)
            {
                //COLLIDE_VERT()
                {
                    collidedVertically=true;
                    crossedVerticalBoundary = false;
                    if (crossedHorizontalBoundary&&crossedVerticalBoundary)
                    {
                        FINISH()
                    }
                }

                goto EndVerticalCollizionSearch;
            }
        }
        if ( (leftTile.x!=rightTile.x)/* && moveVector.x>0*/ && !DotCrossesTheHorBoundary(body_arg.left+body_arg.width)
            &&map_arg.at(rightTile)!=nullptr && map_arg.at(rightTile)->solid_m )
        {
            //COLLIDE_VERT()
            {
                collidedVertically=true;
                crossedVerticalBoundary = false;
                if (crossedHorizontalBoundary&&crossedVerticalBoundary)
                {
                    FINISH()
                }
            }
            goto EndVerticalCollizionSearch;
        }

    }
    EndVerticalCollizionSearch:

    //теперь обрабатываем "прыжки на угол"
    if (!(collidedHorizontally||collidedVertically)
        &&moveVector.x!=0 && moveVector.y!=0 )
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

        if(map_arg.at(tileIndex)!=nullptr && map_arg.at(tileIndex)->solid_m)
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

    FINISH()

#undef COLLIDE_HOR
#undef COLLIDE_VERT
#undef FINISH
}

class PlayableCharacter : public Entity
{
public:
    PlayableCharacter(FloatRect* collizion_arg, Sprite* renderComponent_arg)
    : collizion_m(collizion_arg), renderComponent_m(renderComponent_arg)
    {    }

    ~PlayableCharacter()
    {

    }

    void readApplyUserInput()
    {
        if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A)!=Keyboard::isKeyPressed(Keyboard::D) )
        {
            if (state_m != State_m::inAir)
            {
                if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A) )
                {
                    speed_m.x = -walkingSpeed_m;
                    lookingLeft_m = true;
                }
                else
                {
                    speed_m.x = walkingSpeed_m;
                    lookingLeft_m = false;
                }
                state_m = State_m::walking;
            }
        }
        else if (state_m != State_m::inAir)
        {
            state_m = State_m::standing;
        }
        if  (   ((sf::Keyboard::isKeyPressed(Keyboard::W))||(sf::Keyboard::isKeyPressed(Keyboard::Space)))
                && state_m!= State_m::inAir
            )
        {
            state_m = State_m::inAir;
            speed_m.y = -jumpingSpeed_m;
        }
    }

    void Update(sf::Time frameTime_arg) override
    {
        readApplyUserInput();

        /*
        static vector<Uint32> frameTimes(400);
        static vector<Uint32>::iterator currentIter = frameTimes.begin();
        ++currentIter;
        *currentIter = frameTime_arg.asMicroseconds();
        if (currentIter==frameTimes.end())
        {
            currentIter=frameTimes.begin();
            cout << "mics: "<< ( accumulate(frameTimes.begin(), frameTimes.end(), 0) / 400)  << endl;
        }

        currentAnimFrame_m+=animSpeed_m*frameTime_arg.asMicroseconds();
        */


        //apply state-specific things
        if (state_m == State_m::standing)
            speed_m.x = 0;

        //двигаемся сквозь тайловую карту
        if (MoveTroughtTilesAndCollide(*locationMap_m, *collizion_m, speed_m, frameTime_arg))
            state_m=State_m::standing;

        collizion_m->top += speed_m.y * frameTime_arg.asMicroseconds();

        if (state_m == State_m::inAir)
        {
            speed_m.y+= 9.8/(10000000);
        }


        if( (collizion_m->top + collizion_m->height)  > floorLevel)
        {
            collizion_m->top = floorLevel-collizion_m->height;
            speed_m.y = max(0.0f, speed_m.y);
            if (speed_m.x == 0)
                state_m = State_m::standing;
            else
                state_m = State_m::walking;
            //onGround_m=true;
        }

    }

    sf::Drawable* getDrawableComponent() override
    {
        //приводим спрайт в соотв. с состоянием персонажа и отдаем


        renderComponent_m->setPosition( Vector2f(collizion_m->left, collizion_m->top)
                                        +spriteCoordRelativeToCollision_m
                                      );
        if (state_m == State_m::walking)
        {
            if (currentAnimFrame_m>6)
                currentAnimFrame_m = fmod (currentAnimFrame_m, 6);
            if (lookingLeft_m)
                renderComponent_m->setTextureRect(IntRect(40+40*int(currentAnimFrame_m), 244, -40, 50));
            else
                renderComponent_m->setTextureRect(IntRect(40*int(currentAnimFrame_m), 244, 40, 50));
        }
        else if (state_m == State_m::standing)
        {
            if (currentAnimFrame_m>5)
                currentAnimFrame_m = fmod (currentAnimFrame_m, 5.0f);

            if (currentAnimFrame_m>3)
            {
                if (lookingLeft_m)
                    renderComponent_m->setTextureRect(IntRect(44+44*int(5 - currentAnimFrame_m), 189, -44, 50));
                else
                    renderComponent_m->setTextureRect(IntRect(44*int(5 - currentAnimFrame_m), 189, 44, 50));
            }
            else if (lookingLeft_m)
                renderComponent_m->setTextureRect(IntRect(44+44*int(currentAnimFrame_m), 189, -44, 50));
            else
                renderComponent_m->setTextureRect(IntRect(44*int(currentAnimFrame_m), 189, 44, 50));
        } //189 - top, 44 - width, 50 - height


        return renderComponent_m;
    }

    Vector2f spriteCoordRelativeToCollision_m = Vector2f(0,0);
    //смещение левого верхнего угла спрайта от верхнего левого угла коллизии
    float currentAnimFrame_m=0;

    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;

    bool lookingLeft_m      = false; //в противном случае смотрит вправо

    Vector2f speed_m        = Vector2f(0,0);

    //скорость - в пикселях в микросекунду
    float jumpingSpeed_m        = 0.0012; //вертикальная скорость, которая ему придается при прыжке
    float walkingSpeed_m        = 0.0006;  //скорость, с которой он ходит
    float animSpeed_m           = 0.00001; // смен кадров в микросекунду
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;

    Tileset2d* locationMap_m    = nullptr;
    //Entity, с полем vector<Entity*> *environment_m, которая родитель
    //будет использоваться позже, для взаимодействия с миром
};

int main()
{
    //tiles
    Tile stone;
    stone.solid_m = true;
    sf::RectangleShape stoneTileShape(Vector2f(100, 100));
    stoneTileShape.setFillColor(sf::Color( 200, 200, 200));
    stone.drawableComponent = &stoneTileShape;

    //levelMap
    //Tile* levelTiles[levelWidth][levelHight];
    Tileset2d levelTiles(24,11);
    {
        constexpr char const* levelCheme[] =
        {
            "s                      s",
            "                       s",
            "ssssssssssssssssssssssss",
            "s                      s",
            "s   sssss              s",
            "s                      s",
            "s         sssss        s",
            "s             s        s",
            "s             s        s",
            "s             s        s",
            "ssss   sssssssssssssssss"
        };

        for (int x=0; x<levelTiles.getWidth(); ++x)
            for (int y=0; y<levelTiles.getHeight(); ++y)
        {
            switch (levelCheme[y][x])
            {
            case 's':
                levelTiles.at(x, y) = &stone;
                break;
            default:
                break;
            }
        }
    }

    FloatRect fr (170, 900-0.0003, 80, 100);

    Vector2f speed (0, 0.0005);
    Time time = microseconds(1);
    while (true)
    {
        ;
        MoveTroughtTilesAndCollide(levelTiles, fr, speed, time);
    }
}

int main_fun()
{

    //tiles
    Tile stone;
    stone.solid_m = true;
    sf::RectangleShape stoneTileShape(Vector2f(100, 100));
    stoneTileShape.setFillColor(sf::Color( 200, 200, 200));
    stone.drawableComponent = &stoneTileShape;

    //levelMap
    //Tile* levelTiles[levelWidth][levelHight];
    Tileset2d levelTiles(24,11);
    {
        constexpr char const* levelCheme[] =
        {
            "s                      s",
            "s                      s",
            "s   ssssssssssssssssssss",
            "s                      s",
            "s   sssss              s",
            "s                      s",
            "s         sssss        s",
            "s             s        s",
            "s             s        s",
            "s             s        s",
            "ssss   sssssssssssssssss"
        };

        for (int x=0; x<levelTiles.getWidth(); ++x)
            for (int y=0; y<levelTiles.getHeight(); ++y)
        {
            switch (levelCheme[y][x])
            {
            case 's':
                levelTiles.at(x, y) = &stone;
                break;
            default:
                break;
            }
        }
    }

    //playable character
    sf::Texture myTxtr;
    myTxtr.loadFromFile("fang.png");
    sf::Sprite sprt;
    sprt.setTexture(myTxtr);
    sprt.setTextureRect( IntRect(0, 244, 40, 50) );
    sprt.setScale(2,2);
    sprt.setPosition(100, 100);
    FloatRect FangCollizion = sprt.getGlobalBounds();
    PlayableCharacter Fang(&FangCollizion ,&sprt);
    Fang.locationMap_m = &levelTiles;
    Fang.state_m = PlayableCharacter::State_m::inAir;
    //level content
    vector<Entity*> entitiesOnLevel;
    entitiesOnLevel.push_back(&Fang);


    //View
    sf::View myCamera;
    myCamera.reset(FloatRect(0, 0, 1024, 768) );

    Clock clc; clc.restart();
    //window
    sf::RenderWindow window(sf::VideoMode(1024, 768), "SFML works!", sf::Style::Close);
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);
    window.setView(myCamera);
    //main cycle
    while (window.isOpen())
    {
        sf::Time frameTime = clc.getElapsedTime();
        clc.restart();



        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Return)
            {
                cout << "\nx = "<< Fang.collizion_m->left << " y = " << Fang.collizion_m->top;
            }
        }


        for (Entity* toUpdate : entitiesOnLevel)
        {
            toUpdate->Update(frameTime);
        }

        //rendering

        window.clear();
        myCamera.setCenter(Vector2f (Fang.collizion_m->left +Fang.collizion_m->width/2,
                                       Fang.collizion_m->top+Fang.collizion_m->height/2) );
        window.setView(myCamera);
        for (Entity* toDraw : entitiesOnLevel)
        {
            window.draw( *(toDraw->getDrawableComponent()) );
        }
        for (int x=0; x<levelTiles.getWidth(); ++x)
            for (int y=0; y<levelTiles.getHeight(); ++y)
        {
            if (levelTiles.at(x,y) !=nullptr)
            {
                levelTiles.at(x,y)->drawableComponent->setPosition(levelTiles.tileSize*(x), levelTiles.tileSize*(y));
                window.draw( * (levelTiles.at(x,y)->drawableComponent) );
            }
        }

        window.setView(window.getDefaultView());
        window.draw(shape);

        window.display();
    }
    return 0;
}
