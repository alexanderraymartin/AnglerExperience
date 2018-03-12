#include "Spawner.h"


void Spawner::update(float dt) {
	if (shouldSpawn(dt)) {
		spawn();
	}
}

bool Spawner::shouldSpawn(float dt) {
	cumTime += dt;
	if (cumTime > timePerSpawn) {
		cumTime -= timePerSpawn;
		return true;
	}
	return false;
}