#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Debugger.h>
#include <VertexBuffer.h>
#include <VertexBufferLayout.h>
#include <IndexBuffer.h>
#include <VertexArray.h>
#include <Shader.h>
#include <Texture.h>
#include "RenderTarget.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>

using namespace std;
using namespace glm;

static int soloImage = 0;

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS || key < GLFW_KEY_1 || key > GLFW_KEY_4)
        return;
    int image = key - GLFW_KEY_1 + 1;
    soloImage = (soloImage == image) ? 0 : image;
}

int main(int argc, char* argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Set OpenGL to Version 3.3.0 */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(512, 512, "Assignment 1", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    /* Load GLAD so it configures OpenGL */
    gladLoadGL();

    /* Control frame rate */
    glfwSwapInterval(1);

    /* Print OpenGL version after completing initialization */
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    /* Set scope so that on widow close the destructors will be called automatically */
    {
        // Full-viewport quad. No texture coordinates: the vertex shader derives them from position.
        float positions[] = {
            -1.0f, -1.0f, // 0
             1.0f, -1.0f, // 1
             1.0f,  1.0f, // 2
            -1.0f,  1.0f  // 3
        };

        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        /* Generate VAO, VBO, EBO and bind them */
        VertexArray va;
        VertexBuffer vb(positions, sizeof(positions));
        IndexBuffer ib(indices, sizeof(indices));

        VertexBufferLayout layout;
        layout.Push<float>(2); // position -> location 0
        va.AddBuffer(vb, layout);

        /* Resolve resources relative to the executable so it works from any working directory */
        std::filesystem::path exeDir = std::filesystem::path(argv[0]).parent_path();

        /* Load the texture (stb_image, flipped so it appears upright) */
        std::string texturePath = (exeDir / "res/textures/Lenna.png").string();
        Texture texture(texturePath);
        if (texture.GetWidth() == 0)
            std::cerr << "Failed to load texture: " << texturePath << std::endl;

        /* Every shader samples its input from texture unit 0 */
        auto loadShader = [&](const char* relPath)
        {
            auto shader = std::make_unique<Shader>((exeDir / relPath).string());
            shader->Bind();
            shader->SetUniform1i("u_Texture", 0);
            return shader;
        };

        auto basicShader     = loadShader("res/shaders/basic.shader");
        auto grayscaleShader = loadShader("res/shaders/grayscale.shader");
        auto halftoneShader  = loadShader("res/shaders/halftone.shader");

        /* Canny edge detection, one shader per step. Each pass reads the previous pass's output. */
        std::vector<std::unique_ptr<Shader>> cannyPasses;
        cannyPasses.push_back(loadShader("res/shaders/canny/1_grayscale.shader"));
        cannyPasses.push_back(loadShader("res/shaders/canny/2_blur_horizontal.shader"));
        cannyPasses.push_back(loadShader("res/shaders/canny/3_blur_vertical.shader"));
        cannyPasses.push_back(loadShader("res/shaders/canny/4_gradient.shader"));      // R = magnitude, G = angle
        cannyPasses.push_back(loadShader("res/shaders/canny/5_nms_threshold.shader"));
        cannyPasses.push_back(loadShader("res/shaders/canny/6_hysteresis.shader"));

        /* Two image-sized offscreen textures that the passes alternate between (ping-pong) */
        RenderTarget pingPong[2] = {
            { texture.GetWidth(), texture.GetHeight() },
            { texture.GetWidth(), texture.GetHeight() },
        };

        /* Halftone output: each input pixel becomes a 2x2 block, so the target is twice the size on each axis */
        RenderTarget halftoneTarget(2 * texture.GetWidth(), 2 * texture.GetHeight());

        /* Unbind all to prevent accidentally modifying them */
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        basicShader->Unbind();

        auto drawQuad = [&]()
        {
            GLCall(glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr));
        };

        const int width = texture.GetWidth(), height = texture.GetHeight();
        unsigned int cpuTexture = 0;
        {
            RenderTarget grayscaleTarget(width, height);
            grayscaleTarget.BindAsTarget();
            va.Bind();
            ib.Bind();
            texture.Bind(0);
            grayscaleShader->Bind();
            drawQuad();

            unsigned char *pixels = (unsigned char*)malloc(width * height);
            GLCall(glPixelStorei(GL_PACK_ALIGNMENT, 1));
            GLCall(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
            GLCall(glReadPixels(0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, pixels));
            GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));


            std::vector<float> work(pixels, pixels + width * height);


            for (int y = height - 1; y >= 0; y--)
            {
                for (int x = 0; x < width; x++)
                {
                    float oldValue = work[y * width + x];
                    int q = std::clamp((int)std::lround(oldValue / 17.0f), 0, 15);
                    pixels[y * width + x] = (unsigned char)(q * 17);
                    float error = oldValue - q * 17;

                    bool hasRight = x + 1 < width;
                    bool hasLeft  = x > 0;
                    bool hasDown  = y > 0;

                    /* Neighbours: right 7/16, down-left 3/16, down 5/16, down-right 1/16 */
                    struct Neighbour { int dx, dy; float weight; bool exists; };
                    Neighbour neighbours[4] = {
                        {  1,  0, 7.0f / 16.0f, hasRight            },
                        { -1, -1, 3.0f / 16.0f, hasLeft && hasDown  },
                        {  0, -1, 5.0f / 16.0f, hasDown             },
                        {  1, -1, 1.0f / 16.0f, hasRight && hasDown },
                    };

                    int available = 0;
                    for (const Neighbour& n : neighbours)
                        available += n.exists;

                    /* All 4 neighbours exist: use the 7/3/5/1 weights.
                       At an image edge: split the error equally between the neighbours that exist. */
                    for (const Neighbour& n : neighbours)
                    {
                        if (!n.exists)
                            continue;
                        float share = (available == 4) ? n.weight : 1.0f / available;
                        work[(y + n.dy) * width + (x + n.dx)] += error * share;
                    }
                }
            }

            GLCall(glGenTextures(1, &cpuTexture));
            GLCall(glBindTexture(GL_TEXTURE_2D, cpuTexture));
            GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED));
            GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED));
            free(pixels);
        }

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            /* Framebuffer size can differ from window size (DPI scaling, resizing) */
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            int halfW = width / 2, halfH = height / 2;

            /* Viewport origin is bottom-left, so the top row sits at y = halfH */
            auto setCellViewport = [&](int col, int row)
            {
                if (soloImage == 0)
                {
                    GLCall(glViewport(col * halfW, (1 - row) * halfH, halfW, halfH));
                }
                else if (soloImage == row * 2 + col + 1)
                {
                    GLCall(glViewport(0, 0, width, height));
                }
                else
                {
                    return false;
                }
                return true;
            };

            va.Bind();
            ib.Bind();

            /* --- Halftone: source image -> 2x-size offscreen texture (one fragment per output pixel) --- */
            halftoneTarget.BindAsTarget(); // also sets the viewport to the 2x size
            texture.Bind(0);
            halftoneShader->Bind();
            drawQuad();

            /* --- Canny: all passes but the last render offscreen, alternating between pingPong[0] and [1] --- */
            int dst = 0;
            for (size_t p = 0; p < cannyPasses.size(); p++)
            {
                bool lastPass = (p + 1 == cannyPasses.size());
                bool draw = true;

                /* Input: the source image for the first pass, otherwise whatever the previous pass wrote */
                if (p == 0)
                    texture.Bind(0);
                else
                    pingPong[1 - dst].BindTexture(0);

                /* Output: the other ping-pong texture, or the screen for the last pass */
                if (lastPass)
                {
                    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
                    GLCall(glViewport(0, 0, width, height));
                    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
                    draw = setCellViewport(1, 0);
                }
                else
                {
                    pingPong[dst].BindAsTarget();
                }

                cannyPasses[p]->Bind();
                if (draw)
                    drawQuad();

                dst = 1 - dst;
            }

            /* --- Other cells: one pass each, straight from the source image to the screen --- */
            texture.Bind(0);

            grayscaleShader->Bind();
            if (setCellViewport(0, 0))
                drawQuad();

            /* Halftone cell: show the 2x texture rendered above, unchanged */
            halftoneTarget.BindTexture(0);
            basicShader->Bind();
            if (setCellViewport(0, 1))
                drawQuad();

            GLCall(glActiveTexture(GL_TEXTURE0));
            GLCall(glBindTexture(GL_TEXTURE_2D, cpuTexture));
            basicShader->Bind();
            if (setCellViewport(1, 1))
                drawQuad();

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

        GLCall(glDeleteTextures(1, &cpuTexture));
    }

    glfwTerminate();
    return 0;
}
