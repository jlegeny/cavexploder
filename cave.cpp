#include "cave.h"

#include "util.h"

constexpr int density = 80;
constexpr float envelope_presicion = 1.f / 128.f;
constexpr float spider_probability = 0.1;
constexpr float formation_probablity = 0.005;

Cave::Cave(int seed)
    : generator_(seed) {}

std::vector<Point> Cave::generateVertices(float radius) {
  static std::uniform_real_distribution<float> d(0, 1);
  static std::uniform_int_distribution<int> d_vertex_count(5, 10);

  int vertice_count = d_vertex_count(generator_);
  std::vector<Point> vertices;
  for (int j = 0; j < vertice_count; ++j) {
    float ur = (d(generator_) * 0.2 + 0.8) * radius;
    float skew = (d(generator_) * 0.2 - 0.1) * 2 * M_PI;
    float vx = ur * sin(2 * M_PI * j / vertice_count + skew);
    float vy = ur * cos(2 * M_PI * j / vertice_count + skew);
    vertices.push_back({vx, vy});
  }

  return vertices;
}

void Cave::explodeBoulder(const Boulder &boulder) {
  static std::uniform_real_distribution<float> d_angle(-M_PI / 2, M_PI);
  static std::uniform_real_distribution<float> d_ejection_angle(
      M_PI / 4 + M_PI / 2, M_PI * 2 / 3 + M_PI / 2);

  for (size_t i = 0; i < boulder.vertices.size(); ++i) {
    size_t a = i % boulder.vertices.size();
    size_t b = (i + 1) % boulder.vertices.size();
    float theta = d_angle(generator_);
    std::array<Point, 2> vertices = {
        {{boulder.vertices[a].x, boulder.vertices[a].y},
         {boulder.vertices[b].x, boulder.vertices[b].y}}};
    debris.push_back({.x = boulder.x,
                      .y = boulder.y,
                      .am = 0.f,
                      .vx = sinf(theta),
                      .vy = cosf(theta),
                      .shade = boulder.shade,
                      .vertices = vertices});
  }

  for (auto &spider : floor_spiders) {
    float theta = d_ejection_angle(generator_);
    if (sqdist(spider.x, spider.y, boulder.x, boulder.y) <
        boulder.r * boulder.r * 1.4) {
      spider.walking = false;
      spider.vx = sin(theta);
      spider.vy = cos(theta);
    }
  }
}

void Cave::spiderSpit(const Spider &spider, const Ship &ship) {
  static std::uniform_real_distribution<float> d_r(0.004, 0.007);
  float ship_speed = 0.5;

  float vx =
      (ship.x + ship_speed / spider.spit_speed - spider.x) * spider.spit_speed;
  float vy = (ship.y - spider.y) * spider.spit_speed;

  spits.push_back({
      .x = spider.x,
      .y = spider.y,
      .vx = vx,
      .vy = vy,
      .r = d_r(generator_),
  });
}

float envelopeRound(float f) {
  int n = f / envelope_presicion;
  return n * envelope_presicion;
}

void Cave::generate(float startx, float endx) {
  static std::uniform_real_distribution<float> d(0, 1);
  static std::uniform_int_distribution<int> d_shade(0, 47);
  static std::uniform_real_distribution<float> d_radius(0.02, 0.1);
  static std::uniform_real_distribution<float> d_spider_r(0.008, 0.012);
  static std::uniform_real_distribution<float> d_spider_speed(0.75, 1.5);
  static std::uniform_int_distribution<int> d_spider_burst_rate(1, 5);
  static std::uniform_real_distribution<float> d_spider_fire_rate(0.5, 1.5);
  static std::uniform_real_distribution<float> d_spider_burst_fire_rate(0.1,
                                                                        0.2);
  static std::uniform_real_distribution<float> d_spider_spit_speed(1., 2.);

  for (int i = 0; i < static_cast<int>(density * (endx - startx)); ++i) {
    float x = startx + d(generator_) * (endx - startx);
    float y = d(generator_) * fabs(sin(x)) * 0.3 - 0.05;

    float radius = d_radius(generator_);
    int shade = d_shade(generator_);

    Boulder p = {.x = x,
                 .y = y,
                 .r = radius,
                 .shade = shade,
                 .health = static_cast<int>(radius * 1000),
                 .vertices = generateVertices(radius)};
    boulders.emplace(x, p);
  }

  // floor
  for (int i = 0; i < static_cast<int>(density * (endx - startx)); ++i) {
    float x = startx + d(generator_) * (endx - startx);
    float y = d(generator_) * -fabs(cos(x) + sin(3 * x)) * 0.3 + 1.05;
    if (endx < 2) {
      y = std::max(y, 0.85f);
    }

    float radius = d_radius(generator_);
    int shade = d_shade(generator_);

    float lx = -radius;
    while (lx < radius) {
      float ex = envelopeRound(x + lx);
      float coslx = (lx / radius);
      float ley = y - sqrt(1 - coslx * coslx) * radius;
      lx += envelope_presicion;
      if (!floor_envelope.count(ex) || floor_envelope[ex] > ley) {
        floor_envelope[ex] = ley;
      }
      if (endx > 2.4) {
        if (d(generator_) < spider_probability * envelope_presicion *
                                (100.f + startx) / 100.f) {
          float spider_r = d_spider_r(generator_);
          float spider_speed = d_spider_speed(generator_);
          floor_spiders.push_back({
              .x = ex,
              .y = floor_envelope[ex],
              .walking = true,
              .from = ex,
              .to = ex - envelope_presicion,
              .t = 0,
              .r = spider_r,
              .speed = spider_speed,
              .health = 10,
              .forward = true,
              .burst_rate = d_spider_burst_rate(generator_),
              .burst = 0,
              .cooldown = 0.f,
              .fire_rate = d_spider_fire_rate(generator_),
              .burst_fire_rate = d_spider_burst_fire_rate(generator_),
              .spit_speed = d_spider_spit_speed(generator_),
          });
        }
      }
    }

    Boulder p = {.x = x,
                 .y = y,
                 .r = radius,
                 .shade = shade,
                 .health = static_cast<int>(radius * 3000),
                 .vertices = generateVertices(radius)};
    boulders.emplace(x, p);
  }

  // formations
  float p_formation = d(generator_);
  if (p_formation * (endx - startx) <
      formation_probablity + (startx / 1000.f)) {
    float length = endx - startx;
    for (int i = 0; i < static_cast<int>(density * length * 0.5); ++i) {
      float x = startx + 0.25 * length + d(generator_) * 0.5 * length;
      float y = d(generator_) * fabs(sin(x)) * 0.95 - 0.05;

      float radius = d_radius(generator_) * (1 + ((0.5 - y) * (0.5 - y)));
      int shade = d_shade(generator_);

      Boulder p = {.x = x,
                   .y = y,
                   .r = radius,
                   .shade = shade,
                   .health = static_cast<int>(radius * 1000),
                   .vertices = generateVertices(radius)};
      boulders.emplace(x, p);
    }
  }
}
