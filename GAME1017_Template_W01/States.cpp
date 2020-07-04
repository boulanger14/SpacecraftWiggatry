#include "States.h"
#include "CollisionManager.h"
#include "EventManager.h"
#include "SoundManager.h"
#include "StateManager.h" // Make sure this is NOT in "States.h" or circular reference.
#include "TextureManager.h"
#include "FontManager.h"
#include "MathManager.h"
#include "Engine.h"
#include "Button.h"
#include "Enemy.h"
#include "PlatformPlayer.h"
#include "HookShot.h"
#include <iostream>
#include <fstream>
#include <string>
#include "SpriteManager.h"
#include "Utilities.h"


// Begin State. CTRL+M+H and CTRL+M+U to turn on/off collapsed code.
void State::Render()
{
	SDL_RenderPresent(Engine::Instance().GetRenderer());
}
void State::Resume() {}
// End State.

// Begin GameState.
GameState::GameState() {}

PlatformPlayer* GameState::getPlayer()
{
	return 	m_pPlayer;

}

Enemy* GameState::getEnemy()
{
	return 	m_pEnemy;

}

void GameState::Enter()
{
	std::cout << "Entering GameState..." << std::endl;
	// FOMA::SetSize("Img/font.ttf", "font", 35); not working DX
	m_pPlayer = new PlatformPlayer({ 0,0,400,152 }, { 150.0f,500.0f,96.0f,96.0f }, 
								   Engine::Instance().GetRenderer(), TEMA::GetTexture("player"));
	m_pEnemy = new Enemy({ 0,0,400,140 }, {850.0f, 200.0f, 50.0f, 120.0f}, 
									Engine::Instance().GetRenderer(), TEMA::GetTexture("enemy"), 3, 1);
	m_pauseBtn = new PauseButton({ 0,0,86,78 }, { 1005.0f,0.0f,21.5f,19.5f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("pause"));
	for (int i = 0; i < (5); i++)
		hpUI[i] = new Sprite({ 0,0, 256,256 }, { (float)(35*i),0, 35,35 }, Engine::Instance().GetRenderer(), TEMA::GetTexture("heart"));
	wigUI = new Sprite({ 0,0, 100,100 }, { (float)(185),0, 35,35 }, Engine::Instance().GetRenderer(), TEMA::GetTexture("wig"));
	sprintf_s(buff, "%d", m_pPlayer->getWigs()); // convertersion
	words[0] = new Label("font", 225, 0, buff, { 188,7,208,0 });
	m_pReticle = new Sprite({ 0,0, 36,36 }, { 0,0, 25,25 }, Engine::Instance().GetRenderer(), TEMA::GetTexture("reticle"));

	Engine::LoadLevel("Dat/Level1.txt");
	m_level = Engine::GetLevel();
	m_platforms = SPMR::GetPlatforms();

	m_pPickUpList.push_back(new Wig({ 0,0,100,100 }, { 600.0f, 400.0f,50.0f,50.0f },
					Engine::Instance().GetRenderer(), TEMA::GetTexture("wig")));

	SOMA::PlayMusic("PokerFace");
	std::cout << m_platforms.size() << "\n";
	SPMR::PushSprite(m_pPlayer, Regular);
	SPMR::PushSprite(m_pEnemy, Regular);
}

void GameState::Update()
{

	m_pReticle->SetPos(EVMA::GetMousePos());
	m_pEnemy->Update();

	CheckCollision();

	// Pause button/key
	if (EVMA::KeyPressed(SDL_SCANCODE_P))
	{
		STMA::PushState(new PauseState);
	}
	if (m_pauseBtn->ButtonUpdate() == 1)
		return;

	// UI
	for (int i = 0; i < m_pPickUpList.size(); i++)
		if (m_pPickUpList[i] != nullptr)m_pPickUpList[i]->Update();
	sprintf_s(buff, "%d", m_pPlayer->getWigs());
	words[0]->SetText(buff);

	//// Panning
	//m_bgScrollX = m_bgScrollY = false;
	//if (m_pPlayer->GetVelX() > 0 && m_pPlayer->GetDstP()->x > WIDTH * 0.5f)
	//{
	//	if (m_level[0][COLS - 1]->GetDstP()->x > WIDTH - 32)
	//	{
	//		m_bgScrollX = true;
	//		UpdateTiles((float)m_pPlayer->GetVelX(), true);
	//	}
	//}
	//else if (m_pPlayer->GetVelX() < 0 && m_pPlayer->GetDstP()->x < WIDTH * 0.4f)
	//{
	//	if (m_level[0][0]->GetDstP()->x < 0)
	//	{
	//		m_bgScrollX = true;
	//		UpdateTiles((float)m_pPlayer->GetVelX(), true);
	//	}
	//}
	/*if (m_pPlayer->GetVelY() > 0 && m_pPlayer->GetDstP()->y > HEIGHT * 0.7f)
	{
		if (m_level[ROWS - 1][0]->GetDstP()->y > HEIGHT - 32)
		{
			m_bgScrollY = true;
			UpdateTiles((float)m_pPlayer->GetVelY());
		}
	}*/
	//else if (m_pPlayer->GetVelY() < 0 && m_pPlayer->GetDstP()->y < HEIGHT * 0.3f)
	//{
	//	if (m_level[0][0]->GetDstP()->y < 0)
	//	{
	//		m_bgScrollY = true;
	//		UpdateTiles((float)m_pPlayer->GetVelY());
	//	}
	//}
	m_pPlayer->Update(m_bgScrollX, m_bgScrollY); // Change to player Update here.
	CheckCollision();
}

void GameState::UpdateTiles(float scroll, bool x)
{
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (x)
				m_level[row][col]->GetDstP()->x -= scroll;
			else
				m_level[row][col]->GetDstP()->y -= scroll;
		}
	}
}

void GameState::CheckCollision()
{	

	//for (unsigned i = 0; i < m_platforms.size(); i++) // For each platform.
	//{
	//	if (COMA::AABBCheck(*m_pPlayer->GetDstP(), *m_platforms[i]->GetDstP()))
	//	{
	//		if (m_pPlayer->GetDstP()->y + m_pPlayer->GetDstP()->h - (float)m_pPlayer->GetVelY() <= m_platforms[i]->GetDstP()->y)
	//		{ // Colliding top side of platform.
	//			m_pPlayer->SetGrounded(true);
	//			m_pPlayer->StopY();
	//			m_pPlayer->SetY(m_platforms[i]->GetDstP()->y - m_pPlayer->GetDstP()->h);
	//		}
	//		else if (m_pPlayer->GetDstP()->y - (float)m_pPlayer->GetVelY() >= m_platforms[i]->GetDstP()->y + m_platforms[i]->GetDstP()->h)
	//		{ // Colliding bottom side of platform.
	//			m_pPlayer->StopY();
	//			m_pPlayer->SetY(m_platforms[i]->GetDstP()->y + m_platforms[i]->GetDstP()->h);
	//		}
	//		else if (m_pPlayer->GetDstP()->x + m_pPlayer->GetDstP()->w - m_pPlayer->GetVelX() <= m_platforms[i]->GetDstP()->x)
	//		{ // Collision from left.
	//			m_pPlayer->StopX(); // Stop the player from moving horizontally.
	//			m_pPlayer->SetX(m_platforms[i]->GetDstP()->x - m_pPlayer->GetDstP()->w);
	//		}
	//		else if (m_pPlayer->GetDstP()->x - (float)m_pPlayer->GetVelX() >= m_platforms[i]->GetDstP()->x + m_platforms[i]->GetDstP()->w)
	//		{ // Colliding right side of platform.
	//			m_pPlayer->StopX();
	//			m_pPlayer->SetX(m_platforms[i]->GetDstP()->x + m_platforms[i]->GetDstP()->w);
	//		}
	//	}
	//}

	if (COMA::AABBCheck(*m_pPlayer->GetDstP(), *m_pEnemy->GetDstP()))
	{
		if (m_pPlayer->GetDstP()->x - (float)m_pPlayer->GetVelX() >= m_pEnemy->GetDstP()->x + m_pEnemy->GetDstP()->w)
		{ // Colliding right side of platform.
			m_pPlayer->StopX();
			m_pPlayer->KnockLeft(-10); //knock the player to the right
		}
		else
		{
			m_pPlayer->Stop();
			m_pPlayer->KnockLeft(10);
		}
		m_pPlayer->takeDamage(m_pEnemy->getBaseDamage());
	}
	for (int i = 0; i < m_pPickUpList.size(); i++)
	{
		if (COMA::CircleCircleCheck(m_pPlayer->getCenter(), m_pPickUpList[i]->getCenter(), 40, 50))
		{
			switch (m_pPickUpList[i]->getType())
			{
			case WIG:
				m_pPlayer->add1Wig();
				break;
			case SHIP_PART:
				m_pPlayer->add1ShipPart();
				break;
			default:
				break;
			}
			delete m_pPickUpList[i];
			m_pPickUpList[i] = nullptr;
			std::cout << "collected\n";
			m_pPickUpList.erase(m_pPickUpList.begin()+i);
			m_pPickUpList.shrink_to_fit();
		}
	}
}

void GameState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 0, 0, 0, 255);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			m_level[row][col]->Render();
		}
	}
	//draw the enemy
	m_pEnemy->Render();
	// Draw the player.

	m_pPlayer->Render();
	
	//// Draw the platforms.
	//SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 70, 192, 0, 255);

	m_pReticle->Render();
	m_pauseBtn->Render();
	//SPMR::Render();

	for (int i = 0; i < m_pPickUpList.size(); i++)
		m_pPickUpList[i]->Render();

	for (int i = 0; i < (m_pPlayer->getHealth()); i++)
		hpUI[i]->Render();

	wigUI->Render();
	words[0]->Render();

	// If GameState != current state.
	if (dynamic_cast<GameState*>(STMA::GetStates().back()))
		State::Render();

}

void GameState::Exit()
{
	delete m_pPlayer;
	delete m_pEnemy;
	delete m_pReticle;
	for (int i = 0; i < m_pPickUpList.size(); i++)
	{
		delete m_pPickUpList[i];
		m_pPickUpList[i] = nullptr;
	}
	m_pPickUpList.clear();
	m_pPickUpList.shrink_to_fit();

	CleanVector(SPMR::GetSprites());
	SPMR::RemoveLevel();
	
}

void GameState::Resume() { }
// End GameState.

// Begin TitleState.
TitleState::TitleState() {}

void TitleState::Enter()
{
	m_playBtn = new PlayButton({ 0,0,400,100 }, { 312.0f,400.0f,400.0f,100.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("play"));
	m_quitBtn = new QuitButton({ 0,0,400,100 }, { 312.0f,520.0f,400.0f,100.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("exit"));
	words[0] = new Label("font", 180, 110, "SPACECRAFT", { 188,7,208,0 });
	words[1] = new Label("font", 260, 200, "Wiggatry", { 255,255,255,0 });
	words[2] = new Label("font", 0, 670, "ETTG", { 255,0,180,0 });
	SOMA::Load("Aud/power.wav", "beep", SOUND_SFX);
}

void TitleState::Update()
{
	if (m_playBtn->ButtonUpdate() == 1)
		return; 
	if (m_quitBtn->ButtonUpdate() == 1)
		return;
}

void TitleState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 51, 255, 0);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	for (int i = 0; i < 3; i++)
		words[i]->Render();
	m_playBtn->Render();
	m_quitBtn->Render();
	State::Render();
}

void TitleState::Exit()
{
	std::cout << "Exiting TitleState..." << std::endl;
}
// End TitleState.

// Pause StateStuff
PauseState::PauseState() {}

void PauseState::Enter()
{
	std::cout << "Entering Pause...\n";
	m_resumeBtn = new ResumeButton({ 0,0,200,80 }, { 415.0f,400.0f,200.0f,80.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("resume"));//1024
}

void PauseState::Update()
{
	if (m_resumeBtn->ButtonUpdate() == 1)
		return;
}

void PauseState::Render()
{
	STMA::GetStates().front()->Render();
	SDL_SetRenderDrawBlendMode(Engine::Instance().GetRenderer(), SDL_BLENDMODE_BLEND); // below won't be taken into account unles we do this // blendmode create transparency
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 51, 225, 100);
	SDL_Rect rect = { 173, 128, 700, 512 };
	SDL_RenderFillRect(Engine::Instance().GetRenderer(), &rect);
	m_resumeBtn->Render();
	State::Render();
}

void PauseState::Exit()
{
	std::cout << "Exiting PauseState...\n";
}
// End of PauseState

// Begin DeadState.
DeadState::DeadState() {}

void DeadState::Enter()
{
	std::cout << "Entering DeadState...\n";
	words = new Label("font", 220, 110, "GAME OVER", { 255,255,255,0 });
	m_playBtn = new PlayButton({ 0,0,400,100 }, { 312.0f,400.0f,400.0f,100.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("replay"));
	m_quitBtn = new QuitButton({ 0,0,400,100 }, { 312.0f,520.0f,400.0f,100.0f }, Engine::Instance().GetRenderer(), TEMA::GetTexture("exit"));
	SOMA::Load("Aud/power.wav", "beep", SOUND_SFX);
	SOMA::PlayMusic("WreckingBall");
}

void DeadState::Update()
{
	if (m_playBtn->ButtonUpdate() == 1)
		return;
	if (m_quitBtn->ButtonUpdate() == 1)
		return;
}

void DeadState::Render()
{
	SDL_SetRenderDrawColor(Engine::Instance().GetRenderer(), 255, 0, 0, 0);
	SDL_RenderClear(Engine::Instance().GetRenderer());
	words->Render();
	m_playBtn->Render();
	m_quitBtn->Render();
	State::Render();
}

void DeadState::Exit()
{
	std::cout << "Exiting DeadState...a" << std::endl;
}
// End DeadState.