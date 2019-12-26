#pragma once

#include <ECS/Entities/Entity.h>

namespace cyd
{
void Entity::addComponent( ComponentType type, BaseComponent* pComponent )
{
   m_components[type] = pComponent;
}

void Entity::removeComponent( ComponentType type ) { m_components[type] = nullptr; }
}
