#ifndef GL_GFX_H
#define GL_GFX_H

#include <stdint.h>

#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <SDL_opengl.h>

struct Vertex {
  float x, y;
  float color[4];
};

class GlGFX {
 public:
  GlGFX(int width, int height);
  void deinit();
  void flush();

 public:
  int width, height;
  GLuint program;
  GLint uniform_width;
  GLint uniform_height;
  GLint attribute_coord2d;
  GLint attribute_color;

  mutable std::vector<Vertex> triangles_;
};

void circleColor(const GlGFX& target, int16_t x, int16_t y, int16_t r,
                 uint32_t color);
void filledCircleColor(const GlGFX& target, int16_t x, int16_t y, int16_t r,
                       uint32_t color);
void lineColor(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
               int16_t by, uint32_t color);
void trigonColor(const GlGFX& gfx, int16_t ax, int16_t ay, int16_t bx,
                 int16_t by, int16_t cx, int16_t cy, uint32_t color);
void filledTrigonColor(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
                       int16_t by, int16_t cx, int16_t cy, uint32_t color);
void trigonRGBA(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
                int16_t by, int16_t cx, int16_t cy, uint8_t r, uint8_t g,
                uint8_t b, uint8_t a);
void filledTrigonRGBA(const GlGFX& target, int16_t ax, int16_t ay, int16_t bx,
                      int16_t by, int16_t cx, int16_t cy, uint8_t r, uint8_t g,
                      uint8_t b, uint8_t a);
void GL_GFX_Clear();

#endif  // GL_GFX_H
