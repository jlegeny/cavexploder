#include "renderer.h"

#include <allegro5/allegro_primitives.h>

#include <cmath>
#include <iostream>

#include "util.h"

Renderer::Renderer(int width, int height)
    : width_(width)
    , height_(height) {}

void Renderer::reset(int width, int height) {
  width_ = width;
  height_ = height;
}

Pixel Renderer::toPixel(float x, float y) const {
  return {.x = static_cast<int16_t>(x * height_),  // unit is height
          .y = static_cast<int16_t>(y * height_)};
}

void Renderer::draw(const Game& game) {
  for (auto& [x, boulder] : game.cave.boulders) {
    if (boulder.dead) {
      continue;
    }
    drawBoulder(boulder, game.offsetx, game.offsety);

    if (game.debug) {
      if (game.ship.x - 0.1 < x && x < game.ship.x + 0.1) {
        drawBoulderOutline(boulder, game.offsetx, game.offsety, {0, 0, 255});
      }
    }
  }

  if (game.debug) {
    drawEnvelope(game.cave.floor_envelope, game.offsetx, game.offsety);
  }

  for (auto& debris : game.cave.debris) {
    if (debris.dead) {
      continue;
    }
    drawDebris(debris, game.offsetx, game.offsety);
  }

  if (game.debug) {
    if (!game.collisions.empty()) {
      Pixel bc =
          toPixel(game.ship.x - game.offsetx, game.ship.y - game.offsety);
      al_draw_circle(bc.x, bc.y, game.ship.r * height_, {255, 0, 255, 255}, 2);
    }

    for (auto& boulder : game.collisions) {
      if (boulder.dead) {
        continue;
      }
      drawBoulderOutline(boulder, game.offsetx, game.offsety, {255, 255, 255});
      Pixel bc = toPixel(boulder.x - game.offsetx, boulder.y - game.offsety);
      al_draw_circle(bc.x, bc.y, boulder.r * height_, {255, 255, 0, 255}, 2);
    }
  }

  for (auto& spider : game.cave.floor_spiders) {
    if (spider.dead) {
      continue;
    }
    drawSpider(spider, game.offsetx, game.offsety);
  }
  if (!game.gameover) {
    drawShip(game.ship, game.offsetx, game.offsety);
  }

  for (auto& bullet : game.cave.bullets) {
    if (!bullet.dead) {
      drawBullet(bullet, game.offsetx, game.offsety);
    }
  }
  for (auto& spit : game.cave.spits) {
    if (!spit.dead) {
      drawSpit(spit, game.offsetx, game.offsety);
    }
  }
}

void Renderer::drawShip(const Ship& ship, float offsetx, float offsety) {
  uint8_t shade = std::min<uint8_t>(179 + ship.damaged_cooldown, 255);
  const ALLEGRO_COLOR hull_color = al_map_rgba(shade, shade, shade, 255);
  const float ship_size = ship.r * 4;
  {
    Pixel pc = toPixel(ship.x - offsetx + 0.053 * ship_size,
                       ship.y - offsety + 0.026 * ship_size);
    al_draw_filled_circle(pc.x, pc.y, 0.233 * ship_size * height_, hull_color);
  }
  {
    Pixel pc = toPixel(ship.x - offsetx + 0.324 * ship_size,
                       ship.y - offsety + 0.129 * ship_size);
    al_draw_filled_circle(pc.x, pc.y, 0.175 * ship_size * height_, hull_color);
  }

  {
    Pixel pc = toPixel(ship.x - offsetx - 0.234 * ship_size,
                       ship.y - offsety - 0.175 * ship_size);
    al_draw_filled_circle(pc.x, pc.y, 0.230 * ship_size * height_, hull_color);
  }
  {
    Pixel pc = toPixel(ship.x - offsetx - 0.118 * ship_size,
                       ship.y - offsety - 0.203 * ship_size);
    al_draw_filled_circle(pc.x, pc.y, 0.230 * ship_size * height_, hull_color);
  }

  {
    Pixel pa = toPixel(ship.x - offsetx - 0.087 * ship_size,
                       ship.y - offsety + 0.020 * ship_size);
    Pixel pb = toPixel(ship.x - offsetx + 0.324 * ship_size,
                       ship.y - offsety + 0.172 * ship_size);
    Pixel pc = toPixel(ship.x - offsetx - 0.419 * ship_size,
                       ship.y - offsety + 0.462 * ship_size);
    al_draw_filled_triangle(pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, hull_color);
    Pixel pd = toPixel(ship.x - offsetx - 0.384 * ship_size,
                       ship.y - offsety + 0.305 * ship_size);
    al_draw_filled_triangle(pa.x, pa.y, pb.x, pb.y, pd.x, pd.y, hull_color);
  }
}

void Renderer::drawBullet(const Bullet& bullet, float offsetx, float offsety) {
  static const ALLEGRO_COLOR bullet_color = al_map_rgba(255, 80, 0, 240);
  Pixel pa = toPixel(bullet.x - offsetx + bullet.nx * 0.003,
                     bullet.y - offsety - bullet.ny * 0.003);
  Pixel pb = toPixel(bullet.x - offsetx + bullet.nx * 0.003,
                     bullet.y - offsety + bullet.ny * 0.003);
  Pixel pc = toPixel(bullet.x - offsetx - bullet.vx * 0.025,
                     bullet.y - offsety - bullet.vy * 0.025);
  al_draw_filled_triangle(pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, bullet_color);
}

void Renderer::drawDebris(const Debris& debris, float offsetx, float offsety) {
  Pixel pa = toPixel(debris.x + debris.vertices[0].x - offsetx,
                     debris.y + debris.vertices[0].y - offsety);
  Pixel pb = toPixel(debris.x + debris.vertices[1].x - offsetx,
                     debris.y + debris.vertices[1].y - offsety);
  Pixel pc = toPixel(debris.x - offsetx, debris.y - offsety);
  al_draw_filled_triangle(
      pa.x, pa.y, pb.x, pb.y, pc.x, pc.y,
      al_map_rgb(15 + debris.shade, 10 + debris.shade, debris.shade));
}

void Renderer::drawBoulder(const Boulder& boulder, float offsetx,
                           float offsety) {
  ALLEGRO_COLOR boulder_color =
      al_map_rgb(15 + boulder.shade + boulder.damaged_cooldown,
                 10 + boulder.shade, boulder.shade);
  for (size_t i = 0; i < boulder.vertices.size(); ++i) {
    size_t a = i % boulder.vertices.size();
    size_t b = (i + 1) % boulder.vertices.size();
    Pixel pa = toPixel(boulder.x + boulder.vertices[a].x - offsetx,
                       boulder.y + boulder.vertices[a].y - offsety);
    Pixel pb = toPixel(boulder.x + boulder.vertices[b].x - offsetx,
                       boulder.y + boulder.vertices[b].y - offsety);
    Pixel pc = toPixel(boulder.x - offsetx, boulder.y - offsety);

    al_draw_filled_triangle(pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, boulder_color);
  }
}

void Renderer::drawBoulderOutline(const Boulder& boulder, float offsetx,
                                  float offsety, std::array<uint8_t, 3> color) {
  ALLEGRO_COLOR boulder_color = al_map_rgb(color[0], color[1], color[2]);
  for (size_t i = 0; i < boulder.vertices.size(); ++i) {
    size_t a = i % boulder.vertices.size();
    size_t b = (i + 1) % boulder.vertices.size();
    Pixel pa = toPixel(boulder.x + boulder.vertices[a].x - offsetx,
                       boulder.y + boulder.vertices[a].y - offsety);
    Pixel pb = toPixel(boulder.x + boulder.vertices[b].x - offsetx,
                       boulder.y + boulder.vertices[b].y - offsety);
    Pixel pc = toPixel(boulder.x - offsetx, boulder.y - offsety);

    al_draw_triangle(pa.x, pa.y, pb.x, pb.y, pc.x, pc.y, boulder_color, 2);
  }
}

void Renderer::drawSpider(const Spider& spider, float offsetx, float offsety) {
  const ALLEGRO_COLOR spider_color = al_map_rgb(179, 179, 179);

  Pixel pc = toPixel(spider.x - offsetx, spider.y - offsety);
  al_draw_filled_circle(pc.x, pc.y, spider.r * height_, spider_color);
}

void Renderer::drawSpit(const Spit& spit, float offsetx, float offsety) {
  const ALLEGRO_COLOR spit_color = al_map_rgb(255, 0, 0);

  Pixel pc = toPixel(spit.x - offsetx, spit.y - offsety);
  al_draw_filled_circle(pc.x, pc.y, spit.r * height_, spit_color);
}

void Renderer::drawEnvelope(const std::map<float, float>& envelope,
                            float offsetx, float offsety) {
  if (envelope.size() < 2) {
    return;
  }
  float lx = envelope.begin()->first;
  float ly = envelope.begin()->second;
  auto it = std::next(envelope.begin());
  for (; it != envelope.end(); ++it) {
    float x = it->first;
    float y = it->second;
    Pixel pa = toPixel(lx - offsetx, ly - offsety);
    Pixel pb = toPixel(x - offsetx, y - offsety);
    al_draw_line(pa.x, pa.y, pb.x, pb.y, {255, 0, 0, 255}, 2);
    lx = x;
    ly = y;
  }
}
