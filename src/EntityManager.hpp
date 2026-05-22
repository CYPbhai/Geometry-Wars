#pragma once
#include <map>
using EntityVec = std::vector<std::shared_ptr<Entity>>;
using EntityMap = std::map<std::string, EntityVec>;

class EntityManager
{
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	EntityMap m_entityMap;
	size_t    m_totalEntities = 0;

	void removeDeadEntities(EntityVec& entities)
	{
		entities.erase(
			std::remove_if(entities.begin(),entities.end(),
				[](const std::shared_ptr<Entity>& e) { return !e->isAlive(); }
			),
			entities.end()
		);
	}
public:
	EntityManager() = default;

	void update()
	{
		for (auto e : m_entitiesToAdd)
		{
			m_entities.push_back(e);
			m_entityMap[e->tag()].push_back(e);
		}

		m_entitiesToAdd.clear();

		removeDeadEntities(m_entities);

		for(auto& [tag, entityVec] : m_entityMap)
		{
			removeDeadEntities(entityVec);
		}
	}

	std::shared_ptr<Entity> addEntity(std::string tag)
	{
		auto entity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
		m_entitiesToAdd.push_back(entity);
		return entity;
	}

	const EntityVec& getEntities() const
	{
		return m_entities;
	}

	const EntityVec& getEntities(const std::string& tag) 
	{
		return m_entityMap[tag];
	}

	const EntityMap& getEntityMap() const
	{
		return m_entityMap;
	}

	void clearAll()
	{
		m_entities.clear();
		m_entitiesToAdd.clear();
		m_entityMap.clear();
		m_totalEntities = 0;
	}
};