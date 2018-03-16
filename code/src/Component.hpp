#pragma once
#ifndef COMPONENT_H_
#define COMPONENT_H_
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <common.h>

#define GATHER_SINGLE_COMPONENT(_PT, _TYPE, _SRC) (   (_PT) = (  ( (_PT) != NULL || dynamic_cast<_TYPE>((_SRC)) == NULL || !((_SRC)->isSweepable) || !((_SRC)->isActive) ) ? (_PT) : static_cast<_TYPE>((_SRC))  )   )
#define CATCH_COMPONENT(_PT, _TYPE, _SRC) ( (_PT) = (  !((_SRC)->isSweepable) || !((_SRC)->isActive)  ) ? NULL : dynamic_cast<_TYPE>((_SRC)) )

#define GATHER_HIDDEN_SINGLE_COMPONENT(_PT, _TYPE, _SRC) (   (_PT) = (  ( (_PT) != NULL || dynamic_cast<_TYPE>((_SRC)) == NULL || !((_SRC)->isActive) ) ? (_PT) : static_cast<_TYPE>((_SRC))  )   )
#define CATCH_HIDDEN_COMPONENT(_PT, _TYPE, _SRC) ( (_PT) = ( !((_SRC)->isActive) ? NULL : dynamic_cast<_TYPE>((_SRC)) )   )

using namespace std;

class Component{
 public:
  virtual ~Component(){}
  // A switch to enable/disable the component.
  bool isActive = true;
  // False marks that this component should be treated as inactive by any standard sweep through the scene graph. Probably so that it can be handled as a special case. 
  bool isSweepable = true;
  
  
};

#endif