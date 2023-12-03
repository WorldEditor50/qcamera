#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QObject>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

class Render : protected QOpenGLFunctions
{
public:
    enum Attribute {
        ATTRIBUTE_VERTEX = 0,
        ATTRIBUTE_TEXTURE
    };
public:
    Render(){}
    virtual ~Render(){}
    virtual void initialize() = 0;
    virtual void paint(unsigned char* data, int w, int h, int type) = 0;
    virtual void clear() = 0;
};

class RgbaRender : public Render
{
private:
    /* shader program */
    QOpenGLShaderProgram *program;
    /* texture */
    QOpenGLTexture *texture;
    /* VBO */
    QOpenGLBuffer vbo;
    /* shader */
    GLuint textureUniform;
    /* shader id */
    GLuint textureID;
    /* rotate */
    QMatrix2x2 projectMat;
public:
    RgbaRender():program(nullptr),texture(nullptr){}
    void initialize() override
    {
        /* init opengl */
        initializeOpenGLFunctions();
        /* create VBO */
        vbo.create();
        static const GLfloat vertex[] = {
             /* vertex */
            -1.0f,-1.0f,
            1.0f,-1.0f,
            -1.0f, 1.0f,
            1.0f,1.0f,
            /* texture */
            0.0f, 1.0f,
            1.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 0.0f
        };
        vbo.bind();
        vbo.allocate(vertex, sizeof(vertex));
        /* vertex shader */
        QString vertexSourceCode = "attribute vec4 vertexPosition;"
                                   "attribute vec2 textureCoordinate;"
                                   "varying vec2 texture_Out;"
                                   "void main(void) {"
                                   "    gl_Position = vertexPosition;"
                                   "    texture_Out = textureCoordinate;"
                                   "}";
        program = new QOpenGLShaderProgram(QOpenGLContext::currentContext());
        program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSourceCode);
        program->bindAttributeLocation("vertexPosition", ATTRIBUTE_VERTEX);
        /* fragment shader */
        QString fragmentSourceCode = "varying vec2 texture_Out;"
                                     "uniform sampler2D tex;"
                                     "void main(void) {"
                                     "  gl_FragColor = texture2D(tex, texture_Out);"
                                     "}";
        program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSourceCode);
        program->bindAttributeLocation("textureCoordinate", ATTRIBUTE_TEXTURE);
        /* compile */
        program->link();
        program->bind();
        /* enable */
        program->enableAttributeArray(ATTRIBUTE_VERTEX);
        program->enableAttributeArray(ATTRIBUTE_TEXTURE);
        /* set buffer */
        program->setAttributeBuffer(ATTRIBUTE_VERTEX, GL_FLOAT, 0, 2, 2*sizeof(GLfloat));
        program->setAttributeBuffer(ATTRIBUTE_TEXTURE, GL_FLOAT, 8*sizeof(GLfloat), 2, 2*sizeof(GLfloat));
        /* texture */
        textureUniform = program->uniformLocation("tex");
        texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        texture->create();
        textureID = texture->textureId();
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        return;
    }
    void resize(int w, int h, int angle)
    {

    }
    void paint(unsigned char* data, int w, int h, int type) override
    {
        /* clear */
        //glClearColor(0, 0, 0, 0);
        //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        /* active and bind */
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        /* modify texture */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(textureUniform, 0);
        /* draw */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        return;
    }
    void clear() override
    {
        if (program != nullptr) {
            delete program;
            program = nullptr;
        }
        if (texture != nullptr) {
            delete texture;
            texture = nullptr;
        }
        return;
    }
};

class OpenGLWidget : public QOpenGLWidget
{
    Q_OBJECT
public:
    class Frame
    {
    public:
        unsigned char *data;
        int width;
        int height;
        std::size_t totalSize;
    public:
        Frame():data(nullptr),width(0),height(0),totalSize(0){}
        ~Frame()
        {
            if (data != nullptr) {
                delete [] data;
                data = nullptr;
            }
        }
        void create(int h, int w, unsigned char*rgb)
        {
            width = w;
            height = h;
            std::size_t size = h*w*3;
            if (totalSize < size) {
                totalSize = size;
                if (data != nullptr) {
                    delete [] data;
                }
                data = new unsigned char[size];
            }
            memcpy(data, rgb, totalSize);
            return;
        }
    };
private:
    Frame frame;
    RgbaRender render;
public:
    OpenGLWidget(QWidget *parent):QOpenGLWidget(parent){}
    inline void setFrame(int h, int w, unsigned char *data)
    {
        frame.create(h, w, data);
        update();
        return;
    }
protected:
    void initializeGL() override
    {
        connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLWidget::clear);
        render.initialize();
        return;
    }
    void resizeGL(int w, int h) override
    {
        glViewport(0, 0, w, h);
        return;
    }
    void paintGL() override
    {
        if (frame.data == nullptr) {
            return;
        }
        render.paint(frame.data, frame.width, frame.height, 0);
        return;
    }
    void clear()
    {
        makeCurrent();
        render.clear();
        doneCurrent();
        return;
    }
};

#endif // OPENGLWIDGET_H
