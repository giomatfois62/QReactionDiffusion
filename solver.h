#ifndef SOLVER_H
#define SOLVER_H

#include <vector>

class Solver
{
public:
    Solver();

    void setSize(int val);
    void init();
    void solve();

    double fu(double x, double y);
    double fv(double x, double y);
    double laplace(std::vector<std::vector<double>> &w, int i, int j);

    int size;
    double dt, du, dv, tau, sigma, lambda, k, b, d;

    double maxu, maxv, minu, minv;

    std::vector<std::vector<double>> u0, v0, u, v;

private:
    void updateLimits(float x, float y);
};

#endif // SOLVER_H
