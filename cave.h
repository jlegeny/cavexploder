#ifndef CAVE_H
#define CAVE_H

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <random>
#include <vector>

constexpr int ship_max_health = 1000;

enum class Biome {
  CAVERN,
};

struct Point {
  float x, y;
};

struct Boulder {
  Biome biome;
  float x, y;
  float r;
  int shade;
  int health;
  bool destructible;
  bool dead;
  uint32_t damaged_cooldown;
  const std::vector<Point> vertices;
};

struct Ship {
  float x, y;
  float vx, vy;
  float r;
  float multiplier;
  float speed;
  int32_t cannon_cooldown;
  int health;
  uint32_t damaged_cooldown;
};

struct Bullet {
  float x, y;
  float vx, vy;
  float nx, ny;
  int damage;
  bool dead;
};

struct Debris {
  float x, y;
  float am;
  float vx, vy;
  int shade;
  bool dead;
  const std::array<Point, 2> vertices;
};

struct Spider {
  float x, y;
  bool walking;
  float vx, vy;
  float from, to;
  float t;
  float r;
  float speed;
  float health;
  bool forward;
  bool dead;
  bool smart;
  int burst_rate;
  int burst;
  float cooldown;
  float fire_rate;
  float burst_fire_rate;
  float spit_speed;
};

struct Spit {
  float x, y;
  float vx, vy;
  float r;
  bool dead;
};

struct BackgroundLine {
  Biome biome;
  int shade;
  std::vector<Point> vertices;
};

class Cave
{
 public:
  Cave(int seed = 0);

  void generate(float startx, float endx);
  void explodeBoulder(const Boulder& boulder);
  void spiderSpit(const Spider& spider, const Ship& ship);

 public:
  std::map<float, Boulder> boulders;
  std::map<float, float> floor_envelope;
  std::deque<Spider> floor_spiders;
  std::deque<Bullet> bullets;
  std::deque<Spit> spits;
  std::deque<Debris> debris;
  std::deque<BackgroundLine> background;

 private:
  std::vector<Point> generateBoulderVertices(float radius);
  std::vector<Point> generateBackgroundLineVertices(float x);

  int background_line_shade = 10;
  int background_line_shade_direction = 1;

  std::default_random_engine cave_generator_;
  std::default_random_engine random_generator_;
};

#endif // CAVE_H
