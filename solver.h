#ifndef SOLVER_H
#define SOLVER_H

#include <map>
#include <vector>
#include <string>

#include "tinyexpr.h"

struct Param
{
    double min;
    double max;
    double value;
};

struct Model
{
    std::map<std::string, Param> params;
    std::string fu;
    std::string fv;
};

struct Matrix {
    int sizeX;
    int sizeY;
    double *data;

    Matrix() : sizeX(-1), sizeY(-1), data(nullptr) {}

    Matrix(int n, int m)
    {
        sizeX = n;
        sizeY = m;
        data = new double[n*m];
    }

    ~Matrix()
    {
        if(data)
            delete[] data;
    }

    double& operator()(const int &i, const int &j)
    {
        return data[i*sizeY+j];
    }
};

class Solver
{
public:
    Solver();
    ~Solver();

    void setModel(Model model);
    void setSize(int val);
    void setTimeStep(double val);

    void init();
    void solve();
    void correct();
    void solve(Matrix &u0, Matrix &u, Matrix &v0, Matrix &v, double h, double dt);

    int size;
    double dt, du, dv, tau, sigma, lambda, k, b, d;

    double maxu, maxv, minu, minv;

    std::vector<std::vector<double>> u0, v0, u, v;

private:
    double fu(double x, double y);
    double fv(double x, double y);
    double laplace(std::vector<std::vector<double>> &w, int i, int j);

    void updateLimits(float x, float y);
    int compileParams();
    void freeExpr();

    Model m_model;

    double _x, _y;
    te_expr *fu_expr;
    te_expr *fv_expr;
};

#endif // SOLVER_H
