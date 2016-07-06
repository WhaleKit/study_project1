#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <map>

using namespace std;
using namespace sf;

class Location;

class Entity;

class Character;

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


    Clock clc; clc.restart();

    float currentAnimFrame = 2;


    while (window.isOpen())
    {
        float lastFrameTime = clc.getElapsedTime().asMicroseconds();
        clc.restart();

        float vertSpeed=0.0006;
        float horSpeed =0.0006;
        float rotSpeed =0.0006;
        float animSpeed=0.00001; // смен кадров в микросекунду


        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                    currentAnimFrame += animSpeed*lastFrameTime;
            }

            if ( !(sf::Keyboard::isKeyPressed(sf::Keyboard::E)&&Keyboard::isKeyPressed(Keyboard::Q) ) )
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::E))
                {
                    sprt.rotate(lastFrameTime*rotSpeed*1);
                }
                if (sf::Keyboard::isKeyPressed(Keyboard::Q))
                {
                    sprt.rotate(lastFrameTime*rotSpeed*-1);
                }
            }

            if ( !(sf::Keyboard::isKeyPressed(sf::Keyboard::A)&&Keyboard::isKeyPressed(Keyboard::D) ) )
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
                {
                    currentAnimFrame+=animSpeed*lastFrameTime;

                    sprt.move(lastFrameTime*horSpeed*-1, 0);
                }
                if (sf::Keyboard::isKeyPressed(Keyboard::D))
                {
                    currentAnimFrame+=animSpeed*lastFrameTime;

                    sprt.move(lastFrameTime*horSpeed*1, 0);
                }
            }

            if ( !(sf::Keyboard::isKeyPressed(sf::Keyboard::W)&&Keyboard::isKeyPressed(Keyboard::S) ) )
            {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
                {
                    sprt.move(0, lastFrameTime*-1*vertSpeed);
                }
                if (sf::Keyboard::isKeyPressed(Keyboard::S))
                {
                    sprt.move(0, lastFrameTime*1*vertSpeed);
                }
            }
        }


        if (currentAnimFrame>6)
            currentAnimFrame-=6;
        sprt.setTextureRect(IntRect(40*int(currentAnimFrame), 244, 40, 50));

        window.clear();
        window.draw(shape);
        window.draw(sprt);
        window.display();
    }
    return 0;
}

class Location
{

};

class Entity
{

};

class Character : public Entity
{

};
