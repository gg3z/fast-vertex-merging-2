#pragma once

#include "mesh.h"
#include "meshfactory.hpp"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <array>

using namespace std;

#define DEBUG 1

struct TriangleRecord
{
    array<float,3> normal;
    array<array<float,3>,3> v;     // 3 vertices
    uint16_t attr;
};

class stlreader : public MeshFactory
{
public:
    stlreader(string fname)
    {
        fin.open(fname);
    }
    shared_ptr<Mesh> make()
    {
        if( !fin.is_open() )
            return nullptr;
        char header[80];
        fin.read(header, 80);
        if (!fin)
        {
            std::cerr << "File too short for STL header.\n";
            return nullptr;
        }
        uint32_t num_tris = 0;
        fin.read(reinterpret_cast<char*>(&num_tris), 4);
        if (!fin)
        {
            std::cerr << "File too short for triangle count.\n";
            return nullptr;
        }
#if DEBUG
        cout << "num triangles = " << num_tris << endl;
#endif

        TriangleRecord rec;
        Mesh mesh;

        std::uint8_t buf[50];
        uint32_t num_verts = static_cast<uint32_t>(num_tris) * 3;
        mesh.V.resize(num_verts);
        mesh.F.reserve(num_tris);
        for (uint32_t it = 0; it < num_tris; ++it)
        {
            fin.read(reinterpret_cast<char*>(buf), 50);
            if (!fin)
            {
                std::cerr << "Unexpected EOF at triangle " << it << '\n';
                return nullptr;
            }
            float* f = reinterpret_cast<float*>(buf);
            for( int iv = 0; iv < 3; ++iv )
            {
                auto ix = 3*(1 + iv);
                mesh.V[3*it + iv].p = {f[ix], f[ix + 1], f[ix + 2]};
            }
            mesh.F.emplace_back(Face{3*it, 3*it+1, 3*it+2});
        }
        return make_shared<Mesh>(mesh);
    }
};
