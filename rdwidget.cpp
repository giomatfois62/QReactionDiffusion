#include "rdwidget.h"

#include <QPainter>
#include <QTimer>

RDWidget::RDWidget(QWidget *parent) :
    GLWidget(parent),
    isActive(false)
{
    frame = 0;
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
    u = Matrix(size,size);
    u0 = Matrix(size,size);
    v = Matrix(size,size);
    v0 = Matrix(size,size);

    m_solver.setSize(size);
    m_solver.setTimeStep(dt);

    m_pixmap = QPixmap(size, size);

    draw();
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
    if(frame++%10 == 0)
        draw();

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
            double val = (m_solver.u0[i][j] - m_solver.minu) / (m_solver.maxu - m_solver.minu);
            int b = 255 * val;
            int g = 255 * (1 - val);

            QColor color = QColor(0, g , 0);
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

QPixmap RDWidget::pixmap() const
{
    return m_pixmap;
}
