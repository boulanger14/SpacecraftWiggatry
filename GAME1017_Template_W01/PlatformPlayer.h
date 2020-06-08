#pragma once
#ifndef _PLATFORMPLAYER_H_
#define _PLATFORMPLAYER_H_
#define GRAV 6.0
#define JUMPFORCE 60.0

#include "Sprite.h"
#include "Character.h"

class PlatformPlayer : public Character
{
public:
	PlatformPlayer(SDL_Rect s, SDL_FRect d, SDL_Renderer* r, SDL_Texture* t);
	void Update();
	void Stop();
	void StopX();
	void StopY();
	void KnockLeft(double vel);
	void SetAccelX(double a);
	void SetAccelY(double a);
	double GetAccelX();
	double GetAccelY();
	double GetThurst();
	bool IsGrounded();
	void SetGrounded(bool g);
	double GetVelX();
	double GetVelY();
	void SetVelY(double a);
	void SetX(float y);
	void SetY(float y);
	double GetX();

	void takeDamage(int dmg) override;
private:
	bool m_grounded;
	double m_accelX,
		m_accelY,
		m_velX,
		m_maxVelX,
		m_velY,
		m_maxVelY,
		m_drag,
		m_thrust,
		m_grav;
};

#endif