#include <stdlib.h>
#include <stdio.h>
#include "ShaderLibrary.hpp"
#include "Program.h"
#include "common.h"

using namespace std;

ShaderLibrary::ShaderLibrary(){
  // TODO: Initialize fallback with hardcoded glsl so it is never absent
  // Note: Program.h must be updated before this can happen.
  active = NULL; // <--- This is very bad and should be fixed asap
}

void ShaderLibrary::add(const string &name, Program* prog){
  programs[name] = prog;
}

void ShaderLibrary::makeActive(const string &name){
  unordered_map<string,Program*>::iterator iter;

  if((iter = programs.find(name)) == programs.end()){
    fprintf(stderr, "Invalid name for program %s\n", name.c_str());
    active = &fallback;
    return;
  }else if(iter->second != active){
    // This shader is not already active so switch. 
    active->unbind();
    active = iter->second;
    active->bind();
  }
}

void ShaderLibrary::fastActivate(Program* prog){
  if(prog == NULL){
    fprintf(stderr, "Tried to swap to null shader!\n");
    active = &fallback;
  }else if(prog != active){
    active->unbind();
    active = prog;
    active->bind();
  }
}

const Program& ShaderLibrary::getActive(){

  if(active == &fallback){
    fprintf(stderr,"Warning! Returning error shader from ShaderLibrary.getActive()!\n");
  }
  return(*active);
}

const Program* ShaderLibrary::getActivePtr(){
  return(active);
}

const Program* ShaderLibrary::getPtr(const string& name){
  if(programs.find(name) == programs.end()){
    fprintf(stderr, "Warning! Could not fetch shader pointer!\n");
    return(NULL);
  }else{
    return(programs[name]);
  }
}