#include "solver.h"

#include <cmath>
#include <limits>
#include <iostream>

using namespace std;

Solver::Solver() :
    size(0), dt(1.0f), fu_expr(0), fv_expr(0)
{

}

Solver::~Solver()
{
    freeExpr();
}

void Solver::setModel(Model model)
{
    m_model = model;
    compileParams();
}

void Solver::solve()
{
    if(!fu_expr || !fv_expr)
        return;

    double h = 2.0f / (size -1);
    double invh = 1.0f / (3 * h * h);

    du = m_model.params["du"].value;
    dv = m_model.params["dv"].value;

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

    //correct();
    u0 = u; v0 = v;
}

void Solver::correct()
{
    if(!fu_expr || !fv_expr)
        return;

    vector<vector<double>> u1 = u;
    vector<vector<double>> v1 = v;

    double h = 2.0f / (size -1);
    double invh = 1.0f / (3 * h * h);

    du = m_model.params["du"].value;
    dv = m_model.params["dv"].value;

    for(int i = 1; i < size - 1; i++)
    {
        for(int j = 1; j < size - 1; j++)
        {
            u1[i][j] =  u0[i][j] + dt * (invh * du * 0.5 * (laplace(u0, i, j) + laplace(u, i, j)) + 0.5 * (fu(u0[i][j], v0[i][j]) + fu(u[i][j], v[i][j])));
            v1[i][j] =  v0[i][j] + dt * (invh * dv * 0.5 * (laplace(v0, i, j) + laplace(v, i, j)) + 0.5 * (fv(u0[i][j], v0[i][j]) + fv(u[i][j], v[i][j])));

            updateLimits(u1[i][j], v1[i][j]);
        }
    }

    // set boundary conditions
    for(int i = 0; i < size; i++)
    {
        u1[i][0] = u1[i][1]; u1[i][size - 1] = u1[i][size - 2]; u1[0][i] = u1[1][i]; u1[size - 1][i] = u1[size - 2][i];
        v1[i][0] = v1[i][1]; v1[i][size - 1] = v1[i][size - 2]; v1[0][i] = v1[1][i]; v1[size - 1][i] = v1[size - 2][i];
    }

    u = u1; v = v1;
    u0 = u1; v0 = v1;
}

void Solver::setSize(int val)
{
    if(val <= 1)
        return;

    size = val;
    init();
}

void Solver::setTimeStep(double val)
{
    dt = val;
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
    }

    u = u0;
    v = v0;

    compileParams();
}

double Solver::fu(double x, double y)
{
    _x = x; _y = y;
    return te_eval(fu_expr);

    //return  -x * y * y + b * (1 - x);
}

double Solver::fv(double x, double y)
{
    _x = x; _y = y;
    return te_eval(fv_expr);

    //return x * y * y - d * y;
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

int Solver::compileParams()
{
    te_variable vars[m_model.params.size() + 2];

    int count = 0;
    map<string, Param>::iterator i = m_model.params.begin();
    while (i != m_model.params.end()) {
        vars[count].name = i->first.c_str();
        vars[count].address = &(i->second.value);
        vars[count].type = 0;
        vars[count].context = 0x0;
        count++;
        ++i;
    }
    vars[count].name = "x";
    vars[count].address = &_x;
    vars[count].type = 0;
    vars[count].context = 0x0;
    count++;

    vars[count].name = "y";
    vars[count].address = &_y;
    vars[count].type = 0;
    vars[count].context = 0x0;

    /* Compile the expression with variables. */
    freeExpr();

    int err;
    fu_expr = te_compile(m_model.fu.c_str(), vars, m_model.params.size() + 2, &err);
    fv_expr = te_compile(m_model.fv.c_str(), vars, m_model.params.size() + 2, &err);

    return err;
}

void Solver::freeExpr()
{
    te_free(fu_expr);
    fu_expr = nullptr;
    te_free(fv_expr);
    fv_expr = nullptr;
}
