#ifndef ENTITYSTATE_H
#define ENTITYSTATE_H

#include "SFML/Graphics.hpp"

//���� � ���, ��� �������� ����������-����� �������, ��� ����� �������, ���� isAlive()


//disabled - ��� �� �� ����������, ������ �����������������, �� ��������
//inactive - update �� ����������, ��������, � ��� ����� �����������������
//inactiveForTime - ������������ ����� ������� itersToActivate ��������� ����. ������� ����������� ��� ������ ����������
    //������� ��� ������, ������� �� ��������� ������� ��������� ����������
//active   - ��������, ��� ���� ���������� ����� update
//waitForDeletion - ��������� ��� ������� ������� ������


struct EntityState
{
public:
    enum class States {disabled, inactive, inactiveForTime, active, waitForDeletion};
        EntityState(States state_arg = States::active, sf::Uint8 iters_arg = 0)
        :state_m(state_arg), itersToActvate(iters_arg)
        {}

        ~EntityState() {}
    States state_m;
    sf::Uint8 itersToActvate=0;
};

#endif // ENTITYSTATE_H
