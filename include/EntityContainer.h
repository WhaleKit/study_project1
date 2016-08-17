#ifndef ENTITYCONTAINER_H
#define ENTITYCONTAINER_H

#include <stack>
#include "Range.h"
#include "Entity.h"
#include "RuntimeSizedObjectsStack.h"
#include "ActionOnEntity.h"


static_assert(sizeof(uint8_t)==1, "��������");
class BulletPool;
using namespace sf;
using namespace std;


//Questionable architecture decisions time!




/*
��� ��������� ���������� ���, ��� ��� �� ��������� �������, �.�. ��������� �����
�� ������� �� ������� ���������� ������ � ������� ����� (�� ������� ���� �� ������),
� ������ ������� �� �����
*/
/*
��������� Entities_m:
           activeEntitiesEnd_m
  active entities \/ inactive entities
|_________________|_____________________|

activeEntitiesEnd_m - ����� ������ ���������� ������
*/
class ActionsLibrary;
class BulletPool;
class SceneAdapterForEntity;

class EntityContainer
{
    friend ActionsLibrary;
    friend SceneAdapterForEntity;
public:
    typedef size_t entityNum;
    EntityContainer(vector<Entity*> && entitesContainer_arg  )
        : Entities_m(entitesContainer_arg), activeEntitiesEnd_m(Entities_m.size())
    {    }
    EntityContainer()
    {

    }
    ~EntityContainer()
    {

    }

    void Update(Time time_arg); //������� ����

    //I wanna draw everything
    //I wanna draw
    //Even though I could fail
    void DrawEverything (sf::RenderTarget& rt_arg, sf::RenderStates rstates_arg = RenderStates::Default);

    SceneAdapterForEntity getSceneAdapter ();

    BulletPool* bulletPool_m = nullptr;//�� ������ ���������������� ��� ��������
protected:

private:
    //������, ���������� �����������, ��������������� ��� ������������� � entity::Act()
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
    //����� �������, ����������� �����������, ��������������� ��� ������������� � entity::Act()


    void RemoveEnabledEntity(entityNum toRemove_arg)
    {
        Entities_m[toRemove_arg]->entityState_m.state_m = EntityState::States::deleted;
        if ( toRemove_arg<activeEntitiesEnd_m )//�.�. ������ ��������� � ������ ��������
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
    void RemoveDisabledEntity(entityNum toRemove_arg)
    {
        disabledEntities_m[toRemove_arg]->entityState_m = EntityState::States::deleted;
        disabledEntities_m[toRemove_arg] = disabledEntities_m.back();
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
    void EnableEntity(entityNum toEnable_arg) //������������ ������ "������������" disabled Entity
    {
        AddActiveEntity( disabledEntities_m[toEnable_arg] );
        RemoveDisabledEntity(toEnable_arg);
    }
    void ActivateEntity(entityNum toActivate_arg)
    {
        swap( Entities_m[toActivate_arg], Entities_m[activeEntitiesEnd_m] );
        ++activeEntitiesEnd_m;
    }
    //��� ��� ������� �� ������ ��������������� �������� - ������ ����� ������������ � ����!
    void DoPlannedActions ()
    {
        while (!actions_m.isEmpty())
        {
            ActionOnEntity* actPtr = (ActionOnEntity*)(actions_m.getTopPointer());

            Entity* subjectsArr[actPtr->numberOfSubjects];
            for (size_t i=0; i < actPtr->numberOfSubjects; ++i)
            {
                //������ ������� ��������� ���� ����� �� ������� ActionOnEntity
                size_t subjNum = ( reinterpret_cast<uint8_t*>(actPtr)+sizeof(ActionOnEntity) )[i];
                subjectsArr[i] = Entities_m[subjNum];
            }
            //���. ������ ���� ����� �� �������� ActionOnEntity � ������� ������� ���������
            actPtr->actionFn( const_cast<Entity*>(actPtr->object)
                     , subjectsArr, actPtr->numberOfSubjects
                     , reinterpret_cast<uint8_t*>(actPtr) + actPtr->SizeWithSubjectsListOnly()
                             );

            actions_m.popWithoutGetting( actPtr->SizeWithAllData() );
        }
        while (!entitiesToAdd_m.empty())
        {
            AddActiveEntity(entitiesToAdd_m.top());
            entitiesToAdd_m.pop();
        }
    }

    stack <Entity*> entitiesToAdd_m;
    vector<Entity*> disabledEntities_m;
    vector<Entity*> Entities_m;
    entityNum       activeEntitiesEnd_m=0;
    //�� activeEntitiesEnd_m ���� �������� ������, �����, ������� � ���� - ����������
    RuntimeSizedObjectsStack actions_m;
};


#endif // ENTITYCONTAINER_H
