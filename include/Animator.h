#ifndef ANIMATOR_h
#define ANIMATOR_h

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "GeometryFunctions.h"

using namespace std;
using namespace sf;

//класс абстрагирует анимацию на спрайтщитах, передвигая TextureRect на указанной текстуре в соответствии с текущим кадром
//при помощи указанной функции. Является и аниматором, поддерживая ангимацию внешнего стпрайта, и анимацией, рисуясь,
//т.е. может быть использован как незавасимый drawable, так и для анимации внешнего спрайта
//(он информирован о спрайте, но не аггрегирует его)
class Animator : public Drawable
{
public:
    typedef IntRect (*TextureRectUpdater) (float, void*);

    Animator (Sprite* sprt_arg=nullptr, float animSpeed_arg = 0.000001
              , Uint16 framesLimit_arg=1, TextureRectUpdater updaterFunc_arg=nullptr
              , void* additionalDataPtr_arg=nullptr)
    :   sprt_m(sprt_arg), animSpeed_m(animSpeed_arg), currentAnimFrame_m(0)
        ,framesLimit_m(framesLimit_arg), textureRectUpdaterFunction_m(updaterFunc_arg)
        ,ptrToAdditionalDataForTextureRectUpdaterFunction_m(additionalDataPtr_arg)
    {}

    ~Animator () {}

    inline void timePassed(Time time_arg)
    {
        //приводим текущий кадр в соответствии с прошедшим временем
        currentAnimFrame_m = (currentAnimFrame_m + time_arg.asMicroseconds()*animSpeed_m);
        if (currentAnimFrame_m>=framesLimit_m)
            currentAnimFrame_m-=framesLimit_m;

        updateSprite();
    }
    inline void updateSprite ()
    {
        IntRect  ir= textureRectUpdaterFunction_m(currentAnimFrame_m
                                                , ptrToAdditionalDataForTextureRectUpdaterFunction_m );
        sprt_m->setTextureRect( ir );
        if (facingLeft_m)
            sprt_m->setTextureRect(HorizontalFlip( sprt_m->getTextureRect() ));
    }
    inline void setFacingLeft(bool arg)
        {facingLeft_m=arg;}
    void draw (RenderTarget& renTr_arg, RenderStates states_arg) const override
    {
        //приводим спрайт в соотв. с текущим кадром
        renTr_arg.draw(*sprt_m, states_arg);
    }
    void SetSpritePtr (Sprite* newSprite_arg)
    {
        sprt_m = newSprite_arg;
    }
    Sprite* GetSpritePtr( )
    {
        return sprt_m;
    }
    void ResetAnim ()
    {
        currentAnimFrame_m=0;
    }
    void setAnimSpeed (float newSpeed_arg)
    {
        animSpeed_m = newSpeed_arg;
    }
    float getAnimSpeed ()
    {
        return animSpeed_m;
    }
    Uint16 getAnimFrameLimit ()
    {
        return framesLimit_m;
    }
    void setAnimFrameLimit (Uint16 newFrameLimit_arg)
    {
        framesLimit_m = newFrameLimit_arg;
    }
    void setTextureRectUpdaterFunctionAndData (TextureRectUpdater newTRU_arg, void* newData_arg = nullptr)
    {
        textureRectUpdaterFunction_m = newTRU_arg;
        ptrToAdditionalDataForTextureRectUpdaterFunction_m = newData_arg;
    }
    tuple<TextureRectUpdater, void*> getTextureRectUpdaterFunctionAndData ()
    {
        return make_tuple(textureRectUpdaterFunction_m
                          ,ptrToAdditionalDataForTextureRectUpdaterFunction_m);
    }

private:
    bool facingLeft_m=false;
    Sprite* sprt_m=nullptr;
    float animSpeed_m=0.000001; //смен кадров в микросекунду
    float currentAnimFrame_m=0;
    Uint16 framesLimit_m=1;
    TextureRectUpdater textureRectUpdaterFunction_m = nullptr;
    void * ptrToAdditionalDataForTextureRectUpdaterFunction_m = nullptr;
    //что если бы функции нужно было завиесть от чего-то внешнего? Я мог бы использовать std_function, но он слишком медленный для игр
    //так что в функцию передается указатель на она_сама_разберется_какие_данные, или nullptr, если она в них не нуждается
};
//да, я мог бы сделать animator абтрактным классом, c виртуальным update
// но тогда мне придется все время обращатся к его потомкам по указателю
//или ссылке, и я не смогу гарантировать их одинаковые размеры
//так я сделал 1 класс "толще" на 1 указатель, но получид дополнительную гибкость



#endif // ANIMATOR_h
