#include "PlatformPlayer.h"
#include "Enemy.h"
#include "Engine.h"
#include "StateManager.h"
#include "States.h"
#include "EventManager.h"
#include "Projectile.h"
#include "CollisionManager.h"
#include "SoundManager.h"
#include "TextureManager.h"
#include <algorithm>
#include <iostream>
#include "HookShot.h"
#include "EventManager.h"
#include "SoundManager.h"
#include "SpriteManager.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

PlatformPlayer::PlatformPlayer(SDL_Rect s, SDL_FRect d, SDL_Renderer* r, SDL_Texture* t, int sstart, int smin, int smax, int nf)
	:Character(s, d, r, t, sstart, smin, smax, nf)
{
	m_grounded = true;
	m_accelX = m_accelY = m_velX = m_velY = 0.0;
	m_maxVelX = 10.0;
	m_maxVelY = JUMPFORCE;
	m_thrust = -GRAV;
	m_grav = GRAV;
	m_drag = 0.88;

	health = 5;
	baseDamage = 1;
	m_wigCount = 0;
	m_ShipParts = 0;
	energy = 5;
	m_hookShot = new Hookshot({ 0,0,36,36 }, { d.x, d.y, 32, 32 }, r, TEMA::GetTexture("hookshot"));
	SetState(0);

	// SDL_Rect src, SDL_FRect dst, SDL_Renderer* r, SDL_Texture* t

}

PlatformPlayer::~PlatformPlayer()
{
	delete m_hookShot;
}

void PlatformPlayer::Update()
{
	if (m_grounded)
	{
		groundedX = m_dst.x;
		groundedY = m_dst.y;
	}

	//Do X axis first.
	m_velX += m_accelX;
	m_velX *= (m_grounded ? m_drag : 0.95f);
	m_velX = std::min(std::max(m_velX, -(m_maxVelX)), (m_maxVelX));
	if (!COMA::PlayerCollision(&m_dst, m_velX, 0) )
	{
		m_dst.x += (int)m_velX; // Had to cast it to int to get crisp collision with side of platform.
		if ((m_dst.x > 600 && m_velX > 0) || (m_dst.x < 400 && m_velX < 0 && SPMR::getOffset() > 0))
		{
			SPMR::ScrollAll((int)m_velX);
		}
	}
	else
		Stop();
	// Now do Y axis.
	m_velY += m_accelY + m_grav; // Adjust gravity to get slower jump.
	m_velY = std::min(std::max(m_velY, -(m_maxVelY)), (m_grav * 5));
	if (!COMA::PlayerCollision(&m_dst, 0, m_velY))
	{
		m_dst.y += (int)m_velY; // To remove aliasing, I made cast it to an int too.
		m_grounded = false;
	}
	else
	{
		StopY();
		if(!m_grounded)
		m_dst.y -= 1; //makes it a lot less likely that kiki gets stuck in the ground
		m_grounded = true;
	}

	m_accelX = m_accelY = 0.0;

	// Traps
	if (COMA::PlayerHazardCollision(&m_dst, m_velX, 0))
	{
		takeDamage(1);
		KnockLeft(5);
	}

	if (iCooldown > 0)
	{
		--iCooldown;
	}

	if (m_hookShot->isActive())
	{
		m_hookShot->Update(m_grav);
	}

	// l/r inputs
	switch (m_state)
	{
	case idle:
		if (EVMA::KeyHeld(SDL_SCANCODE_A) || EVMA::KeyHeld(SDL_SCANCODE_D))
		{
			SetState(running);
		}
		break;
	case running:
		if (EVMA::KeyReleased(SDL_SCANCODE_A) || EVMA::KeyReleased(SDL_SCANCODE_D))
		{
			SetState(idle);
			break; // Skip movement parsing below.
		}
		if (EVMA::KeyHeld(SDL_SCANCODE_A))
		{
			m_accelX = -1.0;
			m_facingRight = false;
			//	m_grapplehook = false;
		}
		else if (EVMA::KeyHeld(SDL_SCANCODE_D))
		{
			m_accelX = 1.0;
			m_facingRight = true;
			//	m_grapplehook = false;
		}
		break;
	case slapping:
		if (EVMA::KeyHeld(SDL_SCANCODE_A) || EVMA::KeyHeld(SDL_SCANCODE_D))
		{
			SetState(running);
		}
		break;
		if (EVMA::KeyReleased(SDL_SCANCODE_A) || EVMA::KeyReleased(SDL_SCANCODE_D))
		{
			SetState(idle);
			break;
		}
	}
	Animate();
	if (EVMA::KeyHeld(SDL_SCANCODE_W))
	{
		m_facingUp = true;
	}
	else
	{
		m_facingUp = false;
	}

	// jump
	if (EVMA::KeyHeld(SDL_SCANCODE_SPACE) && !m_grounded)
	{
		if (m_velY >= 0)
		{
			m_accelY = m_grav/2;
			m_velY = 0;
		}
	}
	if (EVMA::KeyPressed(SDL_SCANCODE_SPACE) && m_grounded)
	{
		SOMA::PlaySound("jump");
		m_accelY = -JUMPFORCE; // Sets the jump force.
		m_grounded = false;
	}


	if (EVMA::MousePressed(1))
	{
		m_hookShot->setActive(!m_hookShot->isActive());
		if (m_hookShot->gethookFixed() == false)  
		{
			m_hookShot->SetPos({ (int)(m_dst.x + m_dst.w * 0.5), (int)(m_dst.y + m_dst.h * 0.5) });
			m_hookShot->calHookAngle(&m_dst);
		}
		else
		{
			m_hookShot->deactivateHookshot();
			m_grav = GRAV;
		}
	}

	if (EVMA::KeyPressed(SDL_SCANCODE_Q) || EVMA::MousePressed(3))
	{
		SOMA::PlaySound("slap");
		slap();
		SetState(slapping);
	}
	if (EVMA::KeyPressed(SDL_SCANCODE_E))
	{
		if (energy > 0)
		{
			SOMA::PlaySound("stun");
			createStunGunBullet();
		}
	}
}

void PlatformPlayer::Stop() // If you want a dead stop both axes.
{
	StopX();
	StopY();
}


bool PlatformPlayer::IsGrounded() { return m_grounded; }
void PlatformPlayer::SetGrounded(bool g) { m_grounded = g; }

void PlatformPlayer::SetX(float y) { m_dst.x = y; }
void PlatformPlayer::SetY(float y) { m_dst.y = y; }
double PlatformPlayer::GetX() { return m_dst.x; }
double PlatformPlayer::GetY() { return m_dst.y; }
void PlatformPlayer::GoBack() // put the player back after falling in trap
{
	SOMA::PlaySound("dead", 0, 8);
	SDL_Delay(400);
	m_dst.x = (groundedX < m_dst.x) ? groundedX : groundedX + m_dst.w;
	m_dst.y = groundedY - 10;
	KnockLeft(m_velX *10);
	StopX(); // clear vel for smoother transition... very small difference but...
	StopY();
	takeDamage(1);
}
void PlatformPlayer::KnockLeft(double vel) { m_velX -= vel; }

void PlatformPlayer::Render()
{
	if (iCooldown % 10 < 5)
	SDL_RenderCopyExF(m_pRend, m_pText, GetSrcP(), GetDstP(), m_angle, 0, (SDL_RendererFlip)!m_facingRight);

	if (m_hookShot->isActive())
	{
		m_hookShot->Render();
		SDL_RenderDrawLineF(m_pRend, getCenter().x, getCenter().y, m_hookShot->getCenter().x, m_hookShot->getCenter().y);
	}
}

double PlatformPlayer::GetThurst() { return m_thrust; }

void PlatformPlayer::takeDamage(int dmg)
{
	if (iCooldown <= 0)
	{
		health -= dmg;
		iCooldown = iFrames;
		std::cout << "Health: " << health << std::endl;
		SOMA::PlaySound("oof", 0, 6);
	}
}

void PlatformPlayer::add1Wig()
{
	++m_wigCount;

	// play wig sfx! 
	srand((unsigned)time(NULL));
	int x = rand() % 3;
	if (x == 0)
		SOMA::PlaySound("wig0", 0, 6);
	if (x == 1)
		SOMA::PlaySound("wig1", 0, 6);
	if (x == 2)
		SOMA::PlaySound("wig2", 0, 6);
}


//Player - Enemy Slap Collision

void PlatformPlayer::slap()
{		
	for (int i = 0; i < SPMR::GetEnemies().size(); i++)
	{
		if (SPMR::GetEnemies()[i] != nullptr)
		{
			Enemy* Enemy = SPMR::GetEnemies()[i];
			float PlayerX = m_dst.x + (m_dst.w * 0.5);
			float PlayerY = m_dst.y + (m_dst.h * 0.5);
			
			
			if(COMA::AABBCheck({ m_facingRight ? PlayerX: PlayerX-100, PlayerY, 100.0f, 34.0f }, *Enemy->GetDstP()))
			{
				Enemy->takeDamage(1);

			}
		}
	}
}

void PlatformPlayer::createStunGunBullet()
{
	if(m_facingUp)
	SPMR::PushSprite(new Projectile(true, { m_dst.x + m_dst.w * .5f, m_dst.y + m_dst.h * .2f },
		90, 1, TEMA::GetTexture("laser")));
	else
	SPMR::PushSprite(new Projectile(true, { m_dst.x + m_dst.w * .5f, m_dst.y + m_dst.h * .2f },
		((double)!m_facingRight) * 180, 1, TEMA::GetTexture("laser")));
	reduceEnergy();
}


void PlatformPlayer::SetState(int s)
{
	m_state = static_cast<state>(s);
	m_frame = 0;
	if (m_state == idle)
	{
		m_sprite = m_spriteMin = m_spriteMax = 0;
	}
	else if (m_state == slapping)
	{
		m_sprite = m_spriteMin = 3;
		m_spriteMax = 5;
	}
	else // Only other is running for now...
	{
		m_sprite = m_spriteMin = 0;
		m_spriteMax = 2;
	
	}
}