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
  friend class System;
 public:
  Entity();
  virtual ~Entity() = 0;

  // Intended to be overridden by scene to enable use of a contiguous set of children. 
  virtual void addChildClone(const Entity& entity) = 0;

  // Make a clone of this Entity and it's children. If detach is true the returned 
  virtual Entity* clone(const Entity &original, bool detach = false) = 0;

  // Using getters to discourage processing entities outside of a System or other Entity
  virtual Entity* getParent(){return(parent);}
  virtual const vector<Entity*>& getChildren(){return(children);}
  virtual const vector<Component*>& getComponents(){return(components);}

  bool isActive = true;

 protected:
  vector<Entity*> children;
  vector<Component*> components;
  Entity* parent; // Parent Entity. Same as scene if it has no parent.

 private:

};

#endif