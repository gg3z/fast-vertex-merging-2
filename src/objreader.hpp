#pragma once

#include "meshfactory.hpp"

#include <string>

using namespace std;

class objreader : public MeshFactory
{
    objreader(string fname)
    {
        fin.open(fname);
    }
    shared_ptr<Mesh> make()
    {
        if( !fin.is_open() )
            return nullptr;
        return nullptr;
    }
};