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
  FIRE_4,
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

  bool gameover = false;
  bool debug = true;

 private:
  uint32_t time_;
  float speed_ = 0.5;  // u/ms
  float last_gen = 0;
  int gameover_countdown = 2000;
  float gameover_slowdown = 1.0;
};

#endif // GAME_H
