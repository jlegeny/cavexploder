#include "renderer.h"

#include <SDL2_gfxPrimitives.h>

#include <iostream>

Renderer::Renderer(SDL_Renderer* renderer, int width, int height, float unit)
    : renderer_(renderer)
    , width_(width)
    , height_(height)
    , unit_(unit) {}

Pixel Renderer::toPixel(float x, float y) const {
  return {.x = static_cast<int16_t>(x * height_),  // unit is height
          .y = static_cast<int16_t>(y * height_)};
}

void Renderer::draw(const Game& game) {
  for (auto& [x, boulder] : game.cave.ceiling) {
    drawBoulder(boulder, game.offsetx, game.offsety);

    if (game.ship.x - 0.1 < x && x < game.ship.x + 0.1) {
      drawBoulderOutline(boulder, game.offsetx, game.offsety, 0xffff0000);
    }
  }
  for (auto& [x, boulder] : game.cave.floor) {
    drawBoulder(boulder, game.offsetx, game.offsety);

    if (game.ship.x - 0.1 < x && x < game.ship.x + 0.1) {
      drawBoulderOutline(boulder, game.offsetx, game.offsety, 0xffff0000);
    }
  }

  if (!game.collisions.empty()) {
    Pixel bc = toPixel(game.ship.x - game.offsetx, game.ship.y - game.offsety);
    circleColor(renderer_, bc.x, bc.y, game.ship.r * height_, 0xff00ffff);
  }

  for (auto& boulder : game.collisions) {
    drawBoulderOutline(boulder, game.offsetx, game.offsety, 0xffffffff);
    Pixel bc = toPixel(boulder.x - game.offsetx, boulder.y - game.offsety);
    circleColor(renderer_, bc.x, bc.y, boulder.r * height_, 0xff00ffff);
  }
  drawShip(game.ship, game.offsetx, game.offsety);
}

void Renderer::drawShip(const Ship& ship, float offsetx, float offsety) {
  constexpr uint32_t hullcolor = 0xffb3b3b3;
  const float ship_size = ship.r * 4;
  {
    Pixel pc = toPixel(ship.x - offsetx + 0.053 * ship_size,
                       ship.y - offsety + 0.026 * ship_size);
    filledCircleColor(renderer_, pc.x, pc.y, 0.233 * ship_size * height_,
                      hullcolor);
  }
  {
    Pixel pc = toPixel(ship.x - offsetx + 0.324 * ship_size,
                       ship.y - offsety + 0.129 * ship_size);
    filledCircleColor(renderer_, pc.x, pc.y, 0.175 * ship_size * height_,
                      hullcolor);
  }

  {
    Pixel pc = toPixel(ship.x - offsetx - 0.234 * ship_size,
                       ship.y - offsety - 0.175 * ship_size);
    filledCircleColor(renderer_, pc.x, pc.y, 0.230 * ship_size * height_,
                      hullcolor);
  }
  {
    Pixel pc = toPixel(ship.x - offsetx - 0.118 * ship_size,
                       ship.y - offsety - 0.203 * ship_size);
    filledCircleColor(renderer_, pc.x, pc.y, 0.230 * ship_size * height_,
                      hullcolor);
  }

  {
    Pixel pa = toPixel(ship.x - offsetx - 0.087 * ship_size,
                       ship.y - offsety + 0.020 * ship_size);
    Pixel pb = toPixel(ship.x - offsetx + 0.324 * ship_size,
                       ship.y - offsety + 0.172 * ship_size);
    Pixel pc = toPixel(ship.x - offsetx - 0.419 * ship_size,
                       ship.y - offsety + 0.462 * ship_size);
    filledTrigonColor(renderer_, pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, hullcolor);
    Pixel pd = toPixel(ship.x - offsetx - 0.384 * ship_size,
                       ship.y - offsety + 0.305 * ship_size);
    filledTrigonColor(renderer_, pa.x, pa.y, pb.x, pb.y, pd.x, pd.y, hullcolor);
  }
}

void Renderer::drawBoulder(const Boulder& boulder, float offsetx,
                           float offsety) {
  for (size_t i = 0; i < boulder.vertices.size(); ++i) {
    size_t a = i % boulder.vertices.size();
    size_t b = (i + 1) % boulder.vertices.size();
    Pixel pa = toPixel(boulder.x + boulder.vertices[a].x - offsetx,
                       boulder.y + boulder.vertices[a].y - offsety);
    Pixel pb = toPixel(boulder.x + boulder.vertices[b].x - offsetx,
                       boulder.y + boulder.vertices[b].y - offsety);
    Pixel pc = toPixel(boulder.x - offsetx, boulder.y - offsety);

    filledTrigonRGBA(renderer_, pa.x, pa.y, pb.x, pb.y, pc.x, pc.y,
                     15 + boulder.shade, 10 + boulder.shade, boulder.shade,
                     255);
  }
}

void Renderer::drawBoulderOutline(const Boulder& boulder, float offsetx,
                                  float offsety, uint32_t color) {
  for (size_t i = 0; i < boulder.vertices.size(); ++i) {
    size_t a = i % boulder.vertices.size();
    size_t b = (i + 1) % boulder.vertices.size();
    Pixel pa = toPixel(boulder.x + boulder.vertices[a].x - offsetx,
                       boulder.y + boulder.vertices[a].y - offsety);
    Pixel pb = toPixel(boulder.x + boulder.vertices[b].x - offsetx,
                       boulder.y + boulder.vertices[b].y - offsety);
    Pixel pc = toPixel(boulder.x - offsetx, boulder.y - offsety);

    trigonColor(renderer_, pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, color);
  }
}
