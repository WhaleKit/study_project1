#include <iostream>
#include <cstdlib>
#include <cmath>

#include <utility>

#include <vector>

#include <cassert>
#include <limits>

#include <algorithm>
#include <numeric>

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


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


constexpr float gravityAcceleration = 4*0.98;
template<typename T>
T minByAbs(T arg1, T arg2 )
{
    return min(arg1, arg2, [](T t1, T t2)->bool{return abs(t1)<abs(t2);} );
}

float vectorTan (Vector2f const& vec_arg)
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
        assert(false); //"������, � �������� ��������� 5-� �������"
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
        assert(false); //"������, � �������� ��������� 5-� �������"
    }
}
inline IntRect HorizontalFlip(IntRect const& ir_arg)
{
    return IntRect(ir_arg.left+ir_arg.width ,ir_arg.top, -ir_arg.width ,ir_arg.height);
}

//����� ������������ �������� �� �����������, ���������� TextureRect �� ��������� �������� � ������������ � ������� ������
//��� ������ ��������� �������. �������� � ����������, ����������� ��������� �������� ��������, � ���������, �������,
//�.�. ����� ���� ����������� ��� ����������� drawable, ��� � ��� �������� �������� �������
//(�� ������������ � �������, �� �� ����������� ���)
class Animator : public Drawable
{
public:
    typedef IntRect (*TextureRectUpdater) (float, void*);

    Animator (Sprite* sprt_arg=nullptr, float animSpeed_arg = 0.000001
              , Uint16 framesLimit_arg=1, TextureRectUpdater updaterFunc_arg=nullptr
              , void* additionalDataPtr_arg=nullptr)
    :   sprt_m(sprt_arg), animSpeed_m(animSpeed_arg), currentAnimFrame_m(0)
        ,framesLimit_m(framesLimit_arg), textureRectUpdaterFunction_m(updaterFunc_arg)
        ,ptrToAdditionalDataForTextureRectUpdaterFunction_m(additionalDataPtr_arg)
    {}

    ~Animator () {}

    inline void timePassed(Time time_arg)
    {
        //�������� ������� ���� � ������������ � ��������� ��������
        currentAnimFrame_m = (currentAnimFrame_m + time_arg.asMicroseconds()*animSpeed_m);
        if (currentAnimFrame_m>=framesLimit_m)
            currentAnimFrame_m-=framesLimit_m;

        updateSprite();
    }
    inline void updateSprite ()
    {
        IntRect  ir= textureRectUpdaterFunction_m(currentAnimFrame_m
                                                , ptrToAdditionalDataForTextureRectUpdaterFunction_m );
        sprt_m->setTextureRect( ir );
        if (facingLeft_m)
            sprt_m->setTextureRect(HorizontalFlip( sprt_m->getTextureRect() ));
    }
    inline void setFacingLeft(bool arg)
        {facingLeft_m=arg;}
    void draw (RenderTarget& renTr_arg, RenderStates states_arg) const override
    {
        //�������� ������ � �����. � ������� ������
        renTr_arg.draw(*sprt_m, states_arg);
    }
    void SetSpritePtr (Sprite* newSprite_arg)
    {
        sprt_m = newSprite_arg;
    }
    Sprite* GetSpritePtr( )
    {
        return sprt_m;
    }
    void ResetAnim ()
    {
        currentAnimFrame_m=0;
    }
    void setAnimSpeed (float newSpeed_arg)
    {
        animSpeed_m = newSpeed_arg;
    }
    float getAnimSpeed ()
    {
        return animSpeed_m;
    }
    Uint16 getAnimFrameLimit ()
    {
        return framesLimit_m;
    }
    void setAnimFrameLimit (Uint16 newFrameLimit_arg)
    {
        framesLimit_m = newFrameLimit_arg;
    }
    void setTextureRectUpdaterFunctionAndData (TextureRectUpdater newTRU_arg, void* newData_arg = nullptr)
    {
        textureRectUpdaterFunction_m = newTRU_arg;
        ptrToAdditionalDataForTextureRectUpdaterFunction_m = newData_arg;
    }
    tuple<TextureRectUpdater, void*> getTextureRectUpdaterFunctionAndData ()
    {
        return make_tuple(textureRectUpdaterFunction_m
                          ,ptrToAdditionalDataForTextureRectUpdaterFunction_m);
    }

private:
    bool facingLeft_m=false;
    Sprite* sprt_m=nullptr;
    float animSpeed_m=0.000001; //���� ������ � ������������
    float currentAnimFrame_m=0;
    Uint16 framesLimit_m=1;
    TextureRectUpdater textureRectUpdaterFunction_m = nullptr;
    void * ptrToAdditionalDataForTextureRectUpdaterFunction_m = nullptr;
    //��� ���� �� ������� ����� ���� �������� �� ����-�� ��������? � ��� �� ������������ std_function, �� �� ������� ��������� ��� ���
    //��� ��� � ������� ���������� ��������� �� ���_����_����������_�����_������, ��� nullptr, ���� ��� � ��� �� ���������
};
//��, � ��� �� ������� animator ���������� �������, c ����������� update
// �� ����� ��� �������� ��� ����� ��������� � ��� �������� �� ���������
//��� ������, � � �� ����� ������������� �� ���������� �������
//��� � ������ 1 ����� "�����" �� 1 ���������, �� ������� �������������� ��������


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

float floorLevel = 1205;


//������ ��� ����� � ��� ����� ������������ �� ������
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
        if (x_arg<width_m /*&& x_arg>=0*/ && y_arg<height_m /*&& y_arg>=0*/)
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
    Vector2f startOffset_m = Vector2f(0,0); //� �� ���� ��������� ���� ������� Tileset2d �������� �������� � ������ ���������
    //������������ ������
    vector<Tile*> content_m;
    Uint16 height_m;
    Uint16 width_m;
};




//������� ��������, ������ ���� ������ �� ������ �������-��������� �� ����� � ������-�������� ��������� � ����� ��������
//������ ���� - ����� ����, ������ ���� - ����, 0 - �� ���
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


bool standingOnGround (Tileset2d const& map_arg, FloatRect & body_arg, float soleThickness)
{

}

//������ ������� ������ �������� � ��������� ����, �� �� ��� ��������
//���������� true, ���� � ����� ������ ��� ������� � ���� ���� �����, false - ���� ������� ���
bool MoveTroughtTilesAndCollide(Tileset2d const& map_arg, FloatRect & body_arg, Vector2f & bodySpeed_arg, Time time_arg)
{

    const Vector2f origMoveVector(bodySpeed_arg.x * time_arg.asMicroseconds(), bodySpeed_arg.y * time_arg.asMicroseconds() );
        Vector2f moveVector(bodySpeed_arg.x * time_arg.asMicroseconds(), bodySpeed_arg.y * time_arg.asMicroseconds());

        //�������������� ��� ������������ ����� � �������� ������ ����� � ������������ ������ �����
        //���� ���������� � ��� ����� ������������� ��� � ���������� ���������� ��������
        //���� � ���, ��� ������� �� ����� ����� ���� ���������� ��-�� ����, ��� ���� ���������� ����, �� ������������
        //f8 300 ���, ����� ��������� �� ���� �����, ��� ��� ���������������... ����� ������� ��������� ��������
        //� ����� �������������� � �����
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


    if ((moveVector.x == 0)  && (abs(moveVector.y) <= epsiFraction*abs(moveVector.y)))
    {
        //��������� ������� ����� ��� ������
        bool thereIsFooting = false;
        {

            Vector2f dot = getCornerCoords(body_arg, CornerOfRect::LeftDown);
            dot +=  abs(dot)*Vector2f(epsiFraction, 2*epsiFraction);
            Vector2u leftTile = map_arg.CoordsToIndex(dot);

            dot = getCornerCoords(body_arg, CornerOfRect::RightDown) + Vector2f(-epsiFraction, 2*epsiFraction);
            dot += abs(dot)*Vector2f(-epsiFraction, 2*epsiFraction);
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
        //���� �������� ������� �������/����� ��������� - ��������� �������� �� ����� � ���������� �� ������
        //��� �� �� �����, ����� ���� �� �������� ��������� ������ �����?
        bool grounded = false;

        int stageCount = ceil( max( abs(moveVector.x) , abs(moveVector.y) )/ static_cast<float>(map_arg.tileSize));
        for (int i = 0; i<stageCount; i++)
        {
            //����������� ������� - �� ������ 1, �.�. � ������ �������� �� �������������� ���������� ����� �����
            //������ ����, ��� ���� ������ ����� ���� �����
            if (MoveTroughtTilesAndCollide(map_arg, body_arg, bodySpeed_arg, time_arg/static_cast<Int64>(stageCount) ))
                grounded=true;
            if (bodySpeed_arg==Vector2f(0,0))
                break;
        }
        return grounded;
    }

    bool collidedVertically = false, collidedHorizontally = false;

    //�� ����� ������, � ����� ������ ����������, ��� �������, ������� ��� ���������, ��������� � ������
    static auto finish = [&]()mutable-> bool
        {
            bool grounded = false;
            if (collidedHorizontally)
            {
                if(moveVector.x>0)
                {
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex( LittleLessThan(body_arg.left+body_arg.width,2) ) +1 );
                    moveVector.x = wallX - LittleMoreThan(body_arg.left+body_arg.width) ;

                    moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
                }
                else
                {
                    float wallX = map_arg.XIndexToCoord(  map_arg.XCoordToIndex( LittleMoreThan(body_arg.left) ) );
                    moveVector.x = wallX - LittleLessThan(body_arg.left);

                    moveVector.y = minByAbs(moveVector.y, moveVector.y * (moveVector.y / origMoveVector.y)  );
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

                    moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
                }
                else
                {
                    float ceilingY = map_arg.YCoordToIndex( map_arg.YIndexToCoord(LittleMoreThan(body_arg.top,2) ));
                    moveVector.y = ceilingY - LittleMoreThan(body_arg.top);

                    moveVector.x = minByAbs(moveVector.x, moveVector.x * (moveVector.x / origMoveVector.x)  );
                }
                bodySpeed_arg.y=0;
            }
            //���������, ����� �� ���� �� �����
            if ( abs(moveVector.y) <= epsiFraction*abs(moveVector.y) )
            {
                Vector2f dot = getCornerCoords(body_arg, CornerOfRect::LeftDown);
                dot +=  abs(dot)*Vector2f(epsiFraction, 2*epsiFraction);
                Vector2u leftTile = map_arg.CoordsToIndex(dot);

                dot = getCornerCoords(body_arg, CornerOfRect::RightDown) + Vector2f(-epsiFraction, 2*epsiFraction);
                dot += abs(dot)*Vector2f(-epsiFraction, 2*epsiFraction);
                Uint16 rightTileX = map_arg.CoordsToIndex(dot).x;

                for (int x = leftTile.x; x<=rightTileX; x++)
                {
                    if (map_arg.at(x, leftTile.y)!=nullptr && map_arg.at(x, leftTile.y)->solid_m )
                    {
                        grounded = true;
                        break;
                    }
                }
            }//�� �� ����� ����� �� �����, ����� ����� � ������/����� ��� ������ ���������?

            body_arg.left += moveVector.x;
            body_arg.top  += moveVector.y;
            return grounded;
        };

    //��� ������� ����� ����������, ����� � �������� ������������
    //� �� ���� ������� ��� ����� ������, �.�. ��� ��� � ������� �� ������� ��� ���. ��������

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

    // ��� ������ ���������� �����
    //������������ ������������ �� ���� ����� ��������
    if(  (moveVector.x!=0) && (moveVector.y!=0) )
    {
        Vector2f upperMovingDot, lowerMovingDot,
                   upperEpsiVec, lowerEpsiVec;

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

        //���� ������������� �������� ���� ����� ��� ����� ������ - �� ����, ������� ����� �� �������� - ��� ������� ����� ��� ������ ������
        //� bool-��� == �������� ��� NXOR
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
        //���� ����� ���� �� ���� (������� ��������� ��� ������� �� ������)
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
        //������ ���� ������������ ������ �����
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
    }//�-�-�, ��������� ������ ������������ �����

    //��� - ������ �����
    //������ ���� ������ ������������


    //��������������
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
            //��������� �� ��� ����� �������������� ��������, ������ ��� ������ ������
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


    //������ ��� �� �� �����, �� �� ���� ������ ������������ ��������
    if(crossedVerticalBoundary)
    {
        float ycoordOfBody = moveVector.y>0? (body_arg.top+body_arg.height) : (body_arg.top);

        Vector2u leftTile  = map_arg.CoordsToIndex( LittleMoreThan(body_arg.left+moveVector.x),ycoordOfBody+moveVector.y );
        Vector2u rightTile = map_arg.CoordsToIndex( LittleLessThan(body_arg.left+body_arg.width+moveVector.x)
                                                   ,ycoordOfBody+moveVector.y );

        // ���� �������/������ ���� ��������� ���� ���������, �� ��� ���� ������ ������ - ��� ��� ����
        // "������ �� ����" � "����� �����"
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

    //��� ������ �����
    //������ ������������ "������ �� ����"
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


    //��� �������
    FINISH()

#undef COLLIDE_HOR
#undef COLLIDE_VERT
#undef FINISH
}



class PlayableCharacter : public Entity
{
public:
    PlayableCharacter(FloatRect* collizion_arg, Sprite* renderComponent_arg)
    :   collizion_m(collizion_arg), renderComponent_m(renderComponent_arg)
        ,walkingAnimation_m(renderComponent_arg, 0.000007, 4
                        ,[]( float currentFrame_arg, void* data_arg )->IntRect
                             {
                                return IntRect(108*int(currentFrame_arg), 364, 108 , 182) ;
                             }
                         ,nullptr )
        ,idleAnimation_m(renderComponent_arg, 0.000001, 5
                        ,[]( float currentFrame_arg, void* data_arg )->IntRect
                            {
                                if(currentFrame_arg>2)
                                    return IntRect(216+108*int(5-currentFrame_arg), 0, 108 , 182);
                                else
                                    return IntRect(216+108*int(currentFrame_arg), 0, 108 , 182);
                            }
                         ,nullptr)
    {    }

    ~PlayableCharacter()
    {

    }

    void readApplyUserInput()
    {
        //��� ������ �� ��� ������� ������������� switch
        if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A)!=Keyboard::isKeyPressed(Keyboard::D) )
        {
            if (state_m != State_m::inAir)
            {
                if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A) )
                {
                    speed_m.x = -walkingSpeed_m;
                    facingLeft_m = true;
                }
                else
                {
                    speed_m.x = walkingSpeed_m;
                    facingLeft_m = false;
                }
                state_m = State_m::walking;
            }
            else
            {
                if ( sf::Keyboard::isKeyPressed(sf::Keyboard::A) )
                {
                    speed_m.x = -walkingSpeed_m*airControl_m;
                    facingLeft_m = true;
                }
                else
                {
                    speed_m.x = walkingSpeed_m*airControl_m;
                    facingLeft_m = false;
                }
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

    void SelectPropperAnimation()
    {

        switch ( state_m)
        {
        case State_m::standing:
            if (weaponState_m==WeaponState_m::noWeapon)
                currentAnimation_m = &idleAnimation_m;
            else
                ;//todo
            break;
        case State_m::walking:
            if (weaponState_m==WeaponState_m::noWeapon)
                currentAnimation_m = &walkingAnimation_m;
            else
                ;//todo
            break;
        }

        currentAnimation_m->setFacingLeft(facingLeft_m);
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
        */
        currentAnimation_m->timePassed(frameTime_arg);
        //temp_frame_m+=frameTime_arg.asMicroseconds()*0.000004;
        //


        //apply state-specific things
        if (state_m == State_m::standing)
        {
            speed_m.x = 0;
        }

        if (state_m!=State_m::inAir)
            speed_m.y=0;

        if (state_m == State_m::inAir)
        {
            speed_m.y += frameTime_arg.asMicroseconds()* gravityAcceleration/(1000000000);
        }


        //��������� ������ �������� �����
        bool onGround = MoveTroughtTilesAndCollide(*locationMap_m, *collizion_m, speed_m, frameTime_arg);

        if (onGround && state_m==State_m::inAir)
        {
            if (speed_m.x!=0)
                state_m = State_m::walking;
            else
                state_m = State_m::standing;
        }
        if (!onGround && state_m!=State_m::inAir)
        {
            state_m = State_m::inAir;
        }


        //���������� ���
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
        SelectPropperAnimation();
    }

    sf::Drawable* getDrawableComponent() override
    {
        //�������� ������ � �����. � ���������� ��������� � ������


        renderComponent_m->setPosition( Vector2f(collizion_m->left, collizion_m->top)
                                        +spriteCoordRelativeToCollision_m
                                      );
        /*
        if(facingLeft_m)
            currentAnimation_m->setFacingLeft(facingLeft_m);

        if (state_m==State_m::walking)
        {
            if (temp_frame_m>4)
                temp_frame_m-=4;
            renderComponent_m->setTextureRect( IntRect(108*int(temp_frame_m), 364, 108 , 182) );
            if (facingLeft_m)
                renderComponent_m->setTextureRect( HorizontalFlip( renderComponent_m->getTextureRect() ) );
        }

        if (state_m==State_m::standing)
        {

            if (temp_frame_m>5)
                temp_frame_m-=5;
            if(temp_frame_m>2)
                renderComponent_m->setTextureRect(  IntRect(216+108*int(5-temp_frame_m), 0, 108 , 182)  );
            else
                renderComponent_m->setTextureRect(  IntRect(216+108*int(temp_frame_m), 0, 108 , 182)  );
            if (facingLeft_m)
                renderComponent_m->setTextureRect( HorizontalFlip( renderComponent_m->getTextureRect() ) );
        }*/

        return renderComponent_m;
    }

    Vector2f spriteCoordRelativeToCollision_m = Vector2f(0,0);
    //�������� ������ �������� ���� ������� �� �������� ������ ���� ��������

    //\
    float temp_frame_m=0;
    //
    Animator walkingAnimation_m;
    Animator idleAnimation_m;
    //todo ��� 2 ��������

    Animator* currentAnimation_m=&idleAnimation_m;
    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;
    enum class WeaponState_m {noWeapon, pistol};
    WeaponState_m weaponState_m = WeaponState_m::noWeapon;
    bool facingLeft_m       = false; //� ��������� ������ ������� ������

    Vector2f speed_m        = Vector2f(0,0);

    //�������� - � �������� � ������������
    float jumpingSpeed_m        = 0.002; //������������ ��������, ������� ��� ��������� ��� ������
    float walkingSpeed_m        = 0.001;  //��������, � ������� �� �����
    float airControl_m          = 0.2;      //� ����� ��������� ������������ ������� PC ����� ��������� � �������
    float animSpeed_m           = 0.00001; // ���� ������ � ������������
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;

    Tileset2d* locationMap_m    = nullptr;
    //Entity, � ����� vector<Entity*> *environment_m, ������� ��������
    //����� �������������� �����, ��� �������������� � �����
};

/*
//�������, ������� � ��������
int main_()
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
            "s          ssss        s",
            "s             s        s",
            "ssss sssssssssssssssssss"
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

    FloatRect fr (2220, 300.05, 80, 100);

    Vector2f speed (0.001, -0.00199897  );
    Time time = microseconds(264);
    while (true)
    {
        ;
        MoveTroughtTilesAndCollide(levelTiles, fr, speed, time);
        cout << "f";
    }
}
*/




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
            "s                      s",
            "s   ssssssssssssssssssss",
            "s                      s",
            "s   sssss              s",
            "s                      s",
            "s         sssss        s",
            "s             s        s",
            "s          ssss        s",
            "s             s        s",
            "ssss sssssssssssssssssss"
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
    myTxtr.loadFromFile("spriteSheet.png");
    sf::Sprite sprt;
    sprt.setTexture(myTxtr);
    sprt.setTextureRect( IntRect(0, 364, 108 , 182) );
    sprt.setScale(0.7,0.7);
    sprt.setPosition(-300, 100);
    FloatRect FangCollizion = sprt.getGlobalBounds();
    FangCollizion.height-=FangCollizion.height*0.0001; //��� ����� �� ��� ��������� � ������� ������� � ����
    PlayableCharacter Fang(&FangCollizion ,&sprt);
    Fang.locationMap_m = &levelTiles;
    Fang.state_m = PlayableCharacter::State_m::inAir;

    //level content ������������ ������
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
    static const Time minFrameTime = seconds(1/480); //120 fps should be enought
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

        if (frameTime<minFrameTime)
        {
            sleep(minFrameTime-frameTime);
        }
    }
    return 0;
}
