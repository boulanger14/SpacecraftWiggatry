#include "Enemy.h"
#include "SpriteManager.h"
#include "SoundManager.h"
#include "CollisionManager.h"


Enemy::Enemy(SDL_Rect s, SDL_FRect d, SDL_Renderer* r, SDL_Texture* t, int sstart, int smin, int smax, int nf, int hp, int dmg)
	:Character(s, d, r, t, sstart, smin, smax, nf, hp, dmg) 
{
	enemysWig = (new Wig({ 0,0,50,50 }, { d.x,d.y,40,40 }, Engine::Instance().GetRenderer(), TEMA::GetTexture("wig")));
	sight = new raycast();
	losMax = 200;
	hasWig = true;
	m_dir = -1;
	m_frameMax = 3;
	m_spriteMax = 2;
	m_frame = m_sprite = m_spriteMin = 0;

	m_accelX = m_accelY = m_velX = m_velY = 0.0;
	m_maxVelX = 10.0;
	m_grav = GRAV;
	m_drag = 0.88;
	wallWisker = { d.x, d.y, 30, 16 };
	gapWisker = { d.x, d.y, 16, 16};
}


void Enemy::setState(enemyState nState)
{
	state = nState;
	if (nState == idle)
	{
		m_frameMax = 8;
		m_maxVelX = 5;
		StopX();
	}
	else if (nState == seeking)
	{
	m_frameMax = 4;
		m_maxVelX = 10;
	}
	//else if(nState == fleeing) need to make sure fleeing sprite is in the same texture file as the normal sprite
}


void Enemy::Update()
{
	lookTimer--;

	wallWisker = { getCenter().x+ (m_dst.w * m_dir*.5f), m_dst.y + (m_dst.h * 0.5f) , 32,32 };
	gapWisker = { getCenter().x + (m_dst.w * m_dir*.75f),m_dst.y + (m_dst.h*0.75f), 24,32 };
	//X
	m_velX += m_accelX;
	m_velX *= m_drag;
	m_velX = std::min(std::max(m_velX, -(m_maxVelX)), (m_maxVelX));
	if (!COMA::SmallTileCollision(wallWisker, m_velX, 0) && COMA::SmallTileCollision(gapWisker, m_velX, 0))
	{
		m_dst.x += (int)m_velX;
		Animate();
	}
	else
	{
		if (state == idle)
			m_dir *= -1;
		else
			StopX();
	}

	//Y 
	m_velY += m_accelY + m_grav; // Adjust gravity to get slower jump.
	m_velY = std::min(std::max(m_velY, -(m_maxVelY)), (m_grav * 5));
	if (!COMA::PlayerCollision(&m_dst, 0, m_velY))
		m_dst.y += (int)m_velY; // To remove aliasing, I made cast it to an int too.
	else
		StopY();
	m_accelX = m_accelY = 0.0;

	if (iCooldown > 0)
	{
		--iCooldown;
	}
	if (hasWig)
	{
		enemysWig->SetPos({ (int)(m_dst.x + m_dst.w/2) , (int)(m_dst.y +20)});
	}

	if (state == idle)
	{
		if (lookTimer <= 0)
		{
			substate = (idlestates)(rand() % 3);
			if (substate == left)
				m_dir = -1;
			else if (substate == right)
				m_dir = 1;
			LOSCheck();
		}
		if (substate != none)
		{
			m_accelX = 2.0 * m_dir;
		}
		else
			m_frame = 0;
			StopX();

		if (sight->Update() || COMA::CircleCircleCheck(getCenter(), SPMR::getPlayer()->getCenter(), 400) && ((m_dir == -1 && SPMR::getPlayer()->GetX() <= m_dst.x) || (m_dir == 1 && SPMR::getPlayer()->GetX() >= m_dst.x)))
		{
			setState(seeking);
		}
	}
	else if (state == seeking)
	{
		int playerdir = m_dst.x - SPMR::getPlayer()->GetX();
		if (playerdir > 200)
			m_dir = -1;
		else if (playerdir < -200)
			m_dir = 1;
		groundedMove2(m_dir);

		if (playerdir > 600 || playerdir < -600)
			setState(idle);
	}
	else if (state == fleeing)
	{
		Flee();
	}
	RotatingWig();
}

void Enemy::Flee()
{
	int playerdir = m_dst.x - SPMR::getPlayer()->GetX();
	if (playerdir > 0)
		m_dir = 1;
	else
		m_dir = -1;
	groundedMove2(m_dir);
	if (playerdir > 700 || playerdir < -700)
		readyToDelete = true;
}

void Enemy::LOSCheck()
{
	SDL_FPoint plr = SPMR::getPlayer()->getCenter();
	plr.x = plr.x - m_dst.x * (float)-m_dir ;
	plr.y = plr.y - m_dst.y;
	lookTimer = losMax;
	sight->setPos(m_dst.x, m_dst.y);
	if((m_dir < 0 && plr.x < 0) || (m_dir >0 && plr.x >0))
		sight->move2stop(MAMA::Rad2Deg(MAMA::AngleBetweenPoints(plr.y, plr.x )));
}

void Enemy::Render()
{
	if (iCooldown % 10 < 5)
	{
		if (m_dir == -1)
			Sprite::Render();
		else
			Sprite::RenderFlipped();
		if (hasWig)
		{
			if (m_dir == -1)
				enemysWig->RenderFlipped();
			else
				enemysWig->Render();
		}
	}
}

Wig* Enemy::removeWig()
{
		hasWig = false;
		setState(fleeing);
		m_src.x += m_src.w;
		return enemysWig;
}

void Enemy::takeDamage(int dmg)
{
	if (iCooldown <= 0)
	{
		SOMA::PlaySound("blub", 0, 6);
		health -= dmg;
		iCooldown = iFrames;
		std::cout << "Health: " << health << std::endl;
	}
}


void Enemy::groundedMove2(const int dir)
{
	if (dir < 0)
		m_accelX = -1;
	else if (dir > 0)
		m_accelX = 1;
}

void Enemy::RotatingWig()
{
	if (health <= 0)
	{
		float angle;
		angle = enemysWig->GetAngle();
		angle += 1;
		if(angle >= 360)
		{
			angle = 0;
		}
		enemysWig->SetAngle(angle);
	}
}