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
  for (auto& boulder : game.cave.ceiling) {
    drawBoulder(boulder, game.offsetx, game.offsety);
  }
  for (auto& boulder : game.cave.floor) {
    drawBoulder(boulder, game.offsetx, game.offsety);
  }
  drawShip(game.ship, game.offsetx, game.offsety);
}

void Renderer::drawShip(const Ship& ship, float offsetx, float offsety) {
  Pixel pa = toPixel(ship.x + offsetx - 0.01, ship.y + offsety - 0.01);
  Pixel pb = toPixel(ship.x + offsetx + 0.01, ship.y + offsety);
  Pixel pc = toPixel(ship.x + offsetx - 0.01, ship.y + offsety + 0.01);

  filledTrigonRGBA(renderer_, pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, 200, 50, 15,
                   255);
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
