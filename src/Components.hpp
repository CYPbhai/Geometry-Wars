#pragma once

#include "Vec2.hpp"
#include <SFML/Graphics.hpp>

class Component
{
public:
	bool exists = false;
};

class CTransform : public Component
{
public:
	Vec2f position = { 0.0f, 0.0f };
	Vec2f velocity = { 0.0f, 0.0f };
	Vec2f scale = { 1.0f, 1.0f };
	int angle = 0;

	CTransform() = default;

	CTransform(const Vec2f& pos, const Vec2f& vel, const Vec2f& s, int ang)
		: position(pos), velocity(vel), scale(s), angle(ang) { }
};

class CShape : public Component
{
public:
	sf::CircleShape circle;

	CShape() = default;
	CShape(float radius, size_t points, const sf::Color& fill, const sf::Color& outline, float thickness)
		: circle(radius, points)
	{
		circle.setFillColor(fill);
		circle.setOutlineColor(outline);
		circle.setOutlineThickness(thickness);
		circle.setOrigin({ radius, radius });
	}
};

class CCollision : public Component
{
public:
	float radius = 0.0f;

	CCollision() = default;
	CCollision(float r)
		: radius(r) {
	}
};

class CScore : public Component
{
public:
	int score = 0;

	CScore() = default;
	CScore(int s)
		: score(s) { }
};

class CLifeSpan : public Component
{
public:
	int lifespan = 0;
	int remaining = 0;
	CLifeSpan() = default;
	CLifeSpan(int totalLifespan)
		: lifespan(totalLifespan), remaining(totalLifespan) { }
};

class CInput : public Component
{
public:
	bool up = false;
	bool left = false;
	bool right = false;
	bool down = false;
	bool shoot = false;
	bool specialWeapon = false;

	CInput() = default;
};