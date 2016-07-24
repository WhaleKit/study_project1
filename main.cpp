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

#include "Animator.h"
#include "PlayableCharacter.h"
#include "Tileset2d.h"

using namespace std;
using namespace sf;

void DoNothing()
{
    ;
}

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
            "s   ssssssss  ssssssssss",
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
    sprt.setPosition(-300, -200);
    FloatRect FangCollizion = sprt.getGlobalBounds();
    FangCollizion.height-=FangCollizion.height*0.0001; //��� ����� �� ��� ��������� � ������� ������� � ����
    PlayableCharacter Fang(&FangCollizion ,&sprt);
    Fang.locationMap_m = &levelTiles;
    Fang.state_m = PlayableCharacter::State_m::inAir;

    Fang.collizion_m->left  = -8.33299-Fang.collizion_m->width;
    Fang.collizion_m->top   = 1077.61;
    Fang.speed_m = Vector2f(0.001, 0);
    Time tm = microseconds(8333);
    MoveTroughtTilesAndCollide(levelTiles, *Fang.collizion_m, Fang.speed_m, tm);
    DoNothing();
}


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
            "s   ssssssss  ssssssssss",
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
    sprt.setPosition(-300, -200);
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
    window.setFramerateLimit(120);
    //main cycle
    static const Time minFrameTime = seconds(1.0/120); //120 fps should be enough
    while (window.isOpen())
    {
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
            toUpdate->Update(/*frameTime*/ minFrameTime );
        }

        //rendering
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

        window.clear();
    }
    return 0;
}
