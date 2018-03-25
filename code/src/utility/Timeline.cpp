#include <stdlib.h>
#include <stdio.h>
#include "common.h"
#include "Timeline.hpp"

using namespace std;

#define ERRSOURCE() fprintf(stderr, "Timeline not been given a source of time!\n")
#define BADSOURCE() (timeSource == NULL)

template <typename T>
Timeline<T>::Timeline(T (*sourcefunc)()){
  timeSource = sourcefunc;
  start = timeSource();
  lastTime = 0.0;
  lastElapsed = 0.0;
  pausetime = 0.0;
}
template <typename T>
Timeline<T>::Timeline(){
  start = 0.0;
  lastTime = 0.0;
  lastElapsed = 0.0;
  pausetime = 0.0;
}

template <typename T>
void Timeline<T>::setSource(T (*timeSource)()){
  timeSource = timeSource;
}

template <typename T>
T Timeline<T>::getTime(){
  if(BADSOURCE()){
    ERRSOURCE();
    return(-1);
  }
  if(paused){
    return(pausetime);
  }else{
    lastTime = timeSource()-start + pausetime;
  }
  return(lastTime);
}

template <typename T>
T Timeline<T>::elapsed(){
  if(BADSOURCE()){
    ERRSOURCE();
    return(-1);
  }
  if(paused){
    return(0.0);
  }else{
    T tmp = timeSource()-lastElapsed;
    lastElapsed = timeSource();
    return(tmp);
  }
}

template <typename T>
T Timeline<T>::reset(){
  if(BADSOURCE()){
    ERRSOURCE();
    return(-1.0);
  }
  start = timeSource();
  lastTime = start;
  return(start);
}

template <typename T>
void Timeline<T>::pause(){
  if(BADSOURCE()){
    ERRSOURCE();
    return;
  }
  paused = true;pausetime = timeSource()-start + pausetime;
}

template <typename T>
void Timeline<T>::unpause(){
  if(BADSOURCE()){
    ERRSOURCE();
    return;
  } 
  paused = false;reset();
}

template <typename T>
void Timeline<T>::togglePause(){
  if(BADSOURCE()){
    ERRSOURCE();
  return;
  }
  if(paused) unpause(); else pause();
}


// I don't remember why this is here? Does it need to be?
template class Timeline<float>;
template class Timeline<double>;