# fast-vertex-merging-2 (enhanced version of fast-vertex-merging)

This small c++ project implements an optimal 3D vertex merging algorithm (O(v+f) where v = number of vertices and f = number of triangles).
The underlying algorithm uses a grid hash based on Morton-like codes.

This code focuses on consolidating the vertices and their indices - the faces are not modified though their vertex indices will be updated.

Rationale: repeated vertices are a common occurence in scanned meshes, and especially in STL files, where repetition is guaranteed due to the format itself.
          Vertex consolidation is important:
          
          - to improve performance for downstream mesh operations
          - to facilitate further mesh cleanup operations, e.g. edge collapse, degenerate face removal
          - to reliably derive the expected topology and detect any topological issues


Example 2: a study of the execution time to merge meshes with duplicate vertices of increasingly large degree. These meshes are randomly generated and contain vertex clusters with a predefined diameter d. The grid hashing algorithm reduces each cluster to one vertex.
The code in this repository produces the following output (times are in microseconds)

Time to merge duplicate vertices = 0ms
Vertices merged from 16 to 8 ; Time to merge = 95us
Time to merge duplicate vertices = 0ms
Vertices merged from 32 to 8 ; Time to merge = 148us
Time to merge duplicate vertices = 0ms
Vertices merged from 64 to 8 ; Time to merge = 325us
Time to merge duplicate vertices = 0ms
Vertices merged from 128 to 8 ; Time to merge = 697us
Time to merge duplicate vertices = 1ms
Vertices merged from 256 to 8 ; Time to merge = 1341us
Time to merge duplicate vertices = 2ms
Vertices merged from 512 to 11 ; Time to merge = 2061us
Time to merge duplicate vertices = 8ms
Vertices merged from 1408 to 10 ; Time to merge = 8750us
Time to merge duplicate vertices = 10ms
Vertices merged from 2560 to 11 ; Time to merge = 10345us
Time to merge duplicate vertices = 24ms
Vertices merged from 5632 to 11 ; Time to merge = 24119us
Time to merge duplicate vertices = 47ms
Vertices merged from 11264 to 11 ; Time to merge = 47145us
Time to merge duplicate vertices = 125ms
Vertices merged from 22528 to 9 ; Time to merge = 125316us
Time to merge duplicate vertices = 178ms
Vertices merged from 36864 to 9 ; Time to merge = 178406us

The relationship between initial vertices and execution time is almost linear, as expected.

Example 3: Fairing.stl 
This is a real-world file with medium size and good meshing quality. The average vertex degree is almost exactly 6.

num triangles = 993958
num vertices - original = 2981874
Time to merge duplicate vertices = 10213ms (debug build)
num vertices - cleaned = 496959

TBD:
- random locations should be generated more "randomly"
- there is a bug in the merging of vertices (the final number of vertices should always be 8)
- performance can likely be improved
- code can be refactored with decorator and bridge patterns
