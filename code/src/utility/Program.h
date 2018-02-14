#pragma  once
#ifndef __Program__
#define __Program__

#include <unordered_map>
#include <istream>
#include <string>
#include <json.hpp>

#define GLEW_STATIC
#include <GL/glew.h>

using namespace std;
using json = nlohmann::json;

class Program{
public:
  // Creates empty program object
  Program();

  // Same as calling builFromJSONArray
  Program(const json &program_obj);

  Program(istream &vertex, istream &fragment);

  Program(const string &vpath, const string &fpath);
  
  virtual ~Program();
  
  void setVerbose(bool v) { verbose = v; }
  bool isVerbose() const { return verbose; }

  GLuint getPID() const { return(pid); }


  // Note: istream is used as the interface here so that the functions can be more flexible. 
  // To load from a file an ifstream can be given. To load a string use istringstream, ect...
  
  // Build the classic vertex shader -> fragment shader program from two GLSL sources
  // Returns true if program compiled and linked properly false otherwise
  bool buildVsFsProgram(istream &vertex, istream &fragment);

  // Build from a JSON array containing shader specifications. Program will be linked in order given. 
  // Structure should be as follows
  /* 
  [
    {
      "name":"shaderName"
      "type": "vertex | fragment | geometry | compute"
      "attributes": [["float", "list"], ["vec2", "of"], ["mat4", "attributes"]]
      "uniforms": [["sampler2D", "list"], ["int", "of"], ["vec3", "uniforms"]]
      "src": "#version 330 core\n // Long line of the GLSL code"
    },

    {
      "name":"otherShaderName"
      "type": "vertex | fragment | geometry | compute"
      "attributes": [["float", "list"], ["vec2", "of"], ["mat4", "attributes"]]
      "uniforms": [["sampler2D", "list"], ["int", "of"], ["vec3", "uniforms"]]
      "src": "#version 330 core\n // Long line of the GLSL code"
    }
  ]
  */
  // Returns true if program compiled and linked properly false otherwise
  bool buildFromJsonArray(const json &program_obj);
  
  virtual void bind();
  virtual void unbind();

  GLint getAttribute(const string &name);
  GLint getUniform(const string &name);
  
private:
  GLuint pid;
  unordered_map<string,GLint> attributes;
  unordered_map<string,GLint> uniforms;
  bool verbose;
};

#endif
