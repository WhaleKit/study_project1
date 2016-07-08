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

    virtual sf::Drawable* getDrawableComponent()=0;//������ ���������� nullptr, ���� �� ������������ ��� ���������

    vector<Entity*> *environment_m;
};

float floorLevel = 600;

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
            if (currentAnimFrame_m>4)
                currentAnimFrame_m = fmod (currentAnimFrame_m, 4.0f);

            if (currentAnimFrame_m>3.0)
            {
                if (lookingLeft_m)
                    renderComponent_m->setTextureRect(IntRect(44+44*int(5.0f-currentAnimFrame_m), 189, -44, 50));
                else
                    renderComponent_m->setTextureRect(IntRect(44*int(5.0f-currentAnimFrame_m), 189, 44, 50));
            }

            if (lookingLeft_m)
                renderComponent_m->setTextureRect(IntRect(44+44*int(currentAnimFrame_m), 189, -44, 50));
            else
                renderComponent_m->setTextureRect(IntRect(44*int(currentAnimFrame_m), 189, 44, 50));
        }


        return renderComponent_m;
    }

    Vector2f spriteCoordRelativeToCollision_m = Vector2f(0,0);
    //�������� ������ �������� ���� ������� �� �������� ������ ���� ��������
    float currentAnimFrame_m=0;

    bool onGround_m         = false;

    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;

    bool lookingLeft_m      = false; //� ��������� ������ ������� ������

    Vector2f speed_m        = Vector2f(0,0);

    float jumpingSpeed_m        = 0.0018; //������������ ��������, ������� ��� ��������� ��� ������
    float walkingSpeed_m        = 0.0006;  //��������, � ������� �� �����
    float animSpeed_m           = 0.00001; // ���� ������ � ������������
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;
    //Entity, � ����� vector<Entity*> *environment_m, ������� ��������
};

int main()
{

    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!", sf::Style::Close);
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Texture myTxtr;
    myTxtr.loadFromFile("fang.png");

    sf::Sprite sprt;
    sprt.setTexture(myTxtr);
    sprt.setTextureRect( IntRect(0, 244, 40, 50) );
    sprt.setScale(3,3);
    sprt.setPosition(30, 40);

    FloatRect FangCollizion = sprt.getGlobalBounds();

    PlayableCharacter Fang(&FangCollizion ,&sprt);


    Clock clc; clc.restart();

    vector<Entity*> entitiesOnLevel;

    entitiesOnLevel.push_back(&Fang);

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

        window.clear();

        window.draw(shape);

        for (Entity* toDraw : entitiesOnLevel)
        {
            window.draw( *(toDraw->getDrawableComponent()) );
        }

        window.display();
    }
    return 0;
}
