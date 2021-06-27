#include "cave.h"

constexpr int density = 80;

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

void Cave::generate(float startx, float endx) {
  static std::uniform_real_distribution<float> d(0, 1);
  static std::uniform_int_distribution<int> d_shade(0, 47);
  static std::uniform_real_distribution<float> d_radius(0.02, 0.1);

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

  for (int i = 0; i < static_cast<int>(density * (endx - startx)); ++i) {
    float x = startx + d(generator_) * (endx - startx);
    float y = d(generator_) * -fabs(cos(x) + sin(3 * x)) * 0.3 + 1.05;

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
}
