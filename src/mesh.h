#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <cmath>
#include <cstdlib>
#include <memory>

using namespace std;

using Id = uint32_t;

struct Vertex {
    array<float,3> p{0,0,0};
};

struct Face {
    array<Id, 3> v;
};

struct GridHash
{
    unordered_map<int64_t, vector<int>> buckets;
    double s; // cell size

    int64_t pack(array<int64_t,3> ixyz) const
    {
        // simple 3D Morton-like pack
        const uint64_t A = 73856093, B = 19349663, C = 83492791; // coprime-ish
        return ixyz[0]*A ^ ixyz[1]*B ^ ixyz[2]*C;
    }
    GridHash(double cell):s(cell){}
    
    void insert(const array<float,3>& p, int idx)
    {
        array<int64_t,3> ixyz = {static_cast<int64_t>(llround(floorl(p[0]/s))), static_cast<int64_t>(llround(floorl(p[1]/s))), static_cast<int64_t>(llround(floorl(p[2]/s)))};
        buckets[pack(ixyz)].push_back(idx);
    }

    void neighbors(const array<float,3>& p, vector<int>& out) const
    {
        // out contains all vertex indices that are neighbors of p

        out.clear();
        array<int64_t,3> ixyz = {static_cast<int64_t>(llround(floorl(p[0]/s))-1), static_cast<int64_t>(llround(floorl(p[1]/s))-1), static_cast<int64_t>(llround(floorl(p[2]/s))-1)};
        for( int64_t k = 0; k < 27; k++ )
        {
            array<int64_t,3> neighbor = { ixyz[0]+k%3, ixyz[1]+(k/3)%3, ixyz[2]+k/9 };
            auto it = buckets.find(pack(neighbor));
            if(it!=buckets.end())
                out.insert(out.end(), it->second.begin(), it->second.end());
        }
    }
};

struct Mesh {
    vector<Vertex> V;
    vector<Face>   F; 
    int read(const string& filename);
    int readSTL(const string& filename);
    bool write(const string& filename) const;
    void to_OBJ(const string& filename) const;
    void to_PLY(const string& filename) const;
    void to_PLYbin(const string& filename) const;
    int mergeDupVerts(double tol = 1.e-10);
    void randomizeVertices(uint32_t numPerVtx, double half_len = 0.001);
    void roundValues(int digits = 0);
};


int consolidate(vector<Vertex>&, double tol, vector<uint32_t> &old_to_new);
vector<array<double,3>> uniform_gen(uint64_t numSamples, const array<double,3> &ctr, double half_side);
bool test_merge(string fname);
bool test_randomize(string fname);