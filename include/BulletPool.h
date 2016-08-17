#ifndef BULLETPOOL_H
#define BULLETPOOL_H

#include <list>
#include <array>
#include <SFML/Graphics.hpp>

#include "Bullet.h"

using namespace std;
//пуля считается "свободной"/возвращенной в пулл, если ее Entity::state_m == EntityState::States::deleted;

//снаряды спавнятся и исчезают все время. Они - лучшие кандидаты на то, чтобы использовать для них "пул объектов".
class BulletPool
{
public:
    BulletPool()
    {}
    virtual ~BulletPool() {}

    void ResetBullet (Bullet* toReset_arg)
    {
        toReset_arg->~Bullet();
        new( toReset_arg ) Bullet();
        //выглядит примитивно, но моя задача избежать затрат на new, а не инициализацию
    }
    Bullet* TakeFreeObject()
    {
        for (array<Bullet, 64>& page : objectPages)
        {
            for (Bullet& b: page)
            {
                if (b.entityState_m.state_m == EntityState::States::deleted)
                {
                    ResetBullet(&b);
                    return &b;
                }
            }
        }
        //если не нашлось свободных пуль
        objectPages.emplace_back();
        return &(objectPages.back()[0]);

    }




protected:
    list<array<Bullet, 64>> objectPages; //постраничная аллокация
private:
};


#endif // BULLETPOOL_H
