#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cmath>
#include <utility>
#include <vector>

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

    virtual sf::Drawable* getDrawableComponent()=0;//������ ���������� nullptr, ���� �� ������������ ��� ���������
    //���� ��� �� �����-�� ����������, ���� � ���� ������ ����� ������ �� ��������������� ��� ���������, �����
    //������������ ���� drawable, ��� ������������ �������� �������

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
        return content_m[y_arg*width_m + x_arg];
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
        return CoordsToIndex ( Vector2f(XCoordToIndex(x_arg), YCoordToIndex(y_arg)) );
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
    Vector2f startOffset_m = Vector2f(0,0); //� �� ���� ��������� ���� ������� Tileset2d �������� �������� � ������ ���������
    vector<Tile*> content_m;
    Uint16 height_m;
    Uint16 width_m;
};


//������ ������� ������ �������� � ��������� ����, �� �� ��� ��������
//���������� true, ���� ���� ����������� � ���-���� ����� (�.�. ������������), false - ���� ������ ��������


//������� ��������, ������ ���� ������ �� ������ �������-��������� �� ����� � ������-�������� ��������� � ����� ��������
//-1 ����� ����, 1 - ����, 0 - �� ���
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


bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg)
{
    const Vector2f origMoveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );
    if (bodySpeed_arg==Vector2f(0,0) || bodySpeed_arg == Vector2f(0,0))
    {
        return false;
    }

    bool grounded=false; //������� � ���� "�����������" � �� "�������"

    Vector2f moveVector(bodySpeed_arg* static_cast<float>(time_arg.asMicroseconds()) );
    //��������, ��� ������ ����������� ������ ������� �����
    if (moveVector.x > map_arg.tileSize || moveVector.y > map_arg.tileSize)
    {
        //���� �������� ������� �������/����� ��������� - ��������� �������� �� ����� � ���������� �� ������
        int stageCount = ceil( max( abs(moveVector.x) , abs(moveVector.y) )/ static_cast<float>(map_arg.tileSize));
        for (int i = 0; i<stageCount; i++)
        {
            //����������� ������� - �� ������ 1, �.�. � ������ �������� �� �������������� ���������� ����� �����
            if (MoveTroughtTilesAndCollide(map_arg, body_arg, bodySpeed_arg, time_arg/static_cast<Int64>(stageCount) ))
                grounded=true;
            if (bodySpeed_arg==Vector2f(0,0))
                break;
        }
        return grounded;
    }

    //��������, ����� ���� �� ��������� ������ ���� �����
    //��� ����� ����� ��������, ����� �� ����� ����� �� ��������� �� ���� �����
    if (bodySpeed_arg.y != 0 && bodySpeed_arg.x!=0)
    {
        if (bodySpeed_arg.y>0) //�������� ����
        {
            if ( bodySpeed_arg.x>0 ) //�������� ���� ������
            {
                {
                    //��������� ������� ������ ����
                    Vector2f posUpCorner = getCornerCoords(body_arg, CornerOfRect::RightUp);
                    Vector2u currentUpCornerTile = map_arg.CoordsToIndex(posUpCorner),
                            newUpCornerTile = map_arg.CoordsToIndex(posUpCorner+moveVector);
                    if ( (currentUpCornerTile.x!=newUpCornerTile.x) && (currentUpCornerTile.y != newUpCornerTile.y) )
                    {
                        //�.�. ���� ��������� � ����� ���� �� ������ ����/����, � �� ���������
                        if (map_arg.at(currentUpCornerTile.x+1, newUpCornerTile.y)!= nullptr
                            && map_arg.at(currentUpCornerTile.x+1, newUpCornerTile.y)->solid_m
                            && (DotPositionRelativeToVector( map_arg.IndexToCoord(newUpCornerTile), posUpCorner, moveVector )>0))
                        {
                            //������� � ��� ��������!
                            //������ ���������� ����������� �����, ����� ��� ��� ���������� ���� ��������� � ����, � �� ������������ ���
                            moveVector.x = map_arg.IndexToCoord(newUpCornerTile).x - posUpCorner.x;
                        }
                    }
                }
                {
                    //������ -����� ������ ����
                    Vector2f posDLCorner = getCornerCoords(body_arg, CornerOfRect::LeftDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(posDLCorner),
                            newCornerTileIndex = map_arg.CoordsToIndex(posDLCorner+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y+1);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(newCornerTileIndex), posDLCorner, moveVector )>0)
                        {
                            //������������ �����
                            moveVector.y = minByAbs(map_arg.IndexToCoord(newCornerTileIndex).y - posDLCorner.y, moveVector.y );
                            moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                            grounded=true;
                        }
                    }
                }
            }
            else //�������� ���� �����
            {
                {//��������� �� ������� ����� �����
                    Vector2f cornerULCoord = getCornerCoords(body_arg, CornerOfRect::LeftUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerULCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerULCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x-1, cornerTileIndex.y);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(newCornerTileIndex.x+1, newCornerTileIndex.y)
                                                           , posDLCorner, moveVector )<0)
                        {
                            //������������ �����
                            moveVector.x = minByAbs (moveVector.x, map_arg.XCoordToIndex(cornerTileIndex.x));
                        }
                    }
                }
                {//�� ������ ������
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
                            //������������ �����
                            moveVector.y = minByAbs(moveVector.y, map_arg.YCoordToIndex(newCornerTileIndex.y) - cornerDRCoord.y);
                            moveVector.x = minByAbs( moveVector.x, origMoveVector.x*(origMoveVector.y/moveVector.y) );
                            grounded=true;
                        }
                    }
                }
            }
        }
        else //�������� �����
        {
            if ( bodySpeed_arg.x>0 ) //�������� ����� ������
            {
                {//����� �������
                    Vector2f cornerULCoord = getCornerCoords(body_arg, CornerOfRect::LeftUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerULCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerULCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            && DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex.x+1, cornerTileIndex.y)
                                                           , cornerDRCoord, moveVector )<0)
                        {
                            //������������ �� �������
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord(cornerTileIndex.y) - cornerULCoord.y );
                        }
                    }
                }
                {//������ ������
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
                            //������������ �� ������ ������
                            moveVector.x = minByAbs(moveVector.x
                                                    , map_arg.XIndexToCoord(cornerTileIndex.x+1) - cornerDRCoord);
                        }
                    }
                }
            }
            else //�������� ����� �����
            {
                {//������� ������
                    Vector2f cornerURCoord = getCornerCoords(body_arg, CornerOfRect::RightUp);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerURCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerURCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            &&DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex)
                                                          , cornerDRCoord, moveVector )<0)
                        {
                            //������������ �� �������
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord(cornerTileIndex.y) - cornerURCoord.y );
                        }
                    }
                }
                {//������ �����
                    Vector2f cornerDLCoord = getCornerCoords(body_arg, CornerOfRect::LeftDown);
                    Vector2u cornerTileIndex = map_arg.CoordsToIndex(cornerDLCoord),
                            newCornerTileIndex = map_arg.CoordsToIndex(cornerDLCoord+moveVector);
                    if( (cornerTileIndex.x != newCornerTileIndex.x)&&(cornerTileIndex.y!= newCornerTileIndex.y) )
                    {
                        Tile* tl = map_arg.at(cornerTileIndex.x, cornerTileIndex.y-1);
                        if (tl!=nullptr && tl->solid_m
                            &&DotPositionRelativeToVector(map_arg.IndexToCoord(cornerTileIndex)
                                                          , cornerDRCoord, moveVector )>0)
                        {
                            //�����
                            moveVector.x = minByAbs(moveVector.x, map_arg.XIndexToCoord(CornerTileIndex.x) - cornerDLCoord.x);
                        }
                    }
                }
            }
        }
    }//����� �����, ��������������� ������������ �� ���� ��� �������� �� ���������
    //���� �����, �� ����������� ������ 1/4 �����, � �� � ������

    //����� ������������ �� ����� ������
    {
        // �����. ������������
        {
            Vector2u upperTile, downTile;

            if (moveVector.x>0)
            {
                Vector2u upperTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::RightUp  )+moveVector);
                Vector2u downTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::RightDown)+moveVector);
            }
            else
            {
                Vector2u upperTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::LeftUp  )+moveVector);
                Vector2u downTile = map_arg.CoordsToIndex(getCornerCoords(body_arg, CornerOfRect::LeftDown)+moveVector);
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

                for (Uint16 i = upperTile.y+1; i < downTile.y; ++i) //�������� y ���������� "����"
                {
                    currentTileI = Vector2u (upperTile.x, i);
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
                                            - (moveVector.x>0 ) ?  (body_arg.left+body_arg.width) : body_arg.left )
                }
            }
        }//����� ��������� �����. ������������

        //������������ ������������
        {
            Vector2u leftTileI, rightTileI;
            if (moveVector.y>0)
            {//����
                leftTileI =  map_arg.CoordsToIndex(  Vector2f(body_arg.left,                body_arg.top+body_arg.height)+moveVector );
                rightTileI=  map_arg.CoordsToIndex(  Vector2f(body_arg.left+body_arg.width, body_arg.top+body_arg.height)+moveVector );
            }
            else
            {//�����
                leftTileI =  map_arg.CoordsToIndex(  Vector2f(body_arg.left,                body_arg.top)+moveVector );
                rightTileI=  map_arg.CoordsToIndex(  Vector2f(body_arg.left+body_arg.width, body_arg.top)+moveVector );
            }
            if (leftTileI!=rightTileI)
            {
                if ( map_arg.XIndexToCoord(leftTileI+1) >= body_arg.left
                    && map_arg.at(leftTileI)!=nullptr && map_arg.at(leftTileI)->solid_m )
                {
                    if (moveVector.y>0)
                    {
                        //������������ �����
                        grounded=true;

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y ) - (body_arg.top+body_arg.height);
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        //���� �� �������
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y+1 ) - (body_arg.top);
                    }
                }
                for (Uint16 i = leftTileI.x+1; i<rightTileI.x; i++)
                {
                    Vector2u currentTileI (i, leftTileI.y);
                    if (map_arg.at (currentTileI)!=nullptr && map_arg.at(currentTileI)->solid_m )
                    {
                        if (moveVector.y>0)
                        {
                            //�� ���
                            grounded=true;
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( currentTileI.y ) - (body_arg.top+body_arg.height);
                            moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                        }
                        else
                        {
                            //�� �������
                            moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( currentTileI.y+1 ) - (body_arg.top);
                        }
                    }
                }
                if (map_arg.XIndexToCoord(rightTileI.x) <= (body_arg.left+body_arg.width)
                    && map_arg.at(rightTileI)!=nullptr && map_arg.at(rightTileI)->solid_m )
                {
                    if (moveVector.y>0)
                    {
                        //������������ �����
                        grounded=true;

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( rightTileI.y ) - (body_arg.top+body_arg.height);
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( rightTileI.y+1 ) - (body_arg.top);
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

                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y ) - (body_arg.top+body_arg.height);
                        moveVector.x = minByAbs( (origMoveVector.x*(origMoveVector.y/moveVector.y)), moveVector.x );
                    }
                    else
                    {
                        moveVector.y = minByAbs(moveVector.y, map_arg.YIndexToCoord( leftTileI.y-1 ) - (body_arg.top);
                    }
                }
            }

        }
    }//����� ���� ������� ������ ������������

    //��� �������������� "������ �� ����" �� ���������, ���� ���� �� ����������� �� � ��� �� ��� ���
    if (origMoveVector == moveVector)
    {
        // <<------------ ���������� ���!! ----------------->>






        // <<------------ ���������� ���!! ----------------->>







        // <<------------ ���������� ���!! ----------------->>

    }
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

        currentAnimFrame_m+=animSpeed_m*frameTime_arg.asMicroseconds();

        //����� ������� �������������� � ����� ������������ �������� - �����

        //������� ������� - ����� �������� �� ��������, ����� ���������� �����
        //���� ���� �������� ����� - ���������� �������������� ��������
        //����� - ������������, � "������", ������ - ���������� ��������
        //�� ��������� - ��� ������ ��� ��������� �������
        //� ��� ��������-��, �� ��� �� �����! ������� ������������� ���������
        //��� ���� ���������� ����������� ������������ ��������: ������������ ����
        //������� � ����� ����� � ������� ��������



        //apply state-specific things
        if (state_m == State_m::standing)
            speed_m.x = 0;

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
        //�������� ������ � �����. � ���������� ��������� � ������


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
    //�������� ������ �������� ���� ������� �� �������� ������ ���� ��������
    float currentAnimFrame_m=0;

    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;

    bool lookingLeft_m      = false; //� ��������� ������ ������� ������

    Vector2f speed_m        = Vector2f(0,0);

    //�������� - � �������� � ������������
    float jumpingSpeed_m        = 0.0018; //������������ ��������, ������� ��� ��������� ��� ������
    float walkingSpeed_m        = 0.0006;  //��������, � ������� �� �����
    float animSpeed_m           = 0.00001; // ���� ������ � ������������
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;

    Tileset2d* locationMap_m    = nullptr;
    //Entity, � ����� vector<Entity*> *environment_m, ������� ��������
    //����� �������������� �����, ��� �������������� � �����
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
    Tileset2d levelTiles(38-14,11);
    {
        char * levelCheme[] =
        {
            "ssssssssssssssssssssssss",
            "s                      s",
            "s                      s",
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
    sprt.setPosition(30, 40);
    FloatRect FangCollizion = sprt.getGlobalBounds();
    PlayableCharacter Fang(&FangCollizion ,&sprt);
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
