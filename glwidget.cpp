#include "glwidget.h"

#include <vector>

using namespace std;


GLWidget::GLWidget(QWidget *parent)
    : QOpenGLWidget(parent),
    ebo(QOpenGLBuffer::IndexBuffer)
{

}

GLWidget::~GLWidget()
{
    makeCurrent();

    vao.destroy();
    vbo.destroy();

    delete texture;

    doneCurrent();
}

void GLWidget::setTexture(const QImage &img)
{
    makeCurrent();

    texture->destroy();
    delete texture;

    texture = new QOpenGLTexture(img);
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glClearColor(1.0f,1.0f,1.0f,10.f);
    glEnable(GL_DEPTH_TEST);

    projection.setToIdentity();

    // create shaders & texture
    program.addShaderFromSourceCode(QOpenGLShader::Vertex,
                                    "#version 330 core\n"
                                    "layout (location = 0) in vec3 pos;\n;"
                                    "layout (location = 1) in vec2 tex;\n;"
                                    "uniform mat4 mvp;\n"
                                    "out vec2 texCoords;\n;"
                                    "void main()\n"
                                    "{\n"
                                    "    gl_Position = mvp * vec4(pos, 1.0);\n"
                                    "    texCoords = tex;\n"
                                    "}"
                                    );

    program.addShaderFromSourceCode(QOpenGLShader::Fragment,
                                    "#version 330 core\n"
                                    "in vec2 texCoords\n;"
                                    "out vec4 color\n;"
                                    "uniform sampler2D screenTexture\n;"
                                    "void main()\n"
                                    "{\n"
                                    "    color = texture(screenTexture, texCoords);\n"
                                    "}"
                                    );

    program.link();
    program.bind();

    texture = new QOpenGLTexture(QImage(100,100,QImage::Format::Format_RGB888));
    texture->setMinificationFilter(QOpenGLTexture::Nearest);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);

    // create vao
    vao.create();
    vao.bind();

    // create vbo and ebo
    vbo.create();
    vbo.bind();
    vbo.setUsagePattern(QOpenGLBuffer::StaticDraw);

    float vertices[] = {
        // positions         // texture coords
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f, // top right
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f  // top left
    };

    vbo.allocate(&vertices[0], sizeof(float) * 20);

    ebo.create();
    ebo.bind();
    ebo.setUsagePattern(QOpenGLBuffer::StaticDraw);

    unsigned int indices[] = {
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };

    ebo.allocate(&indices[0], sizeof(unsigned int) * 6);

    program.enableAttributeArray(0);
    program.enableAttributeArray(1);
    program.setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(float) * 5);
    program.setAttributeBuffer(1, GL_FLOAT, sizeof(float) * 3, 2, sizeof(float) * 5);

    vao.release();
    ebo.release();
    vbo.release();

    program.release();

    view.setToIdentity();
    view.lookAt(QVector3D(0,0,2.4), QVector3D(0,0,0), QVector3D(0,1,0));
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport( 0, 0, w, h );
    projection.ortho(QRect(0,0,w,h));

    // Calculate aspect ratio
    qreal aspect = qreal(w) / qreal(h ? h : 1);

    // Set near plane to 3.0, far plane to 7.0, field of view 45 degrees
    const qreal zNear = 0.01, zFar = 3.0, fov = 45.0;

    // Reset projection
    projection.setToIdentity();

    // Set perspective projection
    projection.perspective(fov, aspect, zNear, zFar);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program.bind();
    {
        program.setUniformValue("mvp", projection*view);

        texture->bind(0);
        program.setUniformValue("screenTexture", 0);

        vao.bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao.release();
    }
    program.release();
}
