#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H

#include <stack>
#include "Range.h"
#include "Entity.h"
#include "RuntimeSizedObjectsStack.h"
#include "ActionOnEntity.h"

static_assert(sizeof(uint8_t)==1, "ѕараной€");

using namespace sf;
using namespace std;


//Questionable architecture decisions time!




/*
мои алгоритмы отличаютс€ тем, что они не сохран€ют пор€док, т.к. поведение сцены
не зависит от пор€дка обновлени€ энтити в главном цикле (по крайней мере не должно)
а значит пор€док не важен
*/
/*
структура Entities_m:
           activeEntitiesEnd_m
  active entities \/ inactive entities
|_________________|_____________________|

activeEntitiesEnd_m - номер первой неактивной энтити
*/
class EntityContainer
{
public:
    typedef size_t entityNum ;
    EntityContainer()
    {

    }
    ~EntityContainer()
    {

    }

    void Update(Time time_arg)
    {
        //update-методы
        //for (Entity* toUpdate : make_range(Entities_m.begin(), Entities_m.begin()+activeEntitiesEnd_m))
        for (entityNum toUpdIndex=0; toUpdIndex<activeEntitiesEnd_m; ++toUpdIndex)
        {
            Entity *& toUpdate = Entities_m[toUpdIndex];

            switch (toUpdate->entityState_m.state_m)
            {
            case (EntityState::States::active):
                break;
            case (EntityState::States::disabled):
                DisableEntity(toUpdIndex);
                break;
            case (EntityState::States::inactive):
                DeactivateEntity(toUpdIndex);
                break;
            case (EntityState::States::waitForDeletion):
                RemoveEnabledEntity(toUpdIndex);
                break;
            }

            if (toUpdate->entityState_m.state_m == EntityState::States::active)
                toUpdate->Update(time_arg);

            switch (toUpdate->entityState_m.state_m)
            {
            case (EntityState::States::active):
                break;
            case (EntityState::States::disabled):
                DisableEntity(toUpdIndex);
                break;
            case (EntityState::States::inactive):
                DeactivateEntity(toUpdIndex);
                break;
            case (EntityState::States::inactiveForTime):
                --(toUpdate->entityState_m.itersToActvate);
                if(toUpdate->entityState_m.itersToActvate==0)
                    toUpdate->entityState_m.state_m = EntityState::States::active;
                break;
            case (EntityState::States::waitForDeletion):
                RemoveEnabledEntity(toUpdIndex);
                break;
            }
        }

        for (Entity* toAct : make_range(Entities_m.begin(), Entities_m.begin()+activeEntitiesEnd_m))
        {
            toAct->Act(this);
        }

        DoPlannedActions();
    }
    entityNum numOfEntities ()
    {
        return Entities_m.size();
    }
    Entity const* accessEntity (entityNum num_arg)
    {
        return (Entity const*) (Entities_m[num_arg]);
    }
    void PlanActOnEntity (ActionOnEntity action_arg, uint8_t* dataSrc_arg = nullptr)
    {
        if (action_arg.dataSize!=0)
            actions_m.pushMemory(dataSrc_arg, action_arg.dataSize);
        actions_m.push(action_arg);
    }
    void PushToActionsStack (uint8_t* dataSrc_arg, size_t dataSize_arg = 0)
    {
        actions_m.pushMemory(dataSrc_arg, dataSize_arg);
    }
    void PlanSpawnEntity (Entity* toAdd_arg)
    {
        entitiesToAdd_m.push(toAdd_arg);
    }
protected:
    void RemoveEnabledEntity(entityNum toRemove_arg)
    {
        if ( toRemove_arg<activeEntitiesEnd_m )//т.е. энтити находитс€ в секции активных
        {
            --activeEntitiesEnd_m;
            Entities_m[toRemove_arg] = Entities_m[activeEntitiesEnd_m];
            Entities_m[activeEntitiesEnd_m] = Entities_m.back();
            Entities_m.pop_back();
        }
        else
        {
            Entities_m[toRemove_arg] = Entities_m.back();
            Entities_m.pop_back();
        }
    }
    void RemoveDisabledEntity(entityNum toRemova_arg)
    {
        disabledEntities_m[toRemova_arg] = disabledEntities_m.back();
        disabledEntities_m.pop_back();
    }
    entityNum DisableEntity(entityNum toDisable_arg)
    {
        disabledEntities_m.push_back(Entities_m[toDisable_arg]);
        RemoveEnabledEntity(toDisable_arg);
        return disabledEntities_m.size()-1;
    }
    void DeactivateEntity(entityNum toDeactivate_arg)
    {
        --activeEntitiesEnd_m;
        swap( Entities_m[toDeactivate_arg], Entities_m[activeEntitiesEnd_m] );
    }
    entityNum AddActiveEntity(Entity* toAdd)
    {
        Entities_m.push_back(Entities_m[activeEntitiesEnd_m]);
        Entities_m[activeEntitiesEnd_m] = toAdd;
        ++activeEntitiesEnd_m;
    }
    void EnableEntity(entityNum toEnable_arg) //единственный способ "активировать" disabled Entity
    {
        AddActiveEntity( disabledEntities_m[toEnable_arg] );
        RemoveDisabledEntity(toEnable_arg);
    }
    void ActivateEntity(entityNum toActivate_arg)
    {
        swap( Entities_m[toActivate_arg], Entities_m[activeEntitiesEnd_m] );
        ++activeEntitiesEnd_m;
    }
    //вс€ эта система со стэком запланированных действий - просто пушка направленна€ в ногу!
    void DoPlannedActions ()
    {
        while (!actions_m.isEmpty())
        {
            ActionOnEntity* actPtr = (ActionOnEntity*)(actions_m.getTopPointer());
            actPtr->action(Entities_m[actPtr->subjectNum]
                           , reinterpret_cast<uint8_t*>(actPtr)+sizeof(ActionOnEntity) );
            //в стэке действий данные дл€ действи€ идут сразу за действием,
            //так что указатель на доп. данные - это указатель на месту сразу за действием

            actions_m.popWithoutGetting( actPtr->dataSize + sizeof(ActionOnEntity) );
        }
        while (entitiesToAdd_m.empty())
        {
            AddActiveEntity(entitiesToAdd_m.top());
            entitiesToAdd_m.pop();
        }
    }

private:
    stack<Entity*>  entitiesToAdd_m;
    vector<Entity*> disabledEntities_m;
    vector<Entity*> Entities_m;
    entityNum       activeEntitiesEnd_m=0;//до него идут активные энтити, после - неактивные
    RuntimeSizedObjectsStack actions_m;
};

#endif // ENTITYCONTAINER_H
