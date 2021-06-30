#ifndef RENDERER_H
#define RENDERER_H

#include <array>
#include <cstdint>

#include "cave.h"
#include "game.h"

struct Pixel {
  int16_t x, y;
};

class Renderer
{
 public:
  Renderer(int width, int height);
  void reset(int width, int height);

  void draw(const Game& game);

  Pixel toPixel(float x, float y) const;

 private:
  void drawShip(const Ship& ship, float offsetx, float offsety);
  void drawBullet(const Bullet& bullet, float offsetx, float offsety);
  void drawDebris(const Debris& debris, float offsetx, float offsety);
  void drawBoulder(const Boulder& boulder, float offsetx, float offsety);
  void drawBoulderOutline(const Boulder& boulder, float offsetx, float offsety,
                          std::array<uint8_t, 3> color);
  void drawSpider(const Spider& spider, float offsetx, float offsety);
  void drawSpit(const Spit& spit, float offsetx, float offsety);
  void drawEnvelope(const std::map<float, float>& envelope, float offsetx,
                    float offsety);

 private:
  int width_;
  int height_;
};

#endif // RENDERER_H
