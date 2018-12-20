#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include "matrix.h"

struct Vertex {
    float pos[3];
    float norm[3];
    float tex[2];
    float color[3];
};

class Surface
{
public:
    Surface();
    Surface(Matrix<float> &mat);

    std::vector<Vertex> &vertices();
    std::vector<unsigned int> &indices();

    Vertex *data();
    unsigned int *indicesPtr();

    int size();

private:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

    float m_min, m_max;

    void fillBuffers(Matrix<float> &mat);
    void computeNormals();
    void computeColors();
    void normalizeDepth();

    float colormapRed(float x);
    float colormapGreen(float x);
    float colormapBlue(float x);
    float clamp(float x, float min, float max);
};

#endif // SURFACE_H
