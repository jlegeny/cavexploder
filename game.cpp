#include "game.h"

constexpr float vertical_thrust = 0.6;
constexpr float vertical_thrust_max = 3.6;
constexpr float vertical_deceleration = 0.3;
constexpr float vertical_speed = 0.1;

constexpr float horizontal_thrust = 0.6;
constexpr float horizontal_thrust_max = 3.6;
constexpr float horizontal_deceleration = 0.3;
constexpr float horizontal_speed = 0.2;

Game::Game()
    : ship()
    , time_(0) {
  ship.x = 0.1;
  ship.y = 0.5;
  ship.r = 0.0125;
  last_gen = 1.8f;
  cave.generate(0.f, last_gen);
}

void Game::commands(const std::unordered_set<Command>& commands) {
  if (commands.contains(Command::THRUST_UP)) {
    ship.vy -= vertical_thrust;
    ship.vy = std::max(ship.vy, -vertical_thrust_max);
  } else if (commands.contains(Command::THRUST_DOWN)) {
    ship.vy += vertical_thrust;
    ship.vy = std::min(ship.vy, vertical_thrust_max);
  } else {
    if (ship.vy > 0) {
      ship.vy = std::max(ship.vy - vertical_deceleration, 0.f);
    } else if (ship.vy < 0) {
      ship.vy = std::min(ship.vy + vertical_deceleration, 0.f);
    }
  }

  if (commands.contains(Command::THRUST_BACKWARD)) {
    ship.vx -= horizontal_thrust;
    ship.vx = std::max(ship.vx, -horizontal_thrust_max);
  } else if (commands.contains(Command::THRUST_FORWARD)) {
    ship.vx += vertical_thrust;
    ship.vx = std::min(ship.vx, horizontal_thrust_max);
  } else {
    if (ship.vx > 0) {
      ship.vx = std::max(ship.vx - horizontal_deceleration, 0.f);
    } else if (ship.vx < 0) {
      ship.vx = std::min(ship.vx + horizontal_deceleration, 0.f);
    }
  }
};

void Game::checkCollisions() {
  collisions.clear();
  for (const auto& boulders : {cave.ceiling, cave.floor}) {
    for (auto it = boulders.lower_bound(ship.x - 0.1);
         it != boulders.upper_bound(ship.x + 0.1); ++it) {
      if ((ship.x - it->second.x) * (ship.x - it->second.x) +
              (ship.y - it->second.y) * (ship.y - it->second.y) <
          (ship.r + it->second.r) * (ship.r + it->second.r)) {
        collisions.push_back(it->second);
      }
    }
  }
}

void Game::update(uint32_t dt) {
  ship.y += ship.vy * vertical_speed * (dt / 1000.f);
  ship.x += ship.vx * horizontal_speed * (dt / 1000.f);

  ship.x += dt * speed_;
  if (last_gen - offsetx <= 2.0) {
    float next_gen = offsetx + 2.2;
    cave.generate(last_gen, next_gen);
    last_gen = next_gen;
  }
  offsetx += dt * speed_;

  cave.ceiling.erase(cave.ceiling.begin(),
                     cave.ceiling.lower_bound(offsetx - 0.2));
  cave.floor.erase(cave.floor.begin(), cave.floor.lower_bound(offsetx - 0.2));

  checkCollisions();
}
