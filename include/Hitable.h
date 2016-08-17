#ifndef HITABLE_H
#define HITABLE_H

#include "SFML/Graphics.hpp"

struct Damage
{
    Damage (float amount_arg=0)
    : amount(amount_arg)
    {   }

    float amount;
};

class Hitable
{
    public:
        Hitable() {}
        virtual ~Hitable() {}

        //���� �� ����� ������ ������ � �������, ���� ������ ���������� ���������� � ������������ � �-�� �������
        //������, ������ ��� ������. ����� ������ ����������� ��������� �� �����.
        virtual sf::FloatRect   getHitbox   (Entity const* whoAsking) const=0;
        //� �� ���? � ������ �� �����������?
        virtual float           getHealth   (Entity const* whoAsking) const=0;

        virtual float           getMaxHealth(Entity const* whoAsking) const=0;

        virtual float           takeHit     (Damage damageToDeal,   Entity* whoDidIt)=0;

        virtual void            beHealed    (float DamageToHeal,    Entity* whoDidIt)=0;
        //���� ��� ����� ������, ������ ��� ����� � �����... �� ��� ���������� ��� ����

    protected:

    private:
};

#endif // HITABLE_H
