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
constexpr int bullet_damage = 100;

constexpr float gravity = 2.91;

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
                              .damage = bullet_damage,
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
  const float dts = dt / 1000.f;
  const float offset = dts * speed_;

  ship.y += ship.vy * vertical_speed * dts;
  ship.x += ship.vx * horizontal_speed * dts;

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

  for (auto& spit : cave.spits) {
    spit.x += spit.vx * dts;
    spit.y += spit.vy * dts;
    if (offsetx - 0.1 > spit.x || spit.x > offsetx + 1.8 || spit.y < 0) {
      spit.dead = true;
    }
  }

  cave.boulders.erase(cave.boulders.begin(),
                      cave.boulders.lower_bound(offsetx - 0.2));
  cave.floor_envelope.erase(cave.floor_envelope.begin(),
                            cave.floor_envelope.lower_bound(offsetx - 0.2));

  for (auto& [x, boulder] : cave.boulders) {
    if (boulder.damaged_cooldown > 0) {
      boulder.damaged_cooldown =
          std::max<int32_t>(boulder.damaged_cooldown - dt, 0);
      if (boulder.health <= 0) {
        boulder.dead = true;
        cave.explodeBoulder(boulder);
      }
    }
  }

  for (auto& spider : cave.floor_spiders) {
    if (spider.dead) {
      continue;
    }
    if (spider.walking) {
      spider.t += spider.speed * dts;
      if (spider.t >= 1) {
        auto it = cave.floor_envelope.find(spider.to);
        if (spider.forward) {
          it = std::prev(it);
        } else {
          it = std::next(it);
        }
        spider.from = spider.to;
        spider.to = it->first;
        spider.t = 0;
      }

      auto pfrom = cave.floor_envelope.find(spider.from);
      auto pto = cave.floor_envelope.find(spider.to);

      if (pfrom != cave.floor_envelope.end() &&
          pto != cave.floor_envelope.end()) {
        spider.x = (spider.to - spider.from) * spider.t + spider.from;
        spider.y = (pto->second - pfrom->second) * spider.t + pfrom->second;
      }

    } else {
      spider.x += spider.vx * dts;
      spider.y += spider.vy * dts;
      spider.vy += gravity * dts;
    }

    if (spider.cooldown == 0) {
      cave.spiderSpit(spider, ship);
      spider.burst += 1;
      if (spider.burst >= spider.burst_rate) {
        spider.cooldown = spider.fire_rate;
        spider.burst = 0;
      } else {
        spider.cooldown = spider.burst_fire_rate;
      }
    } else {
      if (spider.cooldown > 0) {
        spider.cooldown = std::max(spider.cooldown - dts, 0.f);
      }
    }

    if (spider.x < offsetx - 0.1 || spider.y > 1) {
      spider.dead = true;
    }
  }

  for (auto& debris : cave.debris) {
    if (debris.dead) {
      continue;
    }
    debris.x += (debris.vx) * dts;
    debris.y += (debris.vy) * dts;
    debris.vy += gravity * dts;

    if (debris.x < offsetx - 0.1 || debris.y > 1.1 || debris.y < -0.1) {
      debris.dead = true;
    }
  }

  while (!cave.floor_spiders.empty() && cave.floor_spiders.front().dead) {
    cave.floor_spiders.pop_front();
  }
  while (!cave.bullets.empty() && cave.bullets.front().dead) {
    cave.bullets.pop_front();
  }
  while (!cave.spits.empty() && cave.spits.front().dead) {
    cave.spits.pop_front();
  }
  while (!cave.debris.empty() && cave.debris.front().dead) {
    cave.debris.pop_front();
  }

  checkCollisions();
}
