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
  static std::uniform_real_distribution<float> d(0, 1);

  float bg_offsetx = game.offsetx * 1.1 + (game.ship.x - game.offsetx) / 10.;
  float bg_offsety = game.offsety;
  float mp_offsetx = game.offsetx;
  float mp_offsety = game.offsety;

  if (game.ship.damaged_cooldown) {
    mp_offsety +=
        (d(random_generator_) - 0.5) * game.ship.damaged_cooldown / 1000;
    mp_offsetx +=
        (d(random_generator_) - 0.5) * game.ship.damaged_cooldown / 1000;
  }

  auto prevbg = game.cave.background.begin();
  auto nextbg = std::next(game.cave.background.begin());
  for (; nextbg != game.cave.background.end(); ++nextbg) {
    drawBackgroundLine(*prevbg, *nextbg, bg_offsetx, bg_offsety);
    prevbg = nextbg;
  }

  for (auto& [x, boulder] : game.cave.boulders) {
    if (boulder.dead) {
      continue;
    }
    drawBoulder(boulder, mp_offsetx, mp_offsety);

    if (game.debug) {
      if (game.ship.x - 0.1 < x && x < game.ship.x + 0.1) {
        drawBoulderOutline(boulder, mp_offsetx, mp_offsety, {0, 0, 255});
      }
    }
  }

  if (game.debug) {
    drawEnvelope(game.cave.floor_envelope, mp_offsetx, mp_offsety);
  }

  for (auto& debris : game.cave.debris) {
    if (debris.dead) {
      continue;
    }
    drawDebris(debris, mp_offsetx, mp_offsety);
  }

  if (game.debug) {
    if (!game.collisions.empty()) {
      Pixel bc = toPixel(game.ship.x - mp_offsetx, game.ship.y - mp_offsety);
      al_draw_circle(bc.x, bc.y, game.ship.r * height_, {255, 0, 255, 255}, 2);
    }

    for (auto& boulder : game.collisions) {
      if (boulder.dead) {
        continue;
      }
      drawBoulderOutline(boulder, mp_offsetx, mp_offsety, {255, 255, 255});
      Pixel bc = toPixel(boulder.x - mp_offsetx, boulder.y - mp_offsety);
      al_draw_circle(bc.x, bc.y, boulder.r * height_, {255, 255, 0, 255}, 2);
    }
  }

  for (auto& spider : game.cave.floor_spiders) {
    if (spider.dead) {
      continue;
    }
    drawSpider(spider, mp_offsetx, mp_offsety);
  }
  if (!game.gameover) {
    drawShip(game.ship, mp_offsetx, mp_offsety);
  }

  for (auto& bullet : game.cave.bullets) {
    if (!bullet.dead) {
      drawBullet(bullet, mp_offsetx, mp_offsety);
    }
  }
  for (auto& spit : game.cave.spits) {
    if (!spit.dead) {
      drawSpit(spit, mp_offsetx, mp_offsety);
    }
  }

  drawHealth(game.ship, ship_max_health);
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

void Renderer::drawBackgroundLine(const BackgroundLine& prev,
                                  const BackgroundLine& next, float offsetx,
                                  float offsety) {
  if (next.vertices.size() < 2) {
    return;
  }
  std::vector<float> vertices;
  for (auto vertex : prev.vertices) {
    Pixel p = toPixel(vertex.x - offsetx, vertex.y - offsety);
    vertices.push_back(p.x);
    vertices.push_back(p.y);
  }
  // for (auto vertex : next.vertices | std::views::reverse) {
  for (auto it = next.vertices.rbegin(); it != next.vertices.rend(); ++it) {
    Pixel p = toPixel(it->x - offsetx, it->y - offsety);
    vertices.push_back(p.x);
    vertices.push_back(p.y);
  }
  ALLEGRO_COLOR bg_color =
      al_map_rgb(10 + next.shade / 2, 5 + next.shade / 2, next.shade / 2);
  al_draw_filled_polygon(vertices.data(), vertices.size() / 2, bg_color);
}

void Renderer::drawHealth(const Ship& ship, int max_health) {
  const ALLEGRO_COLOR health_color =
      al_map_rgb(255, ship.damaged_cooldown * 2, ship.damaged_cooldown * 2);
  float frac = static_cast<float>(std::max(0, ship.health)) / max_health;
  float ratio = static_cast<float>(width_) / height_;

  Pixel pa = toPixel(0.5 * ratio - frac / 2., 0);
  Pixel pb = toPixel(0.5 * ratio + frac / 2., 0.01);

  al_draw_filled_rectangle(pa.x, pa.y, pb.x, pb.y, health_color);
}
