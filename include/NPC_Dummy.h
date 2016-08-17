#ifndef NPC_DUMMY_H
#define NPC_DUMMY_H

#include <iostream>

#include "Entity.h"
#include "Hitable.h"
#include "ActionOnEntity.h"
#include "Tileset2d.h"
#include "GeometryFunctions.h"

using namespace sf;
using namespace std;

class NPC_Dummy :public Hitable, public Entity
{
    public:

        NPC_Dummy ()
        {
            this->entityState_m.state_m = EntityState::States::active;
        }

        virtual ~NPC_Dummy(){};


    void Update(sf::Time frameTime_arg) override
    {

        inAir_m = !(MoveTroughtTilesAndCollide(*locationMap_m, collizion_m, speed_m, frameTime_arg)
                    ||StandingOnTheSolidGround(*locationMap_m, collizion_m, footingDistance)
                    );

        if (inAir_m)
        {
            speed_m.y += frameTime_arg.asMicroseconds()* gravityAcceleration/(1000000000.0f);
        }
        {
            speed_m.x= 0;
        }

    }

    void Act (SceneAdapterForEntity* environment_arg) const override
    {

    }
    sf::Drawable* getDrawableComponent() override
    {
        //drawableComponent_m->setPosition(collizion_m.left, collizion_m.width);
        drawableComponent_m->setPosition( Vector2f(collizion_m.left, collizion_m.top) );
        return drawableComponent_m;
    }
    sf::FloatRect   getHitbox (Entity const* whoAsking) const override
    {
        return collizion_m;
    }
    float getHealth (Entity const* whoAsking) const override
    {
        return 60;
    }

    virtual float getMaxHealth (Entity const* whoAsking) const override
    {
        return 60;
    }

    float takeHit (Damage damageToDeal, Entity* whoDidIt) override
    {
        cout << "\nDummy has just took " << damageToDeal.amount << " damage.";
        return damageToDeal.amount;
    }

    void beHealed (float DamageToHeal, Entity* whoDidIt) override
    {

    }
    bool inAir_m = true;
    FloatRect   collizion_m;
    Vector2f    speed_m;
    Sprite*     drawableComponent_m;
    Tileset2d*  locationMap_m;

    private:
};

#endif // NPC_DUMMY_H
