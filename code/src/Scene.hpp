#pragma once
#ifndef SCENE_H_
#define SCENE_H_

#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <queue>
#include <common.h>

#include "core.h"
#include "GameState.hpp"
#include "Entity.hpp"
#include "Component.hpp"
#include "SimpleComponents.hpp"

using namespace std;

class Scene{
 public:
  Scene() : entities(), killqueue() {}
  Scene(Camera* camera) : camera(camera){}
  ~Scene(){}

  void addEntity(Entity* entity){entities[entity] = entity;}

  void submitToKill(Entity* entity){killqueue.push(entity);}

  void executeKillQueue() {
    while (!killqueue.empty()) {
      entities.erase(killqueue.front());
      killqueue.pop();
    }
  }

  template<typename C>
  Entity* getFirstWithComponent(bool hidden = false);

  Camera* camera = NULL;

  // Unorderded map probably has some overhead compared to vector when it comes to iterating over 
  // the elements, but it should still be the same time complexity if we are always iterating on
  // the entire collection. It also makes removal easy and fast where as a vector would have to 
  // shift the array on each deletion. 
  // I know it's wierd to map the pointer to itself but I think it actually is the best way.
  // From what I understand most engines are the same way except they map based on a UUID
  unordered_map<const Entity*, Entity*> entities;
  
  queue<Entity*> killqueue;

  //Contiguous data goes below this line if needed

};

template<typename C>
Entity* Scene::getFirstWithComponent(bool hidden){
    C* component = nullptr;
    for(pair<const Entity*, Entity*> entpair : entities){
      for(Component *cmpnt : entpair.second->components){
        if(hidden){
          if(CATCH_HIDDEN_COMPONENT(component, C*, cmpnt)){
            return(entpair.second);
          }
        }else{
          if(CATCH_COMPONENT(component, C*, cmpnt)){
            return(entpair.second);
          }
        }
      }
    }
  }

#endif
