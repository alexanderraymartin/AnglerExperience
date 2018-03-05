#pragma once
#ifndef ENTITY_H_
#define ENTITY_H_
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <common.h>

#include "Component.hpp"

using namespace std;

class Entity{
 public:
  Entity() : parent(NULL) {};
  ~Entity(){};

  virtual void attach(Component *component){
    components.push_back(component);
  }
  virtual void addChild(Entity *child){
    children.push_back(child);
  }

  vector<Entity*> children;
  vector<Component*> components;
  Entity* parent; // Parent Entity. Same as scene if it has no parent.

  bool isActive = true;

};

#endif