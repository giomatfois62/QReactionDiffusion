#include "rdwidget.h"

#include <QPainter>
#include <QTimer>

RDWidget::RDWidget(QWidget *parent) :
    GLWidget(parent),
    isActive(false),
    frame(0),
    m_framesToSkip(10)
{

}

RDWidget::~RDWidget()
{

}

Solver *RDWidget::solver()
{
    return &m_solver;
}

void RDWidget::init(int size, double dt)
{
//    u = Matrix(size,size);
//    u0 = Matrix(size,size);
//    v = Matrix(size,size);
//    v0 = Matrix(size,size);

    setSize(size);
    setTimeStep(dt);
    draw();
}

void RDWidget::setSize(int size)
{
    m_solver.setSize(size);
    m_pixmap = QPixmap(size, size);
}

void RDWidget::setTimeStep(double dt)
{
    m_solver.setTimeStep(dt);
}

void RDWidget::setFramesToSkip(uint frames)
{
    m_framesToSkip = frames;
}

void RDWidget::start()
{
    if(isActive)
        return;

    isActive = true;
    QTimer::singleShot(1, this, &RDWidget::render);
}

void RDWidget::stop()
{
    isActive = false;
}

void RDWidget::render()
{
    if(!isActive)
        return;

    m_solver.solve();
    if((++frame)%m_framesToSkip == 0)
    {
        frame = 0;
        draw();
    }

    QTimer::singleShot(1, this, &RDWidget::render);
}

void RDWidget::draw()
{
    int size = m_solver.size;

    QPainter painter(&m_pixmap);
    for(int i = 0; i  < size; i++)
    {
        for(int j = 0; j < size; j++)
        {
            double val = 1.0f - (m_solver.u0[i][j] - m_solver.minu) / (m_solver.maxu - m_solver.minu);

            int r = 255 * clamp(colormapRed(val), 0.0, 1.0);
            int g = 255 * clamp(colormapGreen(val), 0.0, 1.0);
            int b = 255 * clamp(colormapBlue(val), 0.0, 1.0);

            QColor color = QColor(r, g , b);
            painter.setPen(color);
            painter.drawPoint(i, j);
        }
    }

    setTexture(m_pixmap.toImage());
    update();
}

void RDWidget::mousePressEvent(QMouseEvent *e)
{
    // TODO: start drawing initial condition
}

void RDWidget::mouseReleaseEvent(QMouseEvent *e)
{
    // TODO: stop drawing initial condition
}

void RDWidget::mouseMoveEvent(QMouseEvent *e)
{
    // TODO: draw initial condition
}

float RDWidget::colormapRed(float x) {
    if (x < 0.7) {
        return 4.0 * x - 1.5;
    } else {
        return -4.0 * x + 4.5;
    }
}

float RDWidget::colormapGreen(float x) {
    if (x < 0.5) {
        return 4.0 * x - 0.5;
    } else {
        return -4.0 * x + 3.5;
    }
}

float RDWidget::colormapBlue(float x) {
    if (x < 0.3) {
        return 4.0 * x + 0.5;
    } else {
        return -4.0 * x + 2.5;
    }
}

float RDWidget::clamp(float x, float min, float max)
{
    return qMax(qMin(x, max), min);
}

QPixmap RDWidget::pixmap() const
{
    return m_pixmap;
}
