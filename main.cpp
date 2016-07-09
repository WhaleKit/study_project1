#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <cmath>

#include <map>

using namespace std;
using namespace sf;

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
    char ** location_m;
};

float floorLevel = 600;

class Tile
{
public:
    Tile(bool solid_arg)
        :solid_m(solid_arg)
        {    }
    Tile(){}

    bool solid_m = true;
    sf::RectangleShape* drawableComponent;

    typedef void (*effectFunction)(Entity*, const Vector2f&);

    effectFunction onInteract_m = nullptr; //вызывается при прикосновении/коллизии ентитей, если не равна нулю
};

class PlayableCharacter : public Entity
{
public:
    PlayableCharacter(FloatRect* collizion_arg, Sprite* renderComponent_arg)
    : collizion_m(collizion_arg), renderComponent_m(renderComponent_arg)
    {    }

    ~PlayableCharacter()
    {

    }

    void readApplyUserInput(sf::Time lastFrameTime_arg)
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
        readApplyUserInput(frameTime_arg);

        currentAnimFrame_m+=animSpeed_m*frameTime_arg.asMicroseconds();

        collizion_m->left += speed_m.x * frameTime_arg.asMicroseconds();

        if (state_m == State_m::standing)
            speed_m.x = 0;

        collizion_m->top += speed_m.y * frameTime_arg.asMicroseconds();

        if (!onGround_m)
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

    bool onGround_m         = false;

    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;

    bool lookingLeft_m      = false; //в противном случае смотрит вправо

    Vector2f speed_m        = Vector2f(0,0);

    float jumpingSpeed_m        = 0.0018; //вертикальная скорость, которая ему придается при прыжке
    float walkingSpeed_m        = 0.0006;  //скорость, с которой он ходит
    float animSpeed_m           = 0.00001; // смен кадров в микросекунду
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;
    //Entity, с полем vector<Entity*> *environment_m, которая родитель
    //будет использоваться позже, для взаимодействия с миром
};

int main()
{

    //tiles
    constexpr Uint8 tileSize = 100;
    Tile stone;
    stone.solid_m = true;
    sf::RectangleShape stoneTileShape(Vector2f(tileSize, tileSize));
    stoneTileShape.setFillColor(sf::Color( 200, 200, 200));
    stone.drawableComponent = &stoneTileShape;

    //levelMap
    constexpr Uint16 levelHight = 9;
    constexpr Uint16 levelWidth = 19;
    Tile* levelTiles[levelWidth][levelHight];
    {
        char * levelCheme[] =
        {
            "sssssssssssssssssss",
            "s                 s",
            "s                 s",
            "s                 s",
            "s                 s",
            "s                 s",
            "s       sssssss   s",
            "s             s   s",
            "ssss   ssssssssssss"
        };

        for (int x=0; x<levelWidth; ++x)
            for (int y=0; y<levelHight; ++y)
        {
            switch (levelCheme[y][x])
            {
            case 's':
                levelTiles[x][y] = &stone;
                break;
            default:
                levelTiles[x][y] = nullptr;
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
        window.setView(window.getDefaultView());
        window.draw(shape);
        myCamera.setCenter(Vector2f (Fang.collizion_m->left +Fang.collizion_m->width/2,
                                       Fang.collizion_m->top+Fang.collizion_m->height/2) );
        window.setView(myCamera);
        for (Entity* toDraw : entitiesOnLevel)
        {
            window.draw( *(toDraw->getDrawableComponent()) );
        }
        for (int x=0; x<levelWidth; ++x)
            for (int y=0; y<levelHight; ++y)
        {
            if (levelTiles[x][y] !=nullptr)
            {
                levelTiles[x][y]->drawableComponent->setPosition(tileSize*(x), tileSize*(y));
                window.draw( * (levelTiles[x][y]->drawableComponent) );
            }
        }

        window.display();
    }
    return 0;
}
