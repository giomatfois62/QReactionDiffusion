#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QElapsedTimer>

#include "surface.h"

class OpenGLWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit OpenGLWindow();
    explicit OpenGLWindow(const Surface &surf);
    ~OpenGLWindow();

    void setTexture(const QImage &img);
    void setSurface(const Surface &surf);

protected:
    void keyPressEvent(QKeyEvent * event) override;
    void keyReleaseEvent(QKeyEvent * event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void cleanupGL();
    void setupSurface();
    void setupWindow();

private:
    QElapsedTimer timer;
    qint64 elapsed;

    Surface m_surface;

    QOpenGLShaderProgram program;
    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo, ebo;
    QOpenGLTexture *texture;
    QMatrix4x4 projection, view;

    QPoint m_lastPos;
};

#endif // OPENGLWINDOW_H
