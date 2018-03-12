#pragma once
#ifndef SPAWNER_H_
#define SPAWNER_H_

#include "SimpleComponents.hpp"
#include "../Entity.hpp"

class Spawner : public Component {
public:

	void update(float dt);

	bool shouldSpawn(float dt);

	void spawn() { offspring.spawn(location); }

protected:
	Spawnable offspring;
	glm::vec3 location;
	float cumTime = 0;
	float timePerSpawn = 1.0f;
};



class Spawnable : public Component {
public:
	//Overwrite me to make a new entity at location when called
	virtual void spawn(glm::vec3 location);
	//copy entity
	//get pose
	//set pose to location
	//set velocity to head into the scene

protected:
	Entity* entity;
};



#endif