#include "Program.h"

#include <iostream>
#include <fstream>
#include <cassert>

#include "GLSL.h"
#include "common.h"

using namespace std;

Program::Program() :
  pid(0),
  verbose(true)
{
  
}

Program::Program(const json &program_obj) : pid(0), verbose(true){
  if(!buildFromJsonArray(program_obj)){
    fprintf(stderr, "Warning!: Failed building program from json given in constructor.\n");
  }
}

Program::Program(istream &vertex, istream &fragment){
  if(!buildVsFsProgram(vertex, fragment)){
    fprintf(stderr, "Warning!: Failed building VsFs program given in constructor.\n");
  }
}

Program::Program(const string &vpath, const string &fpath){
  ifstream vertex = ifstream(vpath);
  ifstream fragment = ifstream(fpath);
  if(!vertex.is_open()){
    fprintf(stderr, "Warning!: Couldn't open given shader file: ");
    cerr << vpath << endl;
  }else if(!fragment.is_open()){
    fprintf(stderr, "Warning!: Couldn't open given shader file: ");
    cerr << fpath << endl;
  }
  if(!buildVsFsProgram(vertex, fragment)){
    fprintf(stderr, "Warning!: Failed building VsFs program given in constructor.\n");
  }
}

Program::~Program()
{
  
}


bool Program::buildVsFsProgram(istream &vertex, istream &fragment){  
  // Create shader handles
  GLuint VS = glCreateShader(GL_VERTEX_SHADER);
  GLuint FS = glCreateShader(GL_FRAGMENT_SHADER);

  string vshader;
  string fshader;

  // Read shader sources
  // Get number of chars in each source
  vertex.seekg(0, ios::end);
  fragment.seekg(0, ios::end);
  vshader.reserve((size_t) vertex.tellg());
  fshader.reserve((size_t) fragment.tellg());
  vertex.seekg(ios::beg);
  fragment.seekg(ios::beg);

  vshader.assign((std::istreambuf_iterator<char>(vertex)), std::istreambuf_iterator<char>());
  fshader.assign((std::istreambuf_iterator<char>(fragment)), std::istreambuf_iterator<char>());

  //Allocate char* for each source then copy
  const char* vshaderpt = vshader.c_str();
  const char* fshaderpt = fshader.c_str();
  
  glShaderSource(VS, 1, &vshaderpt, NULL);
  glShaderSource(FS, 1, &fshaderpt, NULL);
  
  // Compile shaders
  if(!GLSL::compileAndCheck(VS, verbose)){
    fprintf(stderr,"Compiling vertex shader failed!\n");
    return(false);
  }
  if(!GLSL::compileAndCheck(FS, verbose)){
    fprintf(stderr,"Compiling fragment shader failed!\n");
    return(false);
  }
  
  
  // Create the program and link
  pid = glCreateProgram();
  glAttachShader(pid, VS);
  glAttachShader(pid, FS);
  if(!GLSL::linkAndCheck(pid, verbose)){
    fprintf(stderr,"Linking shader failed!\n");
    return(false);
  }
  
  GLSL::checkError(GET_FILE_LINE);
  return true;
}

// Used to select shader type from json
static map<string, GLenum> typemap = {
  {"vertex", GL_VERTEX_SHADER},
  {"fragment", GL_FRAGMENT_SHADER},
  {"geometry", GL_GEOMETRY_SHADER},
  {"compute", GL_COMPUTE_SHADER}
};
bool Program::buildFromJsonArray(const json &program_obj){
  vector<GLuint> component_shaders;
  vector<const char*> prog_attributes;
  vector<const char*> prog_uniforms;

  // Iterate through JSON array and compile each individual component shader
  for(auto &j : program_obj){
    GLuint cshad = glCreateShader(typemap[j["type"].get<string>()]);
    string src = j["src"].get<string>().c_str();
    const char* srcptr = src.c_str();
    glShaderSource(cshad, 1, &srcptr, NULL);
    if(!GLSL::compileAndCheck(cshad, verbose)){
      cerr << "Compiling shader " << j["basename"].get<string>() << " in JSON array failed!\n";
      return(false);
    }

    // Add attributes and uniforms to vector 

    if(j.find("attributes") != j.end()){
      for(auto &attr : j["attributes"]){
          prog_attributes.push_back(attr[1].get<string>().c_str());
        }
    }
    if(j.find("uniforms") != j.end()){
      for(auto &unif : j["uniforms"]){
        prog_uniforms.push_back(unif[1].get<string>().c_str());
      }
    }
    component_shaders.push_back(cshad);
  }


  // Attatch all given shaders
  pid = glCreateProgram();
  for(GLuint &shader : component_shaders){
    glAttachShader(pid, shader);
  }

  // Link the program
  if(!GLSL::linkAndCheck(pid, verbose)){
    fprintf(stderr,"Linking JSON shader failed!\n");
    return(false);
  }

  GLSL::checkError(GET_FILE_LINE);

  return(true);
}

void Program::bind()
{
  glUseProgram(pid);
}

void Program::unbind()
{
  glUseProgram(0);
}

GLint Program::getAttribute(const string &name)
{
  unordered_map<string,GLint>::const_iterator attr = attributes.find(name.c_str());
  if(attr == attributes.end()) {
    GLint id = GLSL::getAttribLocation(pid, name.c_str(), verbose);
    if(id < 0){
      if(verbose){
        fprintf(stderr, "Attribute %s could not be found!\n", name.c_str());
      }
      return(-1);
    }else{
      attributes[name] = id;
      return(id);
    }
  }else{
    return attr->second;
  }
}

GLint Program::getUniform(const string &name)
{
  unordered_map<string,GLint>::const_iterator unif = uniforms.find(name.c_str());
  if(unif == uniforms.end()) {
    GLint id = GLSL::getUniformLocation(pid, name.c_str(), verbose);
    if(id < 0){
      if(verbose){
        fprintf(stderr, "Uniform %s could not be found!\n", name.c_str());
      }
      return(-1);
    }else{
      uniforms[name] = id;
      return(id);
    }
  }else{
    return unif->second;
  }
}

bool Program::hasAttribute(const string &name){
  unordered_map<string,GLint>::const_iterator attr = attributes.find(name.c_str());
  if(attr == attributes.end()) {
    GLint id = GLSL::getAttribLocation(pid, name.c_str(), verbose);
    if(id < 0){
      return(false);
    }else{
      attributes[name] = id;
      return(true);
    }
  }else{
    return(true);
  }
}

bool Program::hasUniform(const string &name){
  unordered_map<string,GLint>::const_iterator unif = uniforms.find(name.c_str());
  if(unif == uniforms.end()) {
    GLint id = GLSL::getUniformLocation(pid, name.c_str(), verbose);
    if(id < 0){
      return(false);
    }else{
      uniforms[name] = id;
      return(true);
    }
  }else{
    return(true);
  }
}