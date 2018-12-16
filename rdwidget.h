#ifndef RDWIDGET_H
#define RDWIDGET_H

#include "glwidget.h"
#include "solver.h"

class RDWidget : public GLWidget
{
public:
    explicit RDWidget(QWidget *parent = 0);
    ~RDWidget();

    Solver *solver();

    QPixmap pixmap() const;

public slots:
    void init(int size, double dt);
    void setSize(int size);
    void setTimeStep(double dt);
    void setFramesToSkip(uint frames);

    void start();
    void stop();
    void save();

private slots:
    void render();
    void draw();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;

private:
    float colormapRed(float x);
    float colormapGreen(float x);
    float colormapBlue(float x);
    float clamp(float x, float min, float max);

    Solver m_solver;
    QPixmap m_pixmap;
    bool isActive;

    uint frame;
    uint m_framesToSkip;

    //Matrix u0, u, v0, v;
};

#endif // RDWIDGET_H
