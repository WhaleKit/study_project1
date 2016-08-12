#ifndef SCENEADAPTERFORENTITY_H
#define SCENEADAPTERFORENTITY_H

#include "EntityContainer.h"
using namespace sf;
using namespace std;
//класс-представитель EntityContainer дл€ энтити.
//ћы же не хотим, чтобы энтити вызывали update(), draw() или что-то в этом роде.
class SceneAdapterForEntity
{
public:
    friend ActionsLibrary;
    friend EntityContainer;
    SceneAdapterForEntity() = default;
    SceneAdapterForEntity(EntityContainer* adaptee_arg)
    :adaptee_m(adaptee_arg)
    {   }

    //методы, €вл€ющиес€ интерфейсом, предназначенным дл€ использовани€ в entity::Act()
    inline EntityContainer::entityNum numOfEntities ()
    {
        return adaptee_m->numOfEntities();
    }
    inline Entity const* accessEntity (EntityContainer::entityNum num_arg)
    {
        return adaptee_m->accessEntity(num_arg);
    }
    inline void PlanActOnEntity (ActionOnEntity action_arg, uint8_t* dataSrc_arg = nullptr)
    {
        adaptee_m->PlanActOnEntity(action_arg, dataSrc_arg);
    }
    inline void PushToActionsStack (uint8_t* dataSrc_arg, size_t dataSize_arg = 0)
    {
        adaptee_m->PushToActionsStack(dataSrc_arg, dataSize_arg);
    }
    inline void PlanSpawnEntity (Entity* toAdd_arg)
    {
        adaptee_m->PlanSpawnEntity(toAdd_arg);
    }
    BulletPool* getBulletPool()
    {
        return adaptee_m->bulletPool_m;
    }
    //конец методов, €вл€ющиихс€ интерфейсом, предназначенным дл€ использовани€ в entity::Act()
private:
    EntityContainer* adaptee_m = nullptr;
};

#endif // SCENEADAPTERFORENTITY_H
