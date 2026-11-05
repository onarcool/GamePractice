#ifndef ENTITY_H
#define ENTITY_H

#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/RenderTarget.hpp"
#include "SFML/Graphics/Drawable.hpp"

#include <vector>

class Entity : public sf::Drawable
{
public:
	struct PhysicsData
	{
		enum class Shape
		{
			Circle,
			Rectangle
		};

		enum class Type
		{
			Static,
			Dynamic
		};

		PhysicsData(
			Shape shape = Shape::Circle,
			Type type = Type::Static,
			float radius = 0.0f,
			float width = 0.0f,
			float height = 0.0f,
			sf::Vector2f velocity = sf::Vector2f(0.0f, 0.0f))
			: m_eShape(shape)
			, m_eType(type)
			, m_fRadius(radius)
			, m_fWidth(width)
			, m_fHeight(height)
			, m_vVelocity(velocity)
		{
		}

		Shape m_eShape;
		Type m_eType;

		float m_fRadius;
		float m_fWidth;
		float m_fHeight;

		sf::Vector2f m_vVelocity;
		std::vector<Entity*> m_EntitiesToIgnore;
	};

	explicit Entity(const sf::Texture& texture, const PhysicsData& ePhysicsType = PhysicsData())
		: m_Sprite(texture), m_PhysicsData(ePhysicsType)
	{
	}
	~Entity() = default;

	void SetCirclePhysics(float radius)
	{
		m_PhysicsData.m_eShape = PhysicsData::Shape::Circle;
		m_PhysicsData.m_fRadius = radius;
	}

	void SetRectanglePhysics(float width, float height)
	{
		m_PhysicsData.m_eShape = PhysicsData::Shape::Rectangle;
		m_PhysicsData.m_fWidth = width;
		m_PhysicsData.m_fHeight = height;
	}

	void AddEntityToIgnore(Entity* entity)
	{
		m_PhysicsData.m_EntitiesToIgnore.push_back(entity);
	}

	bool ShouldIgnoreEntityForPhysics(Entity* entity)
	{
		for (Entity* ignoredEntity : m_PhysicsData.m_EntitiesToIgnore)
		{
			if (ignoredEntity == entity)
			{
				return true;
			}
		}
		return false;
	}

	void SetVelocity(const sf::Vector2f& velocity)
	{
		m_PhysicsData.m_vVelocity = velocity;
	}

	void SetTexture(const sf::Texture& texture) 
	{
		m_Sprite.setTexture(texture); 
	}

	void SetScale(const sf::Vector2f& scale)
	{
		m_Sprite.setScale(scale);
	}

	void SetOrigin(const sf::Vector2f& origin)
	{
		m_Sprite.setOrigin(origin);
	}

	void SetPosition(const sf::Vector2f& position)
	{
		m_Sprite.setPosition(position);
	}

	void SetSprite(const sf::Sprite& sprite)
	{
		m_Sprite = sprite;
	}

	void GetSprite(sf::Sprite& sprite) const
	{
		sprite = m_Sprite;
	}

	const sf::Sprite& GetSprite() const
	{
		return m_Sprite;
	}

	sf::Vector2f GetPosition() const
	{
		return m_Sprite.getPosition();
	}

	void Move(const sf::Vector2f& offset)
	{
		m_Sprite.move(offset);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(m_Sprite, states);
	}

	void SetTextureRect(const sf::IntRect& rect)
	{
		m_Sprite.setTextureRect(rect);
	}

	PhysicsData GetPhysicsData() const
	{
		return m_PhysicsData;
	}


private:
	sf::Sprite m_Sprite;
	PhysicsData m_PhysicsData;
};

#endif // !ENTITY_H