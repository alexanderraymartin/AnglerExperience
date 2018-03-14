#include "Spawner.h"

using namespace glm;
using namespace std;

void Spawner::update(GameState &state, float dt) {
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

void Spawner::spawn(GameState &state) {
	state.activeScene->addEntity((*newFish)(location));
}

void SpawnSystem::update(ApplicationState &appstate, GameState &gstate, float dt) {
	for (pair<const Entity*, Entity*> entpair : gstate.activeScene->entities) {
		Spawner* spawner = nullptr;
		for (Component *cmpnt : entpair.second->components) {
			GATHER_SINGLE_COMPONENT(spawner, Spawner*, cmpnt);
			if (spawner) {
				spawner->update(gstate, dt);
			}
		}
	}
}