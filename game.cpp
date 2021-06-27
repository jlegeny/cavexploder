#include "game.h"

constexpr float vertical_thrust = 0.6;
constexpr float vertical_thrust_max = 3.6;
constexpr float vertical_deceleration = 0.3;
constexpr float vertical_speed = 0.1;

constexpr float horizontal_thrust = 0.6;
constexpr float horizontal_thrust_max = 3.6;
constexpr float horizontal_deceleration = 0.3;
constexpr float horizontal_speed = 0.2;

constexpr float bullet_speed = 0.0223;
constexpr float cannon_speed = 100;

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

  if (ship.cannon_cooldown == 0 && commands.contains(Command::FIRE)) {
    if (commands.contains(Command::FIRE_4)) {
      cave.bullets.push_back({.x = ship.x,
                              .y = ship.y,
                              .vx = 1.f,
                              .vy = 0.f,
                              .nx = 0.f,
                              .ny = 1.f,
                              .damage = 4,
                              .dead = false});
    }
    ship.cannon_cooldown = cannon_speed;
  }
};

void Game::checkCollisions() {
  collisions.clear();
  for (auto it = cave.boulders.lower_bound(ship.x - 0.1);
       it != cave.boulders.upper_bound(ship.x + 0.1); ++it) {
    if ((ship.x - it->second.x) * (ship.x - it->second.x) +
            (ship.y - it->second.y) * (ship.y - it->second.y) <
        (ship.r + it->second.r) * (ship.r + it->second.r)) {
      collisions.push_back(it->second);
    }
  }

  // Bullet collisions
  for (auto& bullet : cave.bullets) {
    if (bullet.dead) {
      continue;
    }
    for (auto it = cave.boulders.lower_bound(bullet.x - 0.1);
         it != cave.boulders.upper_bound(bullet.x + 0.1); ++it) {
      auto& boulder = it->second;
      if (boulder.dead) {
        continue;
      }
      if ((bullet.x - boulder.x) * (bullet.x - boulder.x) +
              (bullet.y - boulder.y) * (bullet.y - boulder.y) <
          (boulder.r) * (boulder.r)) {
        bullet.dead = true;
        boulder.damaged_cooldown = 50;
        boulder.health -= bullet.damage;
      }
    }
  }
}

void Game::update(uint32_t dt) {
  const float offset = dt * speed_;

  ship.y += ship.vy * vertical_speed * (dt / 1000.f);
  ship.x += ship.vx * horizontal_speed * (dt / 1000.f);

  ship.x += offset;
  if (last_gen - offsetx <= 2.0) {
    float next_gen = offsetx + 2.2;
    cave.generate(last_gen, next_gen);
    last_gen = next_gen;
  }

  if (ship.cannon_cooldown > 0) {
    ship.cannon_cooldown = std::max<int32_t>(ship.cannon_cooldown - dt, 0);
  }

  offsetx += offset;

  for (auto& bullet : cave.bullets) {
    bullet.x += bullet.vx * bullet_speed + offset;
    bullet.y += bullet.vy * bullet_speed;
    if (bullet.x > offsetx + 1.8) {
      bullet.dead = true;
    }
  }

  cave.boulders.erase(cave.boulders.begin(),
                      cave.boulders.lower_bound(offsetx - 0.2));

  for (auto& [x, boulder] : cave.boulders) {
    if (boulder.damaged_cooldown > 0) {
      boulder.damaged_cooldown =
          std::max<int32_t>(boulder.damaged_cooldown - dt, 0);
      if (boulder.health <= 0) {
        boulder.dead = true;
      }
    }
  }

  while (!cave.bullets.empty() && cave.bullets.front().dead) {
    cave.bullets.pop_front();
  }

  checkCollisions();
}
