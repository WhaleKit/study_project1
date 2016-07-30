#ifndef ENTITY_H
#define ENTITY_H


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "EntityState.h"

using namespace std;
using namespace sf;

class EntityContainer;

class Entity
{
public:
    Entity (){}
    Entity ( EntityContainer * where_arg)
        :environment_m(where_arg), entityState_m (EntityState::States::active)
    {   }

    virtual ~Entity(){}

    virtual void Update(sf::Time frameTime_arg)=0;
    //обновляет и делает цельными свои данные

    virtual void Act (EntityContainer* where_agr) const =0;
    //действует на других, получает информацию об окружении


    virtual sf::Drawable* getDrawableComponent()=0;//должен возвращать nullptr, если не предназначем для рисования
    //хотя это не очень-то эффективно, если я буду делать много энтити не предназначенных для рисования, лучше
    //использовать флаг drawable, или пересмотреть функции этого класса

    EntityContainer *environment_m;
    EntityState entityState_m;
};


#endif // ENTITY_H
