#include "solver.h"

#include <cmath>
#include <limits>

using namespace std;

Solver::Solver()
{
    size = 0; dt = 1.0f; du = 0.00002f; dv = 0.00001f; b = 0.04f; d = 0.1f;
}


void Solver::solve()
{
    double h = 2.0f / (size -1);
    double invh = 1.0f / (3 * h * h);

    for(int i = 1; i < size - 1; i++)
    {
        for(int j = 1; j < size - 1; j++)
        {
            u[i][j] =  u0[i][j] + dt * (invh * du * laplace(u0, i, j) + fu(u0[i][j], v0[i][j]));
            v[i][j] =  v0[i][j] + dt * (invh * dv * laplace(v0, i, j) + fv(u0[i][j], v0[i][j]));

            updateLimits(u[i][j], v[i][j]);
        }
    }

    // set boundary conditions
    for(int i = 0; i < size; i++)
    {
        u[i][0] = u[i][1]; u[i][size - 1] = u[i][size - 2]; u[0][i] = u[1][i]; u[size - 1][i] = u[size - 2][i];
        v[i][0] = v[i][1]; v[i][size - 1] = v[i][size - 2]; v[0][i] = v[1][i]; v[size - 1][i] = v[size - 2][i];
    }

    u0 = u; v0 = v;
}

void Solver::setSize(int val)
{
    if(val <= 1)
        return;

    size = val;
    init();
}

void Solver::init()
{
    maxu = maxv = numeric_limits<double>::min();
    minu = minv = numeric_limits<double>::max();

    u0.clear(); v0.clear();
    u0.resize(size); v0.resize(size);

    // init random
    for(int i = 0; i < size; i++)
    {
        u0[i].resize(size); v0[i].resize(size);

        for(int j = 0; j < size; j++)
        {
               double x = -1 + i * 2.0f / (size-1);
               double y = -1 + j * 2.0f / (size-1);

               u0[i][j] = 1 - exp(-80 * ((x+0.05) * (x+0.05) + (y+0.02) * (y+0.02)));
               v0[i][j] = exp(-80 * ((x-0.05) * (x-0.05) + (y-0.02) * (y-0.02)));

               updateLimits(u0[i][j], v0[i][j]);
        }
    }

    // boundary conditions (Dirichlet)
    for(int i = 0; i < size; i++)
    {
        u0[i][0] = u0[i][1]; u0[i][size - 1] = u0[i][size - 2]; u0[0][i] = u0[1][i]; u0[i][size - 1] = u0[i][size - 2];
        v0[i][0] = v0[i][1]; v0[i][size - 1] = v0[i][size - 2]; v0[0][i] = v0[1][i]; v0[i][size - 1] = v0[i][size - 2];
        //u0[0][i] = u0[size - 1][i] = u0[i][0] = u0[i][size - 1] = 0;
        //v0[0][i] = v0[size - 1][i] = v0[i][0] = v0[i][size - 1] = 0;
    }

    u = u0;
    v = v0;
}

double Solver::fu(double x, double y)
{
    return  -x * y * y + b * (1 - x);
}

double Solver::fv(double x, double y)
{
    return x * y * y - d * y;
}

double Solver::laplace(std::vector<std::vector<double> > &w, int i, int j)
{
    return w[i-1][j] + w[i+1][j] + w[i][j-1] + w[i][j+1] +
           w[i-1][j-1] + w[i+1][j+1] + w[i-1][j+1] + w[i+1][j-1] -
           8 * w[i][j];
}

void Solver::updateLimits(float x, float y)
{
    if(x >= maxu)
        maxu = x;
    if(y >= maxv)
        maxv = y;
    if(x <= minu)
        minu = x;
    if(y <= minv)
        minv = y;
}
