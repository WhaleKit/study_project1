#ifndef ENTITY_H
#define ENTITY_H


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

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
};


#endif // ENTITY_H
