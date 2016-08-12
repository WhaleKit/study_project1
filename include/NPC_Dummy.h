#ifndef NPC_DUMMY_H
#define NPC_DUMMY_H

#include <iostream>

#include "Entity.h"
#include "Hitable.h"
#include "ActionOnEntity.h"

using namespace sf;
using namespace std;

class NPC_Dummy :public Hitable, public Entity
{
    public:
        NPC_Dummy(){};

        NPC_Dummy (EntityContainer* where_arg)
        :Entity(where_arg), EntityState(EntityState::States::inactive)
        {   }

        virtual ~NPC_Dummy(){};


    void Update(sf::Time frameTime_arg) override
    {

    }

    void Act (EntityContainer* where_agr) const override
    {

    }
    sf::Drawable* getDrawableComponent() override
    {
        return drawableComponent_m;
    }

    sf::FloatRect getHitbox (Entity* whoAsking) override
    {
        return collizion_m;
    }
    float getHealth (Entity* whoAsking) override
    {
        return 60;
    }

    virtual float getMaxHealth (Entity* whoAsking) override
    {
        return 60;
    }

    float takeHit (Damage damageToDeal, Entity* whoDidIt) override
    {
        cout << "\nDummy has just took " << damageToDeal << " damage.";
        return damageToDeal;
    }

    void beHealed (float DamageToHeal, Entity* whoDidIt) override
    {

    }

    protected:
    FloatRect   collizion_m;
    Vector2f    speed_m;
    Sprite*     drawableComponent_m;

    private:
};

#endif // NPC_DUMMY_H
