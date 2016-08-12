#include "EntityContainer.h"

#include "SceneAdapterForEntity.h"
#include "SFML/Graphics.hpp"

void EntityContainer::Update(Time time_arg) //главный цикл
{
    //update-методы
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

    //энтити добавл€ют действи€ в очередь на основе текущих данных об энтити
    for (Entity* toAct : make_range(Entities_m.begin(), Entities_m.begin()+activeEntitiesEnd_m))
    {
        SceneAdapterForEntity here = this->getSceneAdapter();
        toAct->Act(&here);
    }

    DoPlannedActions();
}

void EntityContainer::DrawEverything
            (sf::RenderTarget& rt_arg, sf::RenderStates rstates_arg /*= sf::RenderStates::Default*/)
{
    for (Entity* toDraw :Entities_m)
    {
        //toDraw->getDrawableComponent()->draw(rt_arg, rstates_arg);
        rt_arg.draw(*toDraw->getDrawableComponent(), rstates_arg);
    }
}

SceneAdapterForEntity EntityContainer::getSceneAdapter ()
{
    return SceneAdapterForEntity(this);
}
