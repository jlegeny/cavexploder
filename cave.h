#ifndef CAVE_H
#define CAVE_H

#include <cstdint>
#include <deque>
#include <map>
#include <random>
#include <vector>

struct Point {
  float x, y;
};

struct Boulder {
  float x, y;
  float r;
  int shade;
  int health;
  bool dead;
  uint32_t damaged_cooldown;
  const std::vector<Point> vertices;
};

struct Ship {
  float x, y;
  float vx, vy;
  float r;
  int32_t cannon_cooldown;
};

struct Bullet {
  float x, y;
  float vx, vy;
  float nx, ny;
  int damage;
  bool dead;
};

class Cave
{
 public:
  Cave(int seed = 0);

  void generate(float startx, float endx);

  std::map<float, Boulder> boulders;
  std::deque<Bullet> bullets;

 private:
  std::vector<Point> generateVertices(float radius);

 private:
  std::default_random_engine generator_;
};

#endif // CAVE_H
