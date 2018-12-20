#include "openglwindow.h"

#include <QDebug>
#include <QKeyEvent>
#include <QMouseEvent>
#include <cmath>

OpenGLWindow::OpenGLWindow() :
    elapsed(0),
    ebo(QOpenGLBuffer::IndexBuffer),
    texture(nullptr)
{
    setupWindow();
}

OpenGLWindow::OpenGLWindow(const Surface &surf) :
    elapsed(0),
    m_surface(surf),
    ebo(QOpenGLBuffer::IndexBuffer),
    texture(nullptr)
{
    setupWindow();
}

OpenGLWindow::~OpenGLWindow()
{
    cleanupGL();
}

void OpenGLWindow::setTexture(const QImage &img)
{
    makeCurrent();

    if(texture)
    {
        texture->destroy();
        delete texture;
        texture = nullptr;
    }

    texture = new QOpenGLTexture(img);
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void OpenGLWindow::setSurface(const Surface &surf)
{
    m_surface = surf;
    setupSurface();
}

void OpenGLWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug() << "Pressed key " << event;
    if(event->key() == Qt::Key_E)
        view.rotate(-1.0f, QVector3D(0,0,1));

    if(event->key() == Qt::Key_Q)
        view.rotate(1.0f,QVector3D(0,0,1));
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *event)
{
    qDebug() << "Released key " << event;
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "Pressed mouse button " << event->button();
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << "Released mouse button " << event->button();
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "Moved mouse " << event->pos();

//    QVector4D screenPos(event->pos().x(), event->pos().y(), 0.0f, 1.0f);
//    QVector4D pos = (projection * view).inverted() * screenPos;
//    pos /= pos.w();

    if (event->buttons() & Qt::RightButton)
    {
            int dx = (event->x() - m_lastPos.x());
            int dy = (event->y() - m_lastPos.y());
            view.rotate(dx/10.0f,QVector3D(0,1,0));
            view.rotate(dy/10.0f,QVector3D(1,0,0));

            m_lastPos = event->pos();
    }
}

void OpenGLWindow::wheelEvent(QWheelEvent *event)
{

}

void OpenGLWindow::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    projection.setToIdentity();
    view.setToIdentity();
    view.lookAt(QVector3D(-1.5f, -1.5f, 2.0f), QVector3D(0,0,0), QVector3D(0,0,1));

    setupSurface();

    timer.start();
    elapsed = timer.elapsed();

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

void OpenGLWindow::resizeGL(int w, int h)
{
    qDebug() << "Resizing window " << w << "x" << h;

    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.01f, zFar = 5.0, fov = 60.0;

    // Reset projection
    projection.setToIdentity();

    // Set orthographic projection
    //projection.ortho(QRect(0,0,w/max,h/max));

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void OpenGLWindow::paintGL()
{
    qint64 current = timer.elapsed();
    qint64 lag = current - elapsed;
    elapsed = current;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    {
        QMatrix4x4 mvp = projection * view;
        program.setUniformValue("mvp", mvp);
        program.setUniformValue("n", mvp.normalMatrix());
        program.setUniformValue("screenTexture", 0);
        program.setUniformValue("useTexture", false);

        if(texture)
        {
            program.setUniformValue("useTexture", true);
            texture->bind(0);
        }


        vao.bind();
        glDrawElements(GL_TRIANGLES, m_surface.indices().size(), GL_UNSIGNED_INT, 0);
        vao.release();
    }
    program.release();
}

void OpenGLWindow::cleanupGL()
{
    makeCurrent();

    vao.destroy();
    vbo.destroy();
    ebo.destroy();

    if(texture)
    {
        texture->destroy();
        delete texture;
        texture = nullptr;
    }
}

void OpenGLWindow::setupSurface()
{
    makeCurrent();

    cleanupGL();

    // create shaders & texture
    program.addShaderFromSourceFile(QOpenGLShader::Vertex,":/shaders/shader.vert");
    program.addShaderFromSourceFile(QOpenGLShader::Fragment,":/shaders/shader.frag");

    program.link();
    program.bind();

    // create vao
    vao.create();
    vao.bind();

    // create vbo and ebo
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    vbo.allocate(m_surface.data(), sizeof(Vertex) * m_surface.vertices().size());

    ebo.create();
    ebo.bind();
    ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);
    ebo.allocate(m_surface.indicesPtr(), sizeof(unsigned int) * m_surface.indices().size());

    program.enableAttributeArray(0);
    program.enableAttributeArray(1);
    program.enableAttributeArray(2);
    program.enableAttributeArray(3);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(Vertex)); // pos
    program.setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 3, sizeof(Vertex)); // norm
    program.setAttributeBuffer(2, GL_FLOAT, sizeof(float) * 6, 2, sizeof(Vertex)); // tex
    program.setAttributeBuffer(3, GL_FLOAT, sizeof(float) * 8, 3, sizeof(Vertex)); // col

    vao.release();
    ebo.release();
    vbo.release();

    program.release();
}

void OpenGLWindow::setupWindow()
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3,3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));

    m_lastPos = QPoint(0,0);
}
