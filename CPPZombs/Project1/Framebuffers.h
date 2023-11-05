#include "Text.h"

class Framebuffer
{
public:
	uint rbo, textureColorbuffer, framebuffer, width, height;
    GLint format;
    bool shouldScreenRes;

	Framebuffer() : rbo(0), textureColorbuffer(0), framebuffer(0), width(0), height(0), format(GL_RGB), shouldScreenRes(true) { }

    Framebuffer(uint width, uint height, GLint format = GL_RGB, bool shouldScreenRes = false):
        rbo(0), framebuffer(0), textureColorbuffer(0), width(width), height(height), format(format), shouldScreenRes(shouldScreenRes)
    {
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

	Framebuffer(uint height, GLint format = GL_RGB, bool shouldScreenRes = true) : Framebuffer(static_cast<uint>(ceilf(height * screenRatio)), height, format, shouldScreenRes) { }

    virtual void ResetDim()
    {
        width = trueScreenWidth;
        height = trueScreenHeight;

        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }

    inline float ScreenRatio()
    {
        return float(width) / height;
    }

    inline float TWidth()
    {
        return height * screenRatio;
    }

    inline iVec2 ScrDim()
    {
        return { (int)width, (int)height };
    }

    inline Vec2 TScrDim()
    {
        return { TWidth(), (float)height};
    }

    void Destroy()
    {
        glDeleteRenderbuffers(1, &rbo);
        glDeleteTextures(1, &textureColorbuffer);
        glDeleteFramebuffers(1, &framebuffer);
    }
};

constexpr int DEFFERED_CHANNEL_COUNT = 3;
class DeferredFramebuffer : public Framebuffer
{
public:
    uint normalBuffer, positionBuffer;

    DeferredFramebuffer() : Framebuffer(), normalBuffer(0), positionBuffer(0) { }

    DeferredFramebuffer(uint width, uint height, GLint format = GL_RGB, bool shouldScreenRes = false) :
        DeferredFramebuffer()
    {
        this->width = width;
        this->height = height;
        this->format = format;
        this->shouldScreenRes = shouldScreenRes;

        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);
        // create normal attachment texture
        glGenTextures(1, &normalBuffer);
        glBindTexture(GL_TEXTURE_2D, normalBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalBuffer, 0);
        // create position attachment texture
        glGenTextures(1, &positionBuffer);
        glBindTexture(GL_TEXTURE_2D, positionBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, positionBuffer, 0);
        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
        uint attachments[DEFFERED_CHANNEL_COUNT] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(DEFFERED_CHANNEL_COUNT, attachments);
        // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
        // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    DeferredFramebuffer(uint height, GLint format = GL_RGB, bool shouldScreenRes = true) : DeferredFramebuffer(static_cast<uint>(ceilf(height * screenRatio)), height, format, shouldScreenRes) { }

    void ResetDim() override
    {
        Framebuffer::ResetDim();
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, normalBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, positionBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
};

constexpr int TRUESCREEN = 0;
constexpr int MAINSCREEN = 1;
constexpr int SHADOWMAP = 2;
unique_ptr<DeferredFramebuffer> mainScreen;
unique_ptr<Framebuffer> shadowMap;
vector<Framebuffer*> framebuffers;
uint currentFramebuffer = TRUESCREEN;

void UseFramebuffer()
{
    if (currentFramebuffer == TRUESCREEN)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, trueScreenWidth, trueScreenHeight);
        return;
    }
    Framebuffer* framebuffer = framebuffers[size_t(currentFramebuffer) - 1];
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
    glViewport(0, 0, framebuffer->width, framebuffer->height);
}

int ScrWidth()
{
    if (currentFramebuffer == TRUESCREEN)
        return trueScreenWidth;
    return framebuffers[size_t(currentFramebuffer) - 1]->width;
}

int ScrHeight()
{
    if (currentFramebuffer == TRUESCREEN)
        return trueScreenHeight;
    return framebuffers[size_t(currentFramebuffer) - 1]->height;
}

iVec2 ScrDim()
{
    if (currentFramebuffer == TRUESCREEN)
        return { int(trueScreenWidth), int(trueScreenHeight) };
    return { int(framebuffers[currentFramebuffer - 1]->width), int(framebuffers[currentFramebuffer - 1]->height) };
}