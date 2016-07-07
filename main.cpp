#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

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

    vector<Entity*> *environment_m;
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        {
                currentAnimFrame_m += animSpeed_m*lastFrameTime_arg.asMicroseconds();
        }

        if ( !(sf::Keyboard::isKeyPressed(sf::Keyboard::A)&&Keyboard::isKeyPressed(Keyboard::D) ) )
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                currentAnimFrame_m+=animSpeed_m*lastFrameTime_arg.asMicroseconds();
                speed_m.x = -walkingSpeed_m;
            }
            if (sf::Keyboard::isKeyPressed(Keyboard::D))
            {
                currentAnimFrame_m+=animSpeed_m*lastFrameTime_arg.asMicroseconds();
                speed_m.x = walkingSpeed_m;
            }
        }
    }

    void Update(sf::Time frameTime_arg) override
    {
        readApplyUserInput(frameTime_arg);

        collizion_m->left += speed_m.x * frameTime_arg.asMicroseconds();
        speed_m = sf::Vector2f(0,0);
    }

    sf::Drawable* getDrawableComponent() override
    {
        //приводим спрайт в соотв. с состоянием персонажа и отдаем
        if (currentAnimFrame_m>6)
            currentAnimFrame_m-=6;


        renderComponent_m->setPosition( Vector2f(collizion_m->left, collizion_m->top)
                                        +spriteCoordRelativeToCollision_m
                                      );

        if (lookingLeft_m)
            renderComponent_m->setTextureRect(IntRect(40+40*int(currentAnimFrame_m), 244, -40, 50));
        else
            renderComponent_m->setTextureRect(IntRect(40*int(currentAnimFrame_m), 244, 40, 50));


        return renderComponent_m;
    }

    Vector2f spriteCoordRelativeToCollision_m = Vector2f(0,0);
    //смещение левого верхнего угла спрайта от верхнего левого угла коллизии
    float currentAnimFrame_m=0;
    bool onGround_m         = false;
    bool lookingLeft_m      = false; //в противном случае смотрит вправо

    Vector2f speed_m         = Vector2f(0,0);

    float jumpingSpeed_m        = 0.06; //вертикальная скорость, которая ему придается при прыжке
    float walkingSpeed_m        = 0.0006;  //скорость, с которой он ходит
    float animSpeed_m           = 0.00001; // смен кадров в микросекунду
    sf::FloatRect* collizion_m  = nullptr;
    Sprite* renderComponent_m   = nullptr;
    //Entity, с полем vector<Entity*> *environment_m, которая родитель
};

int main()
{

    sf::RenderWindow window(sf::VideoMode(400, 300), "SFML works!", sf::Style::Close);
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Texture myTxtr;
    myTxtr.loadFromFile("fang.png");

    sf::Sprite sprt;
    sprt.setTexture(myTxtr);
    sprt.setTextureRect( IntRect(0, 244, 40, 50) );
    sprt.setScale(2,2);
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
