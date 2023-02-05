#include "Text.h"

class Framebuffer
{
public:
	uint fbo, rbo, textureColorbuffer, framebuffer, texturePos, width, height;

	Framebuffer() : fbo(0), rbo(0), textureColorbuffer(0), framebuffer(0), texturePos(0), width(0), height(0) { }

    Framebuffer(uint width, uint height) : fbo(0), rbo(0), framebuffer(0), textureColorbuffer(0), texturePos(0), width(width), height(height)
    {
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        // create a color attachment texture
        glGenTextures(1, &textureColorbuffer);
        glActiveTexture(GL_TEXTURE0 + totalTexturesCreated);
        texturePos = totalTexturesCreated;
        totalTexturesCreated++;
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
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

	Framebuffer(uint height) : Framebuffer(ceilf(height * screenRatio), height) { }

    void ResetWidth()
    {
        int newWidth = int(ceilf(height * screenRatio));
        if (width == newWidth)
            return;
        width = newWidth;
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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

    inline Vec2 ScrDim()
    {
        return { (int)width, (int)height };
    }

    inline Vec2f TScrDim()
    {
        return { TWidth(), (float)height};
    }
};

uint currentFramebuffer = 0;
Framebuffer midRes, subScat, shadowMap; // Shadowmap must be initialized seperately due to how it works.
vector<Framebuffer*> framebuffers{ &midRes, &subScat, &shadowMap };

void UseFramebuffer()
{
    if (currentFramebuffer == 0)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, trueScreenWidth, trueScreenHeight);
        return;
    }
    Framebuffer* framebuffer = framebuffers[currentFramebuffer - 1];
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->framebuffer);
    glViewport(0, 0, framebuffer->width, framebuffer->height);
}

int ScrWidth()
{
    if (currentFramebuffer == 0)
        return trueScreenWidth;
    return framebuffers[currentFramebuffer - 1]->width;
}

int ScrHeight()
{
    if (currentFramebuffer == 0)
        return trueScreenHeight;
    return framebuffers[currentFramebuffer - 1]->height;
}

Vec2 ScrDim()
{
    if (currentFramebuffer == 0)
        return { int(trueScreenWidth), int(trueScreenHeight) };
    return { int(framebuffers[currentFramebuffer - 1]->width), int(framebuffers[currentFramebuffer - 1]->height) };
}