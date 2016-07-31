#ifndef PLAYABLECHARACTER_H
#define PLAYABLECHARACTER_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>


#include "Tileset2d.h"
#include "fpoint_tweaks.h"
#include "Entity.h"
#include "GeometryFunctions.h"
#include "EntityContainer.h"

using namespace std;
using namespace sf;

constexpr float gravityAcceleration = 4*0.98;
constexpr float footingDistance = 0.001;

float floorLevel = 1205;
class PlayableCharacter : public Entity
{
public:
    PlayableCharacter(FloatRect* collizion_arg, Sprite* renderComponent_arg, EntityContainer* environment_arg=nullptr)
    :   Entity(environment_arg), collizion_m(collizion_arg), renderComponent_m(renderComponent_arg)
        ,walkingAnimation_m(renderComponent_arg, 0.000007, 4
                        ,[]( float currentFrame_arg, void* data_arg )->IntRect
                             {
                                return IntRect(108*int(currentFrame_arg), 364, 108 , 182);
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
         ,walkingShootingAnimation_m(renderComponent_arg, 0.000007, 4
                         ,[](float currentFrame_arg, void* data_arg)->IntRect
                             {
                                 return IntRect(108*int(currentFrame_arg), 182, 108 , 182);
                             }
                         ,nullptr)
        ,stayShootingAnimation_m(renderComponent_arg, 0.000007, 2
                         ,[](float currentFrame_arg, void* data_arg)->IntRect
                             {
                                 return IntRect(111*int(currentFrame_arg), 0, 108 , 182);
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
                    facingLeft_m = true;

                    //����� ���������� � ������� �� ������ �������� ������
                    //�� ���� �� ��� ��������� - �� �� �����������, �� � ������� �� �����������
                    if(-speed_m.x < walkingSpeed_m)
                    {
                        speed_m.x -= airAcceleration_m;
                    }
                }
                else//������ D
                {
                    if(speed_m.x<walkingSpeed_m)
                    {
                        speed_m.x += airAcceleration_m;
                    }
                    facingLeft_m = false;
                }
            }
        }
        else if (state_m != State_m::inAir)
        {
            state_m = State_m::standing;
        }
        if  (   ((sf::Keyboard::isKeyPressed(Keyboard::W))||(sf::Keyboard::isKeyPressed(Keyboard::Space)))
                && state_m!= State_m::inAir)
        {
            state_m = State_m::inAir;
            speed_m.y = -jumpingSpeed_m;
        }

        isShooting_m = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);

        if (sf::Keyboard::isKeyPressed(Keyboard::T))
        {
            cout << "teleportation: enter top: ";
            cin >> this->collizion_m->top;
            cout << "\n enter left: ";
            cin >> this->collizion_m->left;
        }
    }

    void SelectPropperAnimation()
    {
        switch ( state_m)
        {
        case State_m::standing:
            if (!isShooting_m)
                currentAnimation_m = &PlayableCharacter::idleAnimation_m;
            else
                {
                    currentAnimation_m = &PlayableCharacter::stayShootingAnimation_m;
                }
            break;
        case State_m::walking:
            if (!isShooting_m)
                currentAnimation_m = &PlayableCharacter::walkingAnimation_m;
            else
                {
                    currentAnimation_m = &PlayableCharacter::walkingShootingAnimation_m;
                }
            break;
        }

        (this->*currentAnimation_m).setFacingLeft(facingLeft_m);
    }


    void Shoot()
    {

    }
    //#define debuglogging
    void Act(EntityContainer* environment_arg) const override
    {

    }
    void Update(sf::Time frameTime_arg) override
    {
        readApplyUserInput();


        /*
        //�������� ������� ����� ������������. ��� ��������� ���������� � ����������.
        static vector<Uint32> frameTimes(400);
        static vector<Uint32>::iterator currentIter = frameTimes.begin();
        ++currentIter;
        *currentIter = frameTime_arg.asMicroseconds();
        if (currentIter==frameTimes.end())
        {
            currentIter=frameTimes.begin();
            cout << "mics: "<< ( accumulate(frameTimes.begin(), frameTimes.end(), 0) / 400)  << endl;
        }*/

        (this->*currentAnimation_m).timePassed(frameTime_arg);


        if (state_m == State_m::standing)
        {
            speed_m.x = 0;
            speed_m.y=0;
        }


        if (state_m == State_m::inAir)
        {

            speed_m.y += frameTime_arg.asMicroseconds()* gravityAcceleration/(1000000000);
        }


    #ifdef debuglogging
    #include <iostream>
        if (Keyboard::isKeyPressed(Keyboard::LShift)  )
        {
            cout << "\npos: ";
            cout << " (" << collizion_m->left+collizion_m->width << ", " << collizion_m->top << ")";
            cout << "  speed: ";
            cout << " (" << speed_m.x << ", " << speed_m.y << ")";
            cout << ", time: " << frameTime_arg.asMicroseconds();
        }
    #endif // debuglogging
        //�������� ������ �������� �����


        bool onGround = MoveTroughtTilesAndCollide(*locationMap_m, *collizion_m, speed_m, frameTime_arg)
                        ||StandingOnTheSolidGround(*locationMap_m, *collizion_m, footingDistance);

        if (onGround && state_m==State_m::inAir)
        {
            if (speed_m.x!=0)
                state_m = State_m::walking;
            else
                state_m = State_m::standing;
        }
        if (!onGround)
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


        return renderComponent_m;
    }

    Vector2f spriteCoordRelativeToCollision_m = Vector2f(0,0);
    //�������� ������ �������� ���� ������� �� �������� ������ ���� ��������


    Animator walkingAnimation_m;
    Animator idleAnimation_m;
    Animator walkingShootingAnimation_m;
    Animator stayShootingAnimation_m;
    //todo ��� 2 ��������

    Animator PlayableCharacter::*  currentAnimation_m= &PlayableCharacter::idleAnimation_m;
    enum class State_m {walking, standing, inAir};
    State_m state_m         = State_m::standing;
    bool isShooting_m       = false;
    bool facingLeft_m       = false; //� ��������� ������ ������� ������

    Vector2f speed_m        = Vector2f(0,0);

    //�������� - � �������� � ������������
    float jumpingSpeed_m        = 0.002;    //������������ ��������, ������� ��� ��������� ��� ������
    float walkingSpeed_m        = 0.001;    //��������, � ������� �� �����
    float airAcceleration_m     = 0.00002;  //��������� � �������
    float animSpeed_m           = 0.00001;  // ���� ������ � ������������
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;

    Tileset2d* locationMap_m    = nullptr;
    //<Entity>, � ����� vector<Entity*> *environment_m, ������� ��������
    //����� �������������� �����, ��� �������������� � �����
};

#endif // PLAYABLECHARACTER_H
