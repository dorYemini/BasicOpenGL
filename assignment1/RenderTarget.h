#pragma once

#include <glad/glad.h>
#include <Debugger.h>

#include <iostream>

/* An offscreen framebuffer with a single color texture: draw into it, then sample it in the next pass. */
class RenderTarget
{
    private:
        unsigned int m_FBO = 0;
        unsigned int m_Texture = 0;
        int m_Width, m_Height;

    public:
        RenderTarget(int width, int height)
            : m_Width(width), m_Height(height)
        {
            /* Float texture so intermediate values (blur, gradients) keep full precision and aren't clamped to [0,1] */
            GLCall(glGenTextures(1, &m_Texture));
            GLCall(glBindTexture(GL_TEXTURE_2D, m_Texture));
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr));
            /* NEAREST: each texel is read exactly. CLAMP_TO_EDGE: neighbours past the border repeat the edge pixel */
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
            GLCall(glBindTexture(GL_TEXTURE_2D, 0));

            GLCall(glGenFramebuffers(1, &m_FBO));
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
            GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_Texture, 0));
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
                std::cerr << "RenderTarget: framebuffer incomplete" << std::endl;
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
        }

        ~RenderTarget()
        {
            glDeleteFramebuffers(1, &m_FBO);
            glDeleteTextures(1, &m_Texture);
        }

        /* Owns GL objects: no copies */
        RenderTarget(const RenderTarget&) = delete;
        RenderTarget& operator=(const RenderTarget&) = delete;

        /* Make this the draw destination, with a viewport covering the whole texture */
        void BindAsTarget() const
        {
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_FBO));
            GLCall(glViewport(0, 0, m_Width, m_Height));
        }

        /* Bind the texture for sampling as the next pass's input */
        void BindTexture(unsigned int slot = 0) const
        {
            GLCall(glActiveTexture(GL_TEXTURE0 + slot));
            GLCall(glBindTexture(GL_TEXTURE_2D, m_Texture));
        }
};
