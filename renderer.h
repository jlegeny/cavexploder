#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>

#include <cstdint>

#include "cave.h"
#include "game.h"
#ifndef SDL_GFX
#include "gl_gfx.h"
#endif

struct Pixel {
  int16_t x, y;
};

class Renderer
{
 public:
#if SDL_GFX
  Renderer(SDL_Renderer* renderer, int width, int height, float unit);
#else
  Renderer(const GlGFX& gfx, int width, int height, float unit);
#endif

  void draw(const Game& game);

  Pixel toPixel(float x, float y) const;

 private:
  void drawShip(const Ship& ship, float offsetx, float offsety);
  void drawBullet(const Bullet& bullet, float offsetx, float offsety);
  void drawDebris(const Debris& debris, float offsetx, float offsety);
  void drawBoulder(const Boulder& boulder, float offsetx, float offsety);
  void drawBoulderOutline(const Boulder& boulder, float offsetx, float offsety,
                          uint32_t color);
  void drawSpider(const Spider& spider, float offsetx, float offsety);
  void drawSpit(const Spit& spit, float offsetx, float offsety);
  void drawEnvelope(const std::map<float, float>& envelope, float offsetx,
                    float offsety);

 private:
#if SDL_GLX
  SDL_Renderer* renderer_;
#else
  const GlGFX& renderer_;
#endif
  const int width_;
  const int height_;
  const int unit_;
};

#endif // RENDERER_H
