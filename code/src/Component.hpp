#pragma once
#ifndef COMPONENT_H_
#define COMPONENT_H_
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <common.h>

using namespace std;

// As best I can tell there is no (good) way to determine component type at runtime.
// Systems need this information in order to operate on the correct components
// Therefore we need to populate this enum each time a new component class is defined
enum class ComponentType{
  NONE,
  PHYSICS,
  CAMERA,
  SWARM,
  PARTICLEMANAGER,
  PARTICLE,
  ANIMATEABLE,
  CONSUMABLE
};

class Component{
 public:
  Component();
  virtual ~Component() = 0;

  // A switch to enable/disable the component.
  bool isActive = true;
  
  // See definition of ComponentType at top of file
  static const ComponentType component_type = ComponentType::NONE;
  
};

#endif