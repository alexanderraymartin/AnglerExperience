#pragma once
#ifndef COMPONENT_H_
#define COMPONENT_H_
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <common.h>

using namespace std;

class Component{
 public:
  virtual ~Component(){}
  // A switch to enable/disable the component.
  bool isActive = true;
  
  
};

#endif