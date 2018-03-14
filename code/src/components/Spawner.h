#pragma once
#ifndef SPAWNER_H_
#define SPAWNER_H_

#include "SimpleComponents.hpp"
#include "../GameState.hpp"

class Spawner : public Component {
public:

	void update(GameState* state, float dt);

	bool shouldSpawn(float dt);

	void spawn(GameState* state) { offspring.spawn(location, state); }

protected:
	Spawnable offspring;
	glm::vec3 location;
	float cumTime = 0;
	float timePerSpawn = 1.0f;
};



class Spawnable : public Component {
public:
	//Overwrite me to make a new entity at location when called
	void spawn(glm::vec3 location, GameState* state);
	//copy entity
	//get pose
	//set pose to location
	//set velocity to head into the scene

	void setFishSpawnFunc(Entity* (*fishFunc)(glm::vec3 location)) { newFish = fishFunc; }

protected:
	Entity* (*newFish)(glm::vec3 location);
};



#endif