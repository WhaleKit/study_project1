#ifndef ACTIONSLIBRARY_H
#define ACTIONSLIBRARY_H

#include "ActionOnEntity.h"
#include "EntityContainer.h"
#include "SceneAdapterForEntity.h"
#include "Hitable.h"

class actFns
{
public:
    static void HurtActionFunction (Entity* object, Entity** subjectsList, size_t subjectsNum, void* data)
    {
        Damage* dmg = reinterpret_cast<Damage*> (data);
        Hitable* entityToHurt = dynamic_cast<Hitable*>( subjectsList[0] );
        if (entityToHurt!=nullptr)
        {
            entityToHurt->takeHit( *dmg, object );
        }
    }
};

class ActionsLibrary
{
public:

    ActionsLibrary () = delete;
    ActionsLibrary (ActionsLibrary const&) = delete;
    ~ActionsLibrary() = delete;
    ActionsLibrary& operator=(ActionsLibrary const&) = delete;

    //SceneAdapterForEntity

    static void PlanHurtAction
            (SceneAdapterForEntity* ecAdapter_arg, Entity const* object_arg, size_t subjectNum_arg, Damage dmg_arg )
    {
        ActionsLibrary::PlanHurtAction(ecAdapter_arg->adaptee_m, object_arg, subjectNum_arg, dmg_arg);
    }

    static void PlanHurtAction
            (EntityContainer* ec_arg, Entity const* object_arg, size_t subjectNum_arg, Damage dmg_arg )
    {
        ec_arg->actions_m.push(dmg_arg);
        ec_arg->actions_m.push(subjectNum_arg);
        ActionOnEntity actionToPush;
        actionToPush.actionFn = &actFns::HurtActionFunction;
        actionToPush.dataSize = sizeof (Damage);
        actionToPush.numberOfSubjects = 1;
        actionToPush.object   = const_cast<Entity*> (object_arg);
        ec_arg->actions_m.push(actionToPush);
    }

protected:

private:
};

#endif // ACTIONSLIBRARY_H
