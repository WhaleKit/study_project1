#ifndef ENTITYSTATE_H
#define ENTITYSTATE_H

#include "SFML/Graphics.hpp"

//дело в том, что проверка переменной-члена быстрее, чем вызов функции, типа isAlive()


//disabled - как бы не существует, нельщ€ взаимодействовать, не рисуетс€
//inactive - update не вызываетс€, рисуетс€, с ним можно взаимодействовать
//inactiveForTime - активируетс€ когда счетчик itersToActivate достигнет нул€. —четчик уменьшаетс€ при каждом обновлении
    //полезно дл€ энтити, которым не требуетс€ высокое временное разрешение
//active   - рисуетс€, дл€ него вызываетс€ метод update
//waitForDeletion - удал€етс€ при проходе главным циклом

//deleted - маркер того, что entity можно удал€ть/вернуть в пул объектов и указател€ на него у контейнера не осталось

struct EntityState
{
public:
    enum class States {disabled, inactive, inactiveForTime, active, waitForDeletion, deleted};
        EntityState(States state_arg = States::active, sf::Uint8 iters_arg = 0)
        :state_m(state_arg), itersToActvate(iters_arg)
        {}

        ~EntityState() {}
    States state_m;
    sf::Uint8 itersToActvate=0;
};

#endif // ENTITYSTATE_H
