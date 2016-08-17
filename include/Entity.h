#ifndef ENTITY_H
#define ENTITY_H


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include "EntityState.h"

using namespace std;
using namespace sf;

class SceneAdapterForEntity;

class Entity
{
public:
    Entity (){}
    Entity ( SceneAdapterForEntity * where_arg)
        :environment_m(where_arg), entityState_m (EntityState::States::active)
    {   }

    virtual ~Entity(){}

    virtual void Update(sf::Time frameTime_arg)=0;
    //��������� � ������ �������� ���� ������

    virtual void Act (SceneAdapterForEntity* where_agr) const =0;
    //��������� �� ������, �������� ���������� �� ���������


    virtual sf::Drawable* getDrawableComponent()=0;//������ ���������� nullptr, ���� �� ������������ ��� ���������
    //���� ��� �� �����-�� ����������, ���� � ���� ������ ����� ������ �� ��������������� ��� ���������, �����
    //������������ ���� drawable, ��� ������������ ������� ����� ������

    SceneAdapterForEntity *environment_m;
    EntityState entityState_m;
};


#endif // ENTITY_H
