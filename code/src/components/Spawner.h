#pragma once
#ifndef SPAWNER_H_
#define SPAWNER_H_

#include "SimpleComponents.hpp"
#include "../GameState.hpp"


class Spawner : public Component {
public:

	Spawner(glm::vec3 location, Entity* (*fishFunc)(glm::vec3 location)) : location(location), newFish(fishFunc) {}
	Spawner(glm::vec3 location, Entity* (*fishFunc)(glm::vec3 location), float spawnTime) : location(location), newFish(fishFunc), timePerSpawn(spawnTime) {}

	void update(GameState &state, float dt);

	bool shouldSpawn(float dt);

	void spawn(GameState &state);

protected:
	glm::vec3 location;
	Entity* (*newFish)(glm::vec3 location);
	float cumTime = 0;
	float timePerSpawn = 1.0f;
};

namespace SpawnSystem {
	void update(ApplicationState &appstate, GameState &gstate, float dt);
}






#endif