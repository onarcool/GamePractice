#include "Game.h"

#include <SFML/Graphics.hpp>
#include "MathHelpers.h"
#include <random>

#include <iostream>
#include <algorithm>


Game::Game()
	: m_Window(sf::VideoMode(sf::Vector2u(1920u, 1080u)), "My First Game ^_^")
	, m_eGameMode(Play)
	, m_iTileOptionIndex(0)
	, m_PlayerTexture("Images/Player.png")
	, m_EnemyTexture("Images/Enemy.png")
	, m_SpearTexture("Images/Spear.png")
	, m_Font("Fonts/calibri.ttf")
	, m_GameModeText(m_Font)
	, m_TileMapTexture("Images/TileMap.png")
	, m_vRequestedPlayerMovementDirection(0.0f, 0.0f)
	, m_fPlayerSpeed(100.0f)
	, m_Player(
		m_PlayerTexture,
		Entity::PhysicsData(
			Entity::PhysicsData::Shape::Circle,
			Entity::PhysicsData::Type::Dynamic))
	, m_Enemy(
		m_EnemyTexture,
		Entity::PhysicsData(
			Entity::PhysicsData::Shape::Circle,
			Entity::PhysicsData::Type::Dynamic))
	, m_Spear(
		m_SpearTexture,
		Entity::PhysicsData(
			Entity::PhysicsData::Shape::Circle,
			Entity::PhysicsData::Type::Static))
	, m_tile(m_TileMapTexture)
	, m_eScrollWheelInput(ScrollWheelInput::None)

{
	m_Player.SetScale(sf::Vector2f(10, 10));
	m_Player.SetOrigin(sf::Vector2f(8, 8));

	m_Player.SetCirclePhysics(80.0f);
	m_Player.AddEntityToIgnore(&m_Spear);

	m_Enemy.SetScale(sf::Vector2f(10, 10));
	m_Enemy.SetPosition(sf::Vector2f(960, 540));
	m_Enemy.SetOrigin(sf::Vector2f(8, 8));

	m_Enemy.SetCirclePhysics(80.0f);

	m_Spear.SetScale(sf::Vector2f(10, 10));
	m_Spear.SetOrigin(sf::Vector2f(8, 8));
	m_Spear.SetCirclePhysics(80.0f);

	m_GameModeText.setPosition(sf::Vector2f(1520, 40));
	m_GameModeText.setString("PlayMode");

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			m_tile.SetTextureRect(sf::IntRect({ x * 16, y * 16 }, { 16, 16 }));
			m_tile.SetScale(sf::Vector2f(10, 10));
			m_tile.SetOrigin(sf::Vector2f(8, 8));
			m_TileOptions.push_back(m_tile);
		}
	}

	m_AllEntities.push_back(&m_Player);
	m_AllEntities.push_back(&m_Enemy);
	m_AllEntities.push_back(&m_Spear);
}

Game::~Game()
{
}



void Game::Run()
{
	sf::Clock clock;

	while (m_Window.isOpen())
	{
		m_DeltaTime = clock.restart();
		HandleInput();

		switch (m_eGameMode)
		{
		case Play:
			UpdatePlay();
		case LevelEditor:
			UpdateLevelEditor();
		}

		Draw();

	}
}

void Game::UpdatePlay()
{
	//m_Player.move(m_vRequestedPlayerMovement * m_DeltaTime.asSeconds() * m_fPlayerSpeed);
	m_Player.SetVelocity(m_vRequestedPlayerMovementDirection * m_fPlayerSpeed);

	// Gather mouse position data
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	sf::Vector2f vPlayerToMouse = vMousePosition - m_Player.GetPosition();

	sf::Vector2f vPlayerToMouseNormalize = MathHelpers::Normalize(vPlayerToMouse);

	// Set the position of the spear to be slightly offset from the player
	// But rotates around it based on where your mouse is
	m_Spear.SetPosition(m_Player.GetPosition() + vPlayerToMouseNormalize * 160.0f);

	// Did the axe hit the enemy?
	//sf::Vector2f vSpearToEnemy = m_Enemy.getPosition() - m_Spear.getPosition();
	//float fLengthFromSpearToEnemy = MathHelpers::Length(vSpearToEnemy);

	//if (fLengthFromSpearToEnemy < 160.0f)
	//{
	//	// The spear has hit the enemy
	//	sf::Vector2f vNewPosition(std::rand() % 1920, std::rand() % 1080);
	//	// Randomly spawn enemy at new location
	//	m_Enemy.setPosition(vNewPosition);
	//}

	// Normalize distance between enemy and player
	sf::Vector2f vEnemyToPlayer = m_Player.GetPosition() - m_Enemy.GetPosition();
	vEnemyToPlayer = MathHelpers::Normalize(vEnemyToPlayer);
	float fEnemySpeed = 50.0f;

	// Make enemy automatically move towards the player
	m_Enemy.SetVelocity(vEnemyToPlayer * fEnemySpeed);

	UpdatePhysics();

}

void Game::UpdateLevelEditor()
{

	return;
}

void Game::UpdatePhysics()
{
	std::vector<Entity*> entities;
	entities.reserve(m_AllEntities.size() + m_Tiles.size());

	for (Entity* entity : m_AllEntities)
	{
		entities.push_back(entity);
	}

	for (Entity& tile : m_Tiles)
	{
		entities.push_back(&tile);
	}

	for (Entity* entity : entities)
	{
		if (entity->GetPhysicsData().m_eType != Entity::PhysicsData::Type::Dynamic)
		{
			continue;
		}

		entity->Move(entity->GetPhysicsData().m_vVelocity * m_DeltaTime.asSeconds());

		for (Entity* otherEntity : entities)
		{
			if (entity == otherEntity)
			{
				continue;
			}

			if (entity->ShouldIgnoreEntityForPhysics(otherEntity))
			{
				continue;
			}

			ProcessCollision(*entity, *otherEntity);
		}
	}
}

void Game::ProcessCollision(Entity& entity1, Entity& entity2)
{
	if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
	{
		// We're a circle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're colliding with another circle
			const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
			const float fDistanceBetweenEntities = MathHelpers::Length(vEntity1ToEntity2);
			const float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;
			if (fDistanceBetweenEntities < fSumOfRadii)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// Entity 2 is static, so we need to push entity 1 away from entity 2
					entity1.Move(-MathHelpers::Normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBetweenEntities));
				}
				else
				{
					// Both entities are dynamic, so we need to push them both away from each other
					entity1.Move(-MathHelpers::Normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBetweenEntities) * 0.5f);
					entity2.Move(MathHelpers::Normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBetweenEntities) * 0.5f);
				}
			}
		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're colliding with a rectangle
			float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth * 0.5f, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth * 0.5f);
			float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight * 0.5f, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight * 0.5f);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity1.GetPhysicsData().m_fRadius)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// Entity 2 is static, so we need to push entity 1 away from entity 2
					entity1.Move(-MathHelpers::Normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistance));
				}
				else
				{
					// Both entities are dynamic, so we need to push them both away from each other
					entity1.Move(-MathHelpers::Normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistance) * 0.5f);
					entity2.Move(MathHelpers::Normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistance) * 0.5f);
				}
			}
		}
	}
	else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
	{
		// We're a rectangle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're colliding with a circle
			float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth * 0.5f, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth * 0.5f);
			float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight * 0.5f, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight * 0.5f);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity2.GetPhysicsData().m_fRadius)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// Entity 2 is static, so we need to push entity 1 away from entity 2
					entity1.Move(MathHelpers::Normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistance));
				}
				else
				{
					// Both entities are dynamic, so we need to push them both away from each other
					entity1.Move(MathHelpers::Normalize(vCircleToClosestPoint) * (entity2.GetPhysicsData().m_fRadius - fDistance) * 0.5f);
					entity2.Move(-MathHelpers::Normalize(vCircleToClosestPoint) * (entity2.GetPhysicsData().m_fRadius - fDistance) * 0.5f);
				}
			}
		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're colliding with another rectangle
			float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
			float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

			float fOverlapX = (entity1.GetPhysicsData().m_fWidth * 0.5f + entity2.GetPhysicsData().m_fWidth * 0.5f) - fDistanceX;
			float fOverlapY = (entity1.GetPhysicsData().m_fHeight * 0.5f + entity2.GetPhysicsData().m_fHeight * 0.5f) - fDistanceY;

			if (fOverlapX > 0.0f && fOverlapY > 0.0f)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// Entity 2 is static, so we need to push entity 1 away from entity 2
					if (fOverlapX < fOverlapY)
					{
						entity1.Move(sf::Vector2f((entity1.GetPosition().x < entity2.GetPosition().x ? -1.0f : 1.0f) * fOverlapX, 0.0f));
					}
					else
					{
						entity1.Move(sf::Vector2f(0.0f, (entity1.GetPosition().y < entity2.GetPosition().y ? -1.0f : 1.0f) * fOverlapY));
					}
				}
				else
				{
					// Both entities are dynamic, so we need to push them both away from each other
					if (fOverlapX < fOverlapY)
					{
						entity1.Move(sf::Vector2f((entity1.GetPosition().x < entity2.GetPosition().x ? -1.0f : 1.0f) * fOverlapX * 0.5f, 0.0f));
						entity2.Move(sf::Vector2f((entity1.GetPosition().x < entity2.GetPosition().x ? 1.0f : -1.0f) * fOverlapX * 0.5f, 0.0f));
					}
					else
					{
						entity1.Move(sf::Vector2f(0.0f, (entity1.GetPosition().y < entity2.GetPosition().y ? -1.0f : 1.0f) * fOverlapY * 0.5f));
						entity2.Move(sf::Vector2f(0.0f, (entity1.GetPosition().y < entity2.GetPosition().y ? 1.0f : -1.0f) * fOverlapY * 0.5f));
					}
				}
			}
		}
	}
}

void Game::Draw()
{
	m_Window.clear();


	// Draw all of the tiles
	for (const Entity& tile : m_Tiles)
	{
		m_Window.draw(tile);
	}


	m_Window.draw(m_Enemy);
	m_Window.draw(m_Player);
	m_Window.draw(m_Spear);
	m_Window.draw(m_GameModeText);
	
	switch (m_eGameMode)
	{
	case Play:
		break;
	case LevelEditor:
		DrawLevelEditor();
		break;
	}

	m_Window.display();
}

void Game::DrawLevelEditor()
{
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	m_TileOptions[m_iTileOptionIndex].SetPosition(vMousePosition);

	m_Window.draw(m_TileOptions[m_iTileOptionIndex]);
}

void Game::HandleInput()
{
	static bool bTwasPressedLastUpdate = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::T))
	{
		if (!bTwasPressedLastUpdate)
		{
			if (m_eGameMode == Play)
			{
				m_eGameMode = LevelEditor;
				m_GameModeText.setString("Level Editor Mode");
			}
			else
			{
				m_eGameMode = Play;
				m_GameModeText.setString("Play Mode");
			}
		}
		bTwasPressedLastUpdate = true;
	}
	else
	{
		bTwasPressedLastUpdate = false;
	}

	m_eScrollWheelInput = None;
	while (const std::optional event = m_Window.pollEvent())
	{
		// Check if the user clicked the close window button
		if (event->is<sf::Event::Closed>())
		{
			m_Window.close();
		}
		else if (const auto* mouseWheel = event->getIf<sf::Event::MouseWheelScrolled>())
		{
			if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical)
			{
				if (mouseWheel->delta > 0.0f)
				{
					m_eScrollWheelInput = ScrollUp;
				}
				else
				{
					m_eScrollWheelInput = ScrollDown;
				}
			}
		}
	}


		switch (m_eGameMode)
		{
		case Play:
			HandlePlayInput();
			break;
		case LevelEditor:
			HandleLevelEditorInput();
			break;
		}
	}

	void Game::HandlePlayInput()
	{
		m_vRequestedPlayerMovementDirection = sf::Vector2f(0.0f, 0.0f);
		// Gather the player's input here
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
		{
			m_vRequestedPlayerMovementDirection += sf::Vector2f(0.0f, -1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
		{
			m_vRequestedPlayerMovementDirection += sf::Vector2f(-1.0f, 0.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
		{
			m_vRequestedPlayerMovementDirection += sf::Vector2f(0.0f, 1.0f);
		}
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
		{
			m_vRequestedPlayerMovementDirection += sf::Vector2f(1.0f, 0.0f);
		}
		// Normalize the player's movement vector
		m_vRequestedPlayerMovementDirection = MathHelpers::Normalize(m_vRequestedPlayerMovementDirection);
	}


void Game::HandleLevelEditorInput()
{
	static bool bLeftMouseWasPressed = false;
	static bool bRightMouseWasPressed = false;

	if (m_eScrollWheelInput == ScrollUp)
	{
		m_iTileOptionIndex++;
		if (m_iTileOptionIndex >= m_TileOptions.size())
		{
			m_iTileOptionIndex = 0;
		}
	}
	else if (m_eScrollWheelInput == ScrollDown)
	{
		m_iTileOptionIndex--;
		if (m_iTileOptionIndex < 0)
		{
			m_iTileOptionIndex = m_TileOptions.size() - 1;
		}
	}

	const bool bLeftMouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
	if (bLeftMouseIsPressed && !bLeftMouseWasPressed)
	{
		sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);

		CreateTileAtPosition(vMousePosition);
	}
	bLeftMouseWasPressed = bLeftMouseIsPressed;

	const bool bRightMouseIsPressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Right);
	if (bRightMouseIsPressed && !bRightMouseWasPressed)
	{
		sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);

		DeleteTileAtPosition(vMousePosition);
	}
	bRightMouseWasPressed = bRightMouseIsPressed;

	return;

}



void Game::CreateTileAtPosition(const sf::Vector2f& position)
{
	int x = position.x / 160;
	int y = position.y / 160;

	Entity tile = m_TileOptions[m_iTileOptionIndex];
	tile.SetPosition(sf::Vector2f(x * 160 + 80, y * 160 + 80));

	for (int i = 0; i < m_Tiles.size(); ++i)
	{
		sf::Vector2f vTilePosition = m_Tiles[i].GetPosition();
		if (vTilePosition.x == tile.GetPosition().x && vTilePosition.y == tile.GetPosition().y)
		{
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			return;
		}
	}
	Entity& newTile = m_Tiles.emplace_back(m_TileMapTexture,
		Entity::PhysicsData(Entity::PhysicsData::Shape::Rectangle,Entity::PhysicsData::Type::Static,0.0f,160.0f,160.0f));
	newTile.SetSprite(tile.GetSprite());
}

void Game::DeleteTileAtPosition(const sf::Vector2f& position)
{
	int x = position.x / 160;
	int y = position.y / 160;
	sf::Vector2f vTilePosition(x * 160 + 80, y * 160 + 80);
	for (int i = 0; i < m_Tiles.size(); ++i)
	{
		sf::Vector2f vCurrentTilePosition = m_Tiles[i].GetPosition();
		if (vCurrentTilePosition.x == vTilePosition.x && vCurrentTilePosition.y == vTilePosition.y)
		{
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			return;
		}
	}
}

