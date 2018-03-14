#include "Spawner.h"

using namespace glm;
using namespace std;

void Spawner::update(GameState* state, float dt) {
	if (shouldSpawn(dt)) {
		spawn(state);
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

void Spawnable::spawn(vec3 location, GameState* state) {
	state->activeScene->addEntity((*newFish)(location));
}