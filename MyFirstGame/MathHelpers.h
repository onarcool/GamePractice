#pragma once

#include <SFML/Graphics.hpp>

namespace MathHelpers 
{
    float Length(const sf::Vector2f& rVector)
    {
        // Function for calculating length
        float fLength = sqrt(rVector.x * rVector.x + rVector.y * rVector.y);
        return fLength;
    }


    sf::Vector2f Normalize(const sf::Vector2f& rVector)
    {
		if (rVector.x == 0.0f && rVector.y == 0.0f)
		{
			return rVector;
		}

        // Function for normalizing
        float fLength = Length(rVector);
        sf::Vector2f vNormalizedVector(rVector.x / fLength, rVector.y / fLength);
        return vNormalizedVector;
    }
}

