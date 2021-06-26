#ifndef CAVE_H
#define CAVE_H

#include <cstdint>
#include <deque>
#include <random>
#include <vector>

struct Point {
  float x, y;
};

struct Boulder {
  float x, y;
  float r;
  int shade;
  const std::vector<Point> vertices;
};

struct Ship {
  float x, y;
  float vx, vy;
};

class Cave
{
 public:
  Cave(int seed = 0);

  void generate(float startx, float endx);

  std::deque<Boulder> ceiling;
  std::deque<Boulder> floor;

 private:
  std::vector<Point> generateVertices(float radius);

 private:
  std::default_random_engine generator_;
};

#endif // CAVE_H
