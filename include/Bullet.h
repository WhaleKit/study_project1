#ifndef BULLET_H
#define BULLET_H


#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "Hitable.h"
#include "EntityContainer.h"

#include "ActionsLibrary.h"
#include "GeometryFunctions.h"
#include "Tileset2d.h"

using namespace std;
using namespace sf;

class Bullet : public Entity
{
public:
    Bullet(){};
    virtual ~Bullet(){};


    void Update(sf::Time frameTime_arg) override
    {
        moveFrameAgo_m = static_cast<float>(frameTime_arg.asMicroseconds()) * speed_m;
        {
            Vector2f speedWas = speed_m;
            MoveTroughtTilesAndCollide(*locationMap_m, collizion_m, speed_m, frameTime_arg);
            if (speed_m!=speedWas)
            {   //пуля врезалась в стену.
                entityState_m = EntityState::States::waitForDeletion;
            }
        }
    }

    void Act (SceneAdapterForEntity* where_arg) const override
    {
        size_t numOfEntities = where_arg->numOfEntities();

        for (size_t i = 0; i<numOfEntities; ++i)
        {
            Hitable const* hPointer = dynamic_cast<Hitable const*> (where_arg->accessEntity(i));
            if(hPointer == nullptr)
            {
                continue;
            }
            if ( collizion_m.intersects( hPointer->getHitbox(this) ) )
            {
                ActionsLibrary::PlanHurtAction(where_arg, this, i, this->bulletDmg_m);
                ActionOnEntity disapear(
                        disapear.actionFn = []( Entity* object, Entity** subjectsList, size_t subjectsNum, void* data )->void
                        {
                            object->entityState_m.state_m = EntityState::States::waitForDeletion;
                        }, this, 0, 0);

                where_arg->PlanActOnEntity(disapear);
                break;
            }
        }
    }

    sf::Drawable* getDrawableComponent() override
    {
        drawableComponent_m->setPosition( collizion_m.left, collizion_m.top );
        return drawableComponent_m;
    }


    Tileset2d*  locationMap_m;
    Entity*     whoShooted_m;//---указатель на выстрелившего. чтобы знать, кому засчитывать фраг
    Vector2f    moveFrameAgo_m;//-на сколько сдвинулся при прошлом обновлении
    FloatRect   collizion_m;
    Vector2f    speed_m;//--------скорость - в пикселях в микросекунду
    Damage      bulletDmg_m;
    Sprite*     drawableComponent_m;

protected:
private:
};

#endif // BULLET_H
