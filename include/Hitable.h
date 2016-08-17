#ifndef HITABLE_H
#define HITABLE_H

#include "SFML/Graphics.hpp"

struct Damage
{
    Damage (float amount_arg=0)
    : amount(amount_arg)
    {   }

    float amount;
};

class Hitable
{
    public:
        Hitable() {}
        virtual ~Hitable() {}

        //если мы хотим всякие обманы и иллюзии, надо всегда передавать информацию о спрашивающем в ф-ии запроса
        //однако, нельзя его менять. Можно только планировать изменения на потом.
        virtual sf::FloatRect   getHitbox   (Entity const* whoAsking) const=0;
        //а ви кто? а почему вы спгашиваете?
        virtual float           getHealth   (Entity const* whoAsking) const=0;

        virtual float           getMaxHealth(Entity const* whoAsking) const=0;

        virtual float           takeHit     (Damage damageToDeal,   Entity* whoDidIt)=0;

        virtual void            beHealed    (float DamageToHeal,    Entity* whoDidIt)=0;
        //если его можно ранить, значит его можно и убить... ну или перевязать ему раны

    protected:

    private:
};

#endif // HITABLE_H
