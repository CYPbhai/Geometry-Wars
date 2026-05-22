#pragma once

#include "Components.hpp"

class EntityManager;

using ComponentTuple = std::tuple <
	CTransform,
	CShape,
	CCollision,
	CInput,
	CScore,
	CLifeSpan
>;

class Entity
{
	ComponentTuple m_components;
	bool m_alive = true;
	std::string m_tag = "default";
	size_t m_id = 0;
public:	
	Entity() = default;
	Entity(size_t id, const std::string& tag)
		: m_id(id)
		, m_tag(tag)
	{
	}
	template<typename T>
	T& get()
	{
		return std::get<T>(m_components);
	}

	template<typename T>
	const T& get() const
	{
		return std::get<T>(m_components);
	}

	template<typename T>
	bool has() const
	{
		return get<T>().exists;
	}

	template<typename T>
	void remove()
	{
		get<T>() = T();
	}

	template<typename T, typename... TArgs>
	T& add(TArgs&&... args)
	{
		auto& component = std::get<T>(m_components);
		component = T(std::forward<TArgs>(args)...);
		component.exists = true;
		return component;
	}

	size_t id() const
	{
		return m_id;
	}
	bool isAlive() const
	{
		return m_alive;
	}
	void destroy()
	{
		m_alive = false;
	}
	const std::string& tag() const
	{
		return m_tag;
	}
};