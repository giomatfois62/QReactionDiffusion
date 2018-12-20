#include "surface.h"

#include <limits>
#include <cmath>
#include <QDebug>

Surface::Surface()
{
    Matrix<float> mat(2,2);

    m_min = std::numeric_limits<float>::max();
    m_max = std::numeric_limits<float>::min();

    fillBuffers(mat);

    normalizeDepth();
    computeNormals();
    computeColors();
}

Surface::Surface(Matrix<float> &mat)
{
    m_min = std::numeric_limits<float>::max();
    m_max = std::numeric_limits<float>::min();

    qDebug() << ("Filling buffers\n");
    fillBuffers(mat);

    qDebug() << ("Computing normals and colors\n");
    normalizeDepth();
    computeNormals();
    computeColors();
}

std::vector<Vertex> &Surface::vertices()
{
    return m_vertices;
}

std::vector<unsigned int> &Surface::indices()
{
    return m_indices;
}

Vertex *Surface::data()
{
    return m_vertices.data();
}

unsigned int *Surface::indicesPtr()
{
    return m_indices.data();
}

int Surface::size()
{
    return m_vertices.size();
}

void Surface::fillBuffers(Matrix<float> &mat)
{
    float aspect = std::min(mat.rows / (float)mat.cols, mat.cols / (float)mat.rows);

    float x0, xstep, y0, ystep;
    if(mat.rows > mat.cols)
    {
        x0 = -aspect;
        y0 = -1.0f;
        xstep = 2.0f * aspect / (mat.cols - 1) ;
        ystep = 2.0f / (mat.rows - 1);
    }
    else
    {
        y0 = -aspect;
        x0 = -1.0f;
        ystep = 2.0f * aspect / (mat.cols - 1) ;
        xstep = 2.0f / (mat.rows - 1);
    }

    for(int i = 0; i < mat.rows; ++i)
    {
        for(int j = 0; j < mat.cols; ++j)
        {
            Vertex v;

            float x = x0 + j * xstep;
            float y = y0 + i * ystep;
            float z = mat(i,j);

            float xtex = 0.0f + j * 1.0f / (mat.cols - 1);
            float ytex = 0.0f + i * 1.0f / (mat.rows - 1);

            v.pos[0] = x;
            v.pos[1] = y;
            v.pos[2] = z;

            v.tex[0] = xtex;
            v.tex[1] = ytex;

            v.norm[0] = 0.0f;
            v.norm[1] = 0.0f;
            v.norm[2] = 0.0f;

            m_vertices.push_back(v);

            if(z > m_max)
                m_max = z;
            if(z < m_min)
                m_min = z;
        }
    }

    for(int i = 0; i < mat.rows - 1; ++i)
    {
        for(int j = 0; j < mat.cols - 1; ++j)
        {
            int current = i * mat.cols + j;
            int up = (i + 1) * mat.cols + j;

            m_indices.push_back(current);
            m_indices.push_back(current + 1);
            m_indices.push_back(up + 1);

            m_indices.push_back(current);
            m_indices.push_back(up + 1);
            m_indices.push_back(up);
        }
    }
}

void Surface::computeNormals()
{
    for(size_t j = 0; j < m_indices.size(); j = j+3)
    {
            //compute triangle normal
            float v[3] = {m_vertices[m_indices[j+1]].pos[0], m_vertices[m_indices[j+1]].pos[1], m_vertices[m_indices[j+1]].pos[2]};
            v[0] -= m_vertices[m_indices[j]].pos[0];
            v[1] -= m_vertices[m_indices[j]].pos[1];
            v[2] -= m_vertices[m_indices[j]].pos[2];

            float w[3] = {m_vertices[m_indices[j+2]].pos[0], m_vertices[m_indices[j+2]].pos[1], m_vertices[m_indices[j+2]].pos[2]};
            w[0] -= m_vertices[m_indices[j]].pos[0];
            w[1] -= m_vertices[m_indices[j]].pos[1];
            w[2] -= m_vertices[m_indices[j]].pos[2];

            m_vertices[m_indices[j]].norm[0] += v[1]*w[2] - v[2]*w[1];
            m_vertices[m_indices[j]].norm[1] += v[2]*w[0] - v[0]*w[2];
            m_vertices[m_indices[j]].norm[2] += v[0]*w[1] - v[1]*w[0];

            m_vertices[m_indices[j+1]].norm[0] += v[1]*w[2] - v[2]*w[1];
            m_vertices[m_indices[j+1]].norm[1] += v[2]*w[0] - v[0]*w[2];
            m_vertices[m_indices[j+1]].norm[2] += v[0]*w[1] - v[1]*w[0];

            m_vertices[m_indices[j+2]].norm[0] += v[1]*w[2] - v[2]*w[1];
            m_vertices[m_indices[j+2]].norm[1] += v[2]*w[0] - v[0]*w[2];
            m_vertices[m_indices[j+2]].norm[2] += v[0]*w[1] - v[1]*w[0];
    }

    for(size_t i = 0; i < m_vertices.size(); ++i)
    {
        float norm[3] = {m_vertices[i].norm[0], m_vertices[i].norm[1], m_vertices[i].norm[2]};

        // normalize
        float length = sqrt(norm[0]*norm[0] + norm[1]*norm[1] + norm[2]*norm[2]);
        norm[0] /= length;
        norm[1] /= length;
        norm[2] /= length;

        m_vertices[i].norm[0] = norm[0];
        m_vertices[i].norm[1] = norm[1];
        m_vertices[i].norm[2] = norm[2];
    }
}

void Surface::computeColors()
{
    for(size_t i = 0; i < m_vertices.size(); ++i)
    {
        double val = m_vertices[i].pos[2];
        float color[3];
        color[0] = clamp(colormapRed(val), 0.0, 1.0);
        color[1] = clamp(colormapGreen(val), 0.0, 1.0);
        color[2] = clamp(colormapBlue(val), 0.0, 1.0);
        m_vertices[i].color[0] = color[0];
        m_vertices[i].color[1] = color[1];
        m_vertices[i].color[2] = color[2];
    }
}

void Surface::normalizeDepth()
{
    for(size_t i = 0; i < m_vertices.size(); ++i)
        m_vertices[i].pos[2] = (m_vertices[i].pos[2] - m_min) / (m_max - m_min);
}

float Surface::colormapRed(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float Surface::colormapGreen(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float Surface::colormapBlue(float x) {
    if (x < 0.3) {
        return 4.0 * x + 0.5;
    } else {
        return -4.0 * x + 2.5;
    }
}

float Surface::clamp(float x, float min, float max)
{
    return std::max(std::min(x, max), min);
}
