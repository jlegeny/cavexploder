#ifndef GAME_H
#define GAME_H

#include <unordered_set>

#include "cave.h"

enum class Command {
  THRUST_UP,
  THRUST_DOWN,
  THRUST_FORWARD,
  THRUST_BACKWARD,
};

class Game
{
 public:
  Game();

  void update(uint32_t dt);
  void commands(const std::unordered_set<Command>& commands);

 public:
  Cave cave;
  float offsetx = 0;
  float offsety = 0;

  Ship ship;

 private:
  uint32_t time;
  float speed = 0.0005;  // u/ms

  float last_gen = 0;
};

#endif // GAME_H
