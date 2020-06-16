#include "HookShot.h"
#include "EventManager.h"
#include "MathManager.h"
#include "States.h"
#include "StateManager.h"
#include "TextureManager.h"
#include "math.h"
#include <iostream>


Hookshot::Hookshot(SDL_Rect src, SDL_FRect dst, SDL_Renderer* r, SDL_Texture* t)
	: Sprite(src, dst, r, t)
{
	line.x = 0; //player
	line.y = 0;
	line.w = 0; //hook
	line.h = 0;
}

Hookshot::~Hookshot()
{

}

void Hookshot::calHookAngle()
{
	SDL_FRect* playerdst = ((GameState*)(STMA::GetStates().back()))->getPlayer()->GetDstP();

	if (EVMA::KeyHeld(SDL_SCANCODE_A))
	{
		
		if (EVMA::KeyHeld(SDL_SCANCODE_W))
		{
			shotAngle = 135;
			velX = cos(MathManager::Deg2Rad(135)) * speed;
			velY = -sin(MathManager::Deg2Rad(135)) * speed;
			int a = 10;
		}
		//x cos180 y sin180
		else
		{
			m_dst.x = playerdst->x - m_dst.w;
			m_dst.y = (playerdst->y + playerdst->h * 0.5) - (m_dst.h * 0.5);
			shotAngle = 180;
			velX = cos(MathManager::Deg2Rad(180)) * speed;
			velY = -sin(MathManager::Deg2Rad(180)) * speed;}
		}

	else if (EVMA::KeyHeld(SDL_SCANCODE_D))
	{
		if (EVMA::KeyHeld(SDL_SCANCODE_W))
		{
			shotAngle = 45;
			velX = cos(MathManager::Deg2Rad(45)) * speed;
			velY = -sin(MathManager::Deg2Rad(45)) * speed;
		}
		//x cos0 y sin0
		else
		{
			m_dst.x = playerdst->x + playerdst->w + m_dst.w;
			m_dst.y = (playerdst->y + playerdst->h * 0.5) - (m_dst.h * 0.5);
			shotAngle = 0;
			velX = cos(MathManager::Deg2Rad(0)) * speed;
			velY = -sin(MathManager::Deg2Rad(0)) * speed;
		}
	}

	else if (EVMA::KeyHeld(SDL_SCANCODE_W))
	{
		m_dst.x = (playerdst->x + (playerdst->w * 0.5)) - (m_dst.w * 0.5);
		m_dst.y = playerdst->y - m_dst.h;
		shotAngle = 90;
		velX = cos(MathManager::Deg2Rad(90)) * speed;
		velY = -sin(MathManager::Deg2Rad(90)) * speed;
	}

	else
	{
		m_dst.x = playerdst->x + playerdst->w + m_dst.w;
		m_dst.y = (playerdst->y + playerdst->h * 0.5) - (m_dst.h * 0.5);
		shotAngle = 0;
		velX = cos(MathManager::Deg2Rad(0)) * speed;
		velY = -sin(MathManager::Deg2Rad(0)) * speed;
	}
}

void Hookshot::move()
{
	m_dst.x += velX * 0.1;
	m_dst.y += velY * 0.1;
}

void Hookshot::Collision()
{
	SDL_Rect temp;
	SDL_Rect Hookdst;
	SDL_Rect Platformdst;
	SDL_FRect** PlatformArray = ((GameState*)(STMA::GetStates().back()))->getPlatform();

	Hookdst.x = m_dst.x;
	Hookdst.y = m_dst.y;
	Hookdst.w = m_dst.w;
	Hookdst.h = m_dst.h;

	for (int i = 0; i < NUMPLATFORMS; ++i)
	{
		Platformdst.x = PlatformArray[i]->x;
		Platformdst.y = PlatformArray[i]->y;
		Platformdst.w = PlatformArray[i]->w;
		Platformdst.h = PlatformArray[i]->h;

		if (SDL_IntersectRect(&Hookdst, &Platformdst, &temp))
		{
			Hookdst.y = Hookdst.y + temp.h;
			m_dst.y = Hookdst.y;
			hookFixed = true;
		}
	}
}

void Hookshot::setLine()
{
	SDL_FRect* playerdst = ((GameState*)(STMA::GetStates().back()))->getPlayer()->GetDstP();

	if (shotAngle == 0)
	{
		line.x = playerdst->x + (playerdst->w * 0.5);
		line.y = (playerdst->y + (playerdst->h * 0.5)) - (5 * 0.5);
		line.w = abs(m_dst.x - line.x);
		line.h = 5;
	}
	else if (shotAngle == 180)
	{
		line.x = m_dst.x + m_dst.w;
		line.y = (playerdst->y + (playerdst->h * 0.5)) - (5 * 0.5);
		line.w = abs(m_dst.x - playerdst->x);
		line.h = 5;
	}
	else if (shotAngle == 90)
	{
		line.x = playerdst->x + (playerdst->w * 0.5) - (5 * 0.5);
		line.y = m_dst.y + m_dst.h;
		line.w = 5;
		line.h = abs(line.y - playerdst->y);
		int a = 10;
	}
}

float Hookshot::MyLerp(float a, float b, float t)
{
	return a + t * (b - a);
}

void Hookshot::Update()
{
	if (hookFixed == false)
	{
		move();
	}
	
	Collision();
	setLine();

	if (hookFixed == true)
	{
		SDL_FRect* playerdst = ((GameState*)(STMA::GetStates().back()))->getPlayer()->GetDstP();

		if (lerpCo <= 1.0f)
		{
			playerdst->x = MyLerp(playerdst->x, m_dst.x  + (m_dst.w * 0.5) - (playerdst->w * 0.5), lerpCo);
			playerdst->y = MyLerp(playerdst->y, m_dst.y, lerpCo);
			lerpCo += 0.01f;
		}
		else
		{
			playerdst->x = m_dst.x + (m_dst.w * 0.5) - (playerdst->w * 0.5);
			playerdst->y = m_dst.y;
		}
	}


}

void Hookshot::Render()
{
	SDL_Rect m_srcline = { 0, 0, 18, 17 };
	SDL_FRect m_dstline = { line.x, line.y, line.w, line.h };
	//SDL_RenderCopyExF(m_pRend, TEMA::GetTexture("line(temp)"), &m_srcline, &m_dstline, shotAngle, nullptr, SDL_FLIP_NONE);
	SDL_RenderCopyF(m_pRend, TEMA::GetTexture("line(temp)"), &m_srcline, &m_dstline);
	SDL_RenderCopyF(m_pRend, m_pText, &m_src, &m_dst);
}

void Hookshot::sethookFixed(bool b)
{
	hookFixed = b;
}

void Hookshot::setlerpCo(float lc)
{
	lerpCo = lc;
}