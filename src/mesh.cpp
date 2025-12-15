#include "mesh.h"

#include <array>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <unordered_map>
#include <random>
#include <chrono>
#include <stdexcept>

using namespace std;

#define DEBUG 0
#define PROFILE 1 // TODO: use a decorator to separate profiling code from functionality

vector<array<float,3>> uniform_gen(uint64_t numSamples, const array<float,3> &ctr, double half_side)
{
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<float> disx(ctr[0] - half_side, ctr[0] + half_side);
    uniform_real_distribution<float> disy(ctr[1] - half_side, ctr[1] + half_side);
    uniform_real_distribution<float> disz(ctr[2] - half_side, ctr[2] + half_side);
    vector<array<float,3>> vs;
    for( int k = 0; k < numSamples; ++k )
        vs.push_back({disx(gen), disy(gen), disz(gen)});
    return vs;
}

void Mesh::roundValues(int digits)
{
    for( auto &v : V )
    {
        for( int k = 0; k < 3; ++k )
        {
            v.p[k] = round(v.p[k]);
            if( abs(v.p[k]) < 1.*pow(10, -digits) )
                v.p[k] = 0.;
        }
    }
}

int Mesh::read(const string& filename)
{
    // read vertex locations and triangle indices from a file
    // return -1 if file cannot be opened
    // return 0 if successful
    // return 1 if error reading vertices
    // return 2 if error reading faces
    // existing mesh data will be cleared

    V.clear();
    F.clear();
    ifstream fin(filename);
    if (!fin)
    {
        return -1;
    }
    string line;
    int nV = 0, nF = 0;
    while (getline(fin, line))
    {
        if (line.empty() || line[0] == '#') continue;
        if( nV == 0 || nF == 0 )
        {
            stringstream ss(line);
            ss >> nV >> nF;
            break;
        }
    }
    // read vertices
    while( nV > 0 && getline(fin, line) )
    {
        if (line.empty() || line[0] == '#') continue;
        stringstream ss(line);
        Vertex v;
        ss >> v.p[0] >> v.p[1] >> v.p[2];
        V.emplace_back(v);
        nV--;
    }
    if( nV != 0 ) return 1;
    // read faces
    F.reserve(nF);
    while( nF > 0 && getline(fin, line) )
    {
        if (line.empty() || line[0] == '#') continue;
        Face face;
        stringstream ss(line);
        ss >> face.v[0] >> face.v[1] >> face.v[2];
        F.emplace_back(face);
        nF--;
    }
    if( nF != 0 ) return 2;
    return 0;
}

bool Mesh::write(const string& filename) const
{
    ofstream fout(filename);
    if (!fout)
    {
        cerr << "Error: cannot open output file " << filename << endl;
        return false;
    }
    // write vertices
    fout << "# Cleaned mesh\n";
    fout << V.size() << " " << F.size() << "\n";
    fout << "# Vertices\n";
    for( const auto& v : V )
    {
        fout << v.p[0] << " " << v.p[1] << " " << v.p[2] << "\n";
    }
    // write faces
    fout << "# Faces\n";
    for( const auto& f : F )
    {
        fout << f.v[0] << " " << f.v[1] << " " << f.v[2] << "\n";
    }
    return true;
}

int Mesh::mergeDupVerts(double tol)
{
    vector<uint32_t> o2n;
#if PROFILE
    const auto start = std::chrono::steady_clock::now();
#endif

    int nNewInds = consolidate(V, tol, o2n);

#if PROFILE
    const auto end = std::chrono::steady_clock::now();
    chrono::milliseconds merge_time = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Time to merge duplicate vertices = " << merge_time << " ms" << endl;
#endif
#if DEBUG
    int iv = 0;
    for( const auto &v : V )
    {
        cout << "(" << v.p[0] << "," << v.p[1] << "," << v.p[2] << ")\n";
    }
    for( auto n : o2n )
    {
        cout << iv++ << " --> " << n << endl;
    }
#endif
    // update face indices
    for( auto &f : F )
    {
        f.v[0] = o2n[f.v[0]];
        f.v[1] = o2n[f.v[1]];
        f.v[2] = o2n[f.v[2]];
    }
    return nNewInds;
}

void Mesh::randomizeVertices(uint32_t nPerVtx, double half_len)
{
    vector<Vertex> RV;
    for( const auto &v : V )
    {
        auto rand_pos = uniform_gen(nPerVtx, v.p, half_len);
        Vertex rv;
        for( const auto &p : rand_pos )
        {
            rv.p = p;
            RV.emplace_back(rv);
        }
    }
    V.swap(RV);
}

int consolidate(vector<Vertex>& V, double eps, vector<uint32_t>& old2new)
{
    const double cell = max(1e-12, eps);
    GridHash gh(cell);
    int n = (int)V.size();
    old2new.assign(n,-1);
    vector<int> neigh;
    int newCount=0;
    vector<Vertex> NV; NV.reserve(n);
    for(int i=0;i<n;++i)
    {
        const auto& p = V[i].p;
        gh.neighbors(p, neigh);
        int match=-1;
        for(int j: neigh)
        {
            if(old2new[j]==-1) continue; // neighbor not mapped yet? skip
            int rep = old2new[j];
            const array<float,3>& q = NV[rep].p;
            double dx=p[0]-q[0], dy=p[1]-q[1], dz=p[2]-q[2];
            if(dx*dx+dy*dy+dz*dz <= eps*eps){ match = rep; break; }
        }
        if(match==-1){
            match = newCount++;
            Vertex v;
            v.p = p;
            NV.push_back(v);
        }
        old2new[i]=match;
        gh.insert(p,i);
    }
    V.swap(NV);
    return newCount;
}

void Mesh::to_OBJ(const string &fname) const
{
    try
    {
        std::ofstream out(fname);
        if (!out) {
            throw std::runtime_error("Failed to open " + fname);
        }

        // Vertices
        for (const auto& v : V) {
            out << "v " << v.p[0] << ' ' << v.p[1] << ' ' << v.p[2] << '\n';
        }

        // Faces (OBJ is 1-based index)
        for (const auto& f : F) {
            out << "f "
                << (f.v[0] + 1) << ' '
                << (f.v[1] + 1) << ' '
                << (f.v[2] + 1) << '\n';
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void Mesh::to_PLYbin(const string &fname) const
{
    try
    {
        std::ofstream out(fname);
        if (!out) {
            throw std::runtime_error("Failed to open " + fname);
        }
        // --- ASCII header (must end with '\n' after end_header) ---
        out << "ply\n";
        out << "format binary_little_endian 1.0\n";
        out << "comment generated by my triangulation tool\n";
        out << "element vertex " << V.size() << "\n";
        out << "property float x\n";
        out << "property float y\n";
        out << "property float z\n";
        out << "element face " << F.size() << "\n";
        out << "property list uchar int vertex_indices\n";
        out << "end_header\n";

        // --- Binary vertex data: x, y, z as float (4 bytes each) ---
        for (const auto& v : V)
        {
            out.write(reinterpret_cast<const char*>(&v.p[0]), sizeof(float));
            out.write(reinterpret_cast<const char*>(&v.p[1]), sizeof(float));
            out.write(reinterpret_cast<const char*>(&v.p[2]), sizeof(float));
        }

        // --- Binary faces: [uchar 3][int i0][int i1][int i2] ---
        for (const auto& f : F)
        {
            std::uint8_t count = 3;
            out.write(reinterpret_cast<const char*>(&count), sizeof(std::uint8_t));

            // PLY 'int' is typically 32-bit signed; if you prefer, cast explicitly
            int32_t i0 = static_cast<std::int32_t>(f.v[0]);
            int32_t i1 = static_cast<std::int32_t>(f.v[1]);
            int32_t i2 = static_cast<std::int32_t>(f.v[2]);

            out.write(reinterpret_cast<const char*>(&i0), sizeof(int32_t));
            out.write(reinterpret_cast<const char*>(&i1), sizeof(int32_t));
            out.write(reinterpret_cast<const char*>(&i2), sizeof(int32_t));
        }

        if (!out)
        {
            throw std::runtime_error("Error while writing " + fname);
        }
    }
    catch (std::runtime_error e)
    {
        std::cerr << e.what() << std::endl; 
    }
}

bool test_merge(string fname)
{
    Mesh mesh;
    if( mesh.read(fname) > 0 )
    {
#if DEBUG
        cout << "file not found\n";
#endif
        return false;
    }
    string vmerged("merged_");
    vmerged += fname;
    int nV = mesh.V.size();
    const auto start = std::chrono::steady_clock::now();
    mesh.mergeDupVerts();
    const auto end = std::chrono::steady_clock::now();
    chrono::milliseconds merge_time = chrono::duration_cast<std::chrono::milliseconds>(end - start);
    cout << "Vertices merged from " << nV << " to " << mesh.V.size()  << " ; Time to merge = " << merge_time << " ms" << endl;
    mesh.write(vmerged);
    return true;
}

bool test_randomize(string fname)
{
    Mesh mesh;
    if( mesh.read(fname) > 0 )
    {
#if DEBUG
        cout << "file not found\n";
#endif
        return false;
    }
    for( int nR = 2; nR <= 4096; nR *= 2 )
    {
        mesh.randomizeVertices(nR, 0.2);
        string randomized("random_");
        randomized += to_string(nR);
        randomized += fname;
        mesh.write(randomized);
        int nV = mesh.V.size();
        const auto start = std::chrono::steady_clock::now();
        mesh.mergeDupVerts(0.5);
        const auto end = std::chrono::steady_clock::now();
        mesh.roundValues();
        chrono::microseconds merge_time = chrono::duration_cast<std::chrono::microseconds>(end - start);
        cout << "Vertices merged from " << nV << " to " << mesh.V.size()  << " ; Time to merge = " << merge_time << endl;
        string merged("merged_");
        merged += randomized;
        mesh.write(merged);
        // TODO: input and output files should be identical
    }
    return true; 
}