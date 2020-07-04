#pragma once
#include "Character.h"
#include "Pickup.h"
#include "TextureManager.h"

class Enemy :
    public Character
{
private:
    bool hasWig;
    Wig* enemysWig;
protected:

public:
    Enemy(SDL_Rect s, SDL_FRect d, SDL_Renderer* r, SDL_Texture* t, int sstart = 0, int smin = 0, int smax = 0, int nf = 0, int hp = 2, int dmg = 1);

    bool getHasWig()
    {return hasWig; }
    Wig* getenemysWig()
    { return enemysWig; }

    void setHasWig(bool b);
    void removeWig()
    {
        hasWig = false;
    }

    void Update();
    void Render() override;

};

