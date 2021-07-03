#ifndef GAME_H
#define GAME_H

#include <unordered_set>
#include <vector>

#include "cave.h"

enum class Command {
  THRUST_UP,
  THRUST_DOWN,
  THRUST_FORWARD,
  THRUST_BACKWARD,
  FIRE,
};

class Game
{
 public:
  Game();

  void update(uint32_t dt);
  void commands(const std::unordered_set<Command>& commands);
  void checkCollisions();

 public:
  Cave cave;
  float offsetx = 0;
  float offsety = 0;

  Ship ship;
  std::vector<Boulder> collisions;

  bool started = false;
  bool gameover = false;
  bool debug = false;
  int64_t score = 0;

 private:
  uint32_t time_;
  float last_gen = 0;
  int gameover_countdown = 2000;
  float gameover_slowdown = 1.0;
  float bullet_angle = 0.0;
  float bullet_angle_delta = +M_PI / 16;

 private:
  std::default_random_engine generator_;
};

#endif // GAME_H
