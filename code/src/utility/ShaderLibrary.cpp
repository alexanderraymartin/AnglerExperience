#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include "ShaderLibrary.hpp"
#include "Program.h"
#include "common.h"

using namespace std;

void ShaderLibrary::init(){
  istringstream evs(errorvs);
  istringstream efs(errorfs);

  if(!fallback.buildVsFsProgram(&evs,&efs)){
    exit(1212);
  }
  fallback.setVerbose(false);
  fallback.bind();
  active = &fallback;
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

Program& ShaderLibrary::getActive(){

  if(active == &fallback){
    fprintf(stderr,"Warning! Returning error shader from ShaderLibrary.getActive()!\n");
  }
  return(*active);
}

Program* ShaderLibrary::getActivePtr(){
  return(active);
}

Program* ShaderLibrary::getPtr(const string& name){
  if(programs.find(name) == programs.end()){
    fprintf(stderr, "Warning! Could not fetch shader pointer!\n");
    return(NULL);
  }else{
    return(programs[name]);
  }
}