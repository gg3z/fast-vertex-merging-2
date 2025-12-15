#pragma once

#include <memory>

struct Mesh;

class MeshFactory
{
protected:
    ifstream fin;
public:
    virtual std::shared_ptr<Mesh> make() = 0;
    virtual ~MeshFactory() = default;
};