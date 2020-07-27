#include "PlatformPlayer.h"
#include "Enemy.h"
#include "Engine.h"
#include "StateManager.h"
#include "States.h"
#include "EventManager.h"
#include "StunGun.h"
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
	//Do X axis first.
	m_velX += m_accelX;
	m_velX *= (m_grounded ? m_drag : 1);
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
			m_dst.y += (int)m_velY; // To remove aliasing, I made cast it to an int too.
	else
	{
		StopY();
		if(!m_grounded)
		m_dst.y -= 3; //makes it a lot less likely that kiki gets stuck in the ground
		m_grounded = true;
	}

	if (m_dst.y > 655)
		m_dst.y = 655;

	m_accelX = m_accelY = 0.0;
  
	//Bullet update
	for (int i = 0; i < m_vPBullets.size(); ++i)
	{
		m_vPBullets[i]->update();
	}
	//Projectile update
	for (int i = 0; i < m_vPProjectiles.size(); ++i)
	{
		m_vPProjectiles[i]->update();
	}
	//StunGun Collision update
	StunGunCollision();
	//Projectile Collision update
	ProjectileCollision();
	//StunGun Bound Check
	ProjectileBoundCheck();

	//Projectile Collision Check
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
			m_dir = -1;
			//	m_grapplehook = false;
		}
		else if (EVMA::KeyHeld(SDL_SCANCODE_D))
		{
			m_accelX = 1.0;
			m_facingRight = true;
			m_dir = 1;
			//	m_grapplehook = false;
		}
		break;
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
			m_accelY = m_thrust;
			m_velY = 0;
		}
	}
	if (EVMA::KeyPressed(SDL_SCANCODE_SPACE) && m_grounded)
	{
		SOMA::PlaySound("jump");
		m_accelY = -JUMPFORCE; // Sets the jump force.
		m_grounded = false;
	}

	//// hookshot
	//if (m_grapplehook == false)
	//	RemoveHookShot();

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

	if (EVMA::KeyPressed(SDL_SCANCODE_Q))
	{
		//SOMA::PlaySound("jump"); //change jump into slap sound
		slap();
	}
	if (EVMA::KeyPressed(SDL_SCANCODE_E))
	{
		if(energy > 0)
		//SOMA::PlaySound("jump"); //change jump into bullet sound
		createStunGunBullet();
	}
	if (EVMA::KeyPressed(SDL_SCANCODE_R))
	{
		createProjectile();
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
	StopX(); // clear vel for smoother transition... very small difference but...
	StopY();
	SDL_Delay(700);
	m_dst.x -= 120;
	m_dst.y -= 100;
	if(!m_grounded) //in case the player hookshots themself through the ground
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

	for (int i = 0; i < m_vPBullets.size(); ++i)
	{
		m_vPBullets[i]->Render();
	}

	for (int i = 0; i < m_vPProjectiles.size(); ++i)
	{
		m_vPProjectiles[i]->Render();
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
			float EnemyX = Enemy->GetDstP()->x + (Enemy->GetDstP()->w * 0.5);
			float EnemyY = Enemy->GetDstP()->y + (Enemy->GetDstP()->h * 0.5);

			m_distance = sqrtf(((PlayerX - EnemyX) * (PlayerX - EnemyX)) + ((PlayerY - EnemyY) * (PlayerY - EnemyY)));

			if (m_distance < 100) 
			{
				Enemy->takeDamage(1);

			}
		}
	}
}

void PlatformPlayer::createStunGunBullet()
{
	StunGun* stungun = new StunGun({ 0,0,36,36 }, { m_dst.x, m_dst.y, 32, 32 }, Engine::Instance().GetRenderer(), TEMA::GetTexture("lightning"));
	reduceEnergy();

	if (m_facingRight == true)
	{
		stungun->setVelX(20);
	}
	else
	{
		stungun->setVelX(-20);
	}
	
	if (m_facingUp == true)
	{
		stungun->setVelX(0);
		stungun->setVelY(-20);
	}

	m_vPBullets.push_back(stungun);
}

void PlatformPlayer::createProjectile()
{
	Projectile* projectile = new Projectile(true, { m_dst.x + m_dst.w * .5f, m_dst.y + m_dst.h * .2f }, ((double)!m_facingRight)*180, 1 , TEMA::GetTexture("lightning"));

	/*if (m_facingRight == true)
	{
		projectile->setVelX(20);
	}
	else
	{
		projectile->setVelX(-20);
	}

	if (m_facingUp == true)
	{
		projectile->setVelX(0);
		projectile->setVelY(-20);
	}*/

	m_vPProjectiles.push_back(projectile);
}

//Playre - Enemy StunGun Collision
void PlatformPlayer::StunGunCollision()
{
	for (int i = 0; i < SPMR::GetEnemies().size(); i++)
	{
		if (SPMR::GetEnemies()[i] != nullptr)
		{
			Enemy* Enemy = SPMR::GetEnemies()[i];
			SDL_Rect EnemyDst;
			EnemyDst.x = Enemy->GetDstP()->x;
			EnemyDst.y = Enemy->GetDstP()->y;
			EnemyDst.w = Enemy->GetDstP()->w;
			EnemyDst.h = Enemy->GetDstP()->h;

			SDL_Rect temp;
			std::vector<StunGun*>::iterator iterBegin = m_vPBullets.begin();

			for (int i = 0; i < (int)m_vPBullets.size(); ++i, ++iterBegin)
			{
				SDL_Rect gunshotDst;
				gunshotDst.x = m_vPBullets[i]->GetDstP()->x;
				gunshotDst.y = m_vPBullets[i]->GetDstP()->y;
				gunshotDst.w = m_vPBullets[i]->GetDstP()->w;
				gunshotDst.h = m_vPBullets[i]->GetDstP()->h;

				if (SDL_IntersectRect(&EnemyDst, &gunshotDst, &temp))
				{
					Enemy->takeDamage(1);
					delete m_vPBullets[i];
					m_vPBullets.erase(iterBegin);

					break;
				}
			}
		}
	}
}

void PlatformPlayer::ProjectileCollision()
{
	for (int i = 0; i < SPMR::GetEnemies().size(); i++)
	{
		if (SPMR::GetEnemies()[i] != nullptr)
		{
			Enemy* Enemy = SPMR::GetEnemies()[i];
			SDL_Rect EnemyDst;
			EnemyDst.x = Enemy->GetDstP()->x;
			EnemyDst.y = Enemy->GetDstP()->y;
			EnemyDst.w = Enemy->GetDstP()->w;
			EnemyDst.h = Enemy->GetDstP()->h;

			SDL_Rect temp;
			std::vector<Projectile*>::iterator iterBegin = m_vPProjectiles.begin();

			for (int i = 0; i < (int)m_vPProjectiles.size(); ++i, ++iterBegin)
			{
				SDL_Rect gunshotDst;
				gunshotDst.x = m_vPProjectiles[i]->GetDstP()->x;
				gunshotDst.y = m_vPProjectiles[i]->GetDstP()->y;
				gunshotDst.w = m_vPProjectiles[i]->GetDstP()->w;
				gunshotDst.h = m_vPProjectiles[i]->GetDstP()->h;

				if (SDL_IntersectRect(&EnemyDst, &gunshotDst, &temp))
				{
					Enemy->takeDamage(1);
					delete m_vPProjectiles[i];
					m_vPProjectiles.erase(iterBegin);
					break;
				}
			}
		}
	}
}

void PlatformPlayer::BulletBoundCheck()
{
	
	std::vector<StunGun*>::iterator iterBegin = m_vPBullets.begin();

	for (int i = 0; i < (int)m_vPBullets.size(); ++i, ++iterBegin)
	{
		if (m_vPBullets[i]->GetDstP()->x > WIDTH)
		{
			delete m_vPBullets[i];
			m_vPBullets.erase(iterBegin);
			break;
		}
	}
}

void PlatformPlayer::ProjectileBoundCheck()
{
	std::vector<Projectile*>::iterator iterBegin = m_vPProjectiles.begin();

	for (int i = 0; i < (int)m_vPProjectiles.size(); ++i, ++iterBegin)
	{
		if (m_vPProjectiles[i]->GetDstP()->x > WIDTH)
		{
			delete m_vPProjectiles[i];
			m_vPProjectiles.erase(iterBegin);
			break;
		}
	}
}

void PlatformPlayer::SetState(int s)
{
	m_state = static_cast<state>(s);
	m_frame = 0;
	if (m_state == idle)
	{
		m_sprite = m_spriteMin = m_spriteMax = 0;
	}
	else // Only other is running for now...
	{
		m_sprite = m_spriteMin = 0;
		m_spriteMax = 2;
	
	}
}

