#ifndef BULLETPOOL_H
#define BULLETPOOL_H

#include <list>
#include <array>
#include <SFML/Graphics.hpp>

#include "Bullet.h"

using namespace std;
//���� ��������� "���������"/������������ � ����, ���� �� Entity::state_m == EntityState::States::deleted;

//������� ��������� � �������� ��� �����. ��� - ������ ��������� �� ��, ����� ������������ ��� ��� "��� ��������".
class BulletPool
{
public:
    BulletPool()
    {}
    virtual ~BulletPool() {}

    void ResetBullet (Bullet* toReset_arg)
    {
        toReset_arg->~Bullet();
        new( toReset_arg ) Bullet();
        //�������� ����������, �� ��� ������ �������� ������ �� new, � �� �������������
    }
    Bullet* TakeFreeObject()
    {
        for (array<Bullet, 64>& page : objectPages)
        {
            for (Bullet& b: page)
            {
                if (b.entityState_m.state_m == EntityState::States::deleted)
                {
                    ResetBullet(&b);
                    return &b;
                }
            }
        }
        //���� �� ������� ��������� ����
        objectPages.emplace_back();
        return &(objectPages.back()[0]);

    }




protected:
    list<array<Bullet, 64>> objectPages; //������������ ���������
private:
};


#endif // BULLETPOOL_H
