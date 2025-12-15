#include "mesh.h"
#include "stlreader.hpp"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>

using namespace std;

int main(int argc, char* argv[])
{
  static_assert(sizeof(std::array<float,3>) == 3 * sizeof(float), "unexpected padding");
  static_assert(alignof(std::array<float,3>) == alignof(float), "unexpected alignment");

  assert(test_randomize("mesh_ex2.dat"));
  
  // stlreader r("tello_2.stl");
  // stlreader r("Spanner-stl.stl");
  stlreader r("Fairing.stl");
  auto local_mesh = r.make();
  cout << "num vertices - original = " << local_mesh->V.size() << endl;
  // local_mesh->to_OBJ("tello_2.obj");
  local_mesh->mergeDupVerts(1.e-6);
  cout << "num vertices - cleaned = " << local_mesh->V.size() << endl;
  // local_mesh->to_OBJ("tello_2_dedup_verts.obj");
  // if( !local_mesh->to_PLYbin("tello_2_dedup_verts.ply") )
  // if( !local_mesh->to_PLYbin("Spanner-ply-dedup-v.ply") )
  local_mesh->to_PLYbin("Fairing-dedup-v.ply");
  return 0;
}