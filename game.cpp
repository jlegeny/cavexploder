#include "game.h"

#include "util.h"

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
  ship.health = 1000;
  last_gen = 1.8f;
  cave.generate(0.f, last_gen);
}

void Game::commands(const std::unordered_set<Command>& commands) {
  if (commands.count(Command::THRUST_UP)) {
    ship.vy -= vertical_thrust;
    ship.vy = std::max(ship.vy, -vertical_thrust_max);
  } else if (commands.count(Command::THRUST_DOWN)) {
    ship.vy += vertical_thrust;
    ship.vy = std::min(ship.vy, vertical_thrust_max);
  } else {
    if (ship.vy > 0) {
      ship.vy = std::max(ship.vy - vertical_deceleration, 0.f);
    } else if (ship.vy < 0) {
      ship.vy = std::min(ship.vy + vertical_deceleration, 0.f);
    }
  }

  if (commands.count(Command::THRUST_BACKWARD)) {
    ship.vx -= horizontal_thrust;
    ship.vx = std::max(ship.vx, -horizontal_thrust_max);
  } else if (commands.count(Command::THRUST_FORWARD)) {
    ship.vx += vertical_thrust;
    ship.vx = std::min(ship.vx, horizontal_thrust_max);
  } else {
    if (ship.vx > 0) {
      ship.vx = std::max(ship.vx - horizontal_deceleration, 0.f);
    } else if (ship.vx < 0) {
      ship.vx = std::min(ship.vx + horizontal_deceleration, 0.f);
    }
  }

  if (ship.cannon_cooldown == 0 && commands.count(Command::FIRE)) {
    if (fired_forward) {
      cave.bullets.push_back({.x = ship.x,
                              .y = ship.y,
                              .vx = sqrtf(3.f) / 2.f,
                              .vy = 0.5f,
                              .nx = 0.5f,
                              .ny = -sqrtf(3.f) / 2.f,
                              .damage = bullet_damage,
                              .dead = false});
    } else {
      cave.bullets.push_back({.x = ship.x,
                              .y = ship.y,
                              .vx = 1.f,
                              .vy = 0.f,
                              .nx = 0.f,
                              .ny = 1.f,
                              .damage = bullet_damage,
                              .dead = false});
    }
    fired_forward = !fired_forward;
    ship.cannon_cooldown = cannon_speed;
  }
};

void Game::checkCollisions() {
  collisions.clear();
  for (auto it = cave.boulders.lower_bound(ship.x - 0.1);
       it != cave.boulders.upper_bound(ship.x + 0.1); ++it) {
    if (it->second.dead) {
      continue;
    }
    if ((ship.x - it->second.x) * (ship.x - it->second.x) +
            (ship.y - it->second.y) * (ship.y - it->second.y) <
        (ship.r + it->second.r) * (ship.r + it->second.r)) {
      collisions.push_back(it->second);
      it->second.dead = true;
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
        boulder.health -= bullet.damage * multiplier;
        score += 50 * boulder.r * multiplier;
      }
    }
  }
}

void Game::update(uint32_t dt) {
  static std::uniform_real_distribution<float> d_angle(0, M_PI * 2);
  if (!started) {
    return;
  }
  const float dts = dt / 1000.f;
  const float offset = dts * speed_ * gameover_slowdown * multiplier;

  ship.y += ship.vy * vertical_speed * dts;
  ship.x += ship.vx * horizontal_speed * dts;

  ship.x += offset;

  if (ship.x < offsetx) {
    ship.x = offsetx;
  }
  if (ship.x > offsetx + 1.77) {
    ship.x = offsetx + 1.77;
  }
  ship.y = std::min(std::max(ship.y, 0.f), 1.f);

  if (last_gen - offsetx <= 2.0) {
    float next_gen = offsetx + 2.2;
    cave.generate(last_gen, next_gen);
    last_gen = next_gen;
  }

  if (ship.cannon_cooldown > 0) {
    ship.cannon_cooldown = std::max<int32_t>(ship.cannon_cooldown - dt, 0);
  }

  if (!gameover) {
    offsetx += offset;
  }

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
    if (sqdist(spit.x, spit.y, ship.x, ship.y) < ship.r * ship.r) {
      spit.dead = true;
      ship.health -= spit.r * 2000;
      ship.damaged_cooldown = 50;

      float theta = d_angle(generator_);
      std::array<Point, 2> vertices = {
          {{cosf(theta - 0.1f) * 0.02f, sinf(theta - 0.1f) * 0.02f},
           {cosf(theta + 0.1f) * 0.02f, sinf(theta + 0.1f) * 0.02f}}};
      Debris d = {
          .x = ship.x,
          .y = ship.y,
          .am = 0.f,
          .vx = spit.vx,
          .vy = spit.vy,
          .shade = 100,
          .vertices = vertices,
      };
      cave.debris.push_back(d);
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

  for (auto& boulder : collisions) {
    ship.health -= 1000 * boulder.r;
    ship.damaged_cooldown = 100;
    cave.explodeBoulder(boulder);
  }

  if (ship.damaged_cooldown > 0) {
    ship.damaged_cooldown = std::max<int32_t>(ship.damaged_cooldown - dt, 0);
    if (ship.health <= 0) {
      if (!gameover) {
        for (int j = 0; j < 50; ++j) {
          float theta = d_angle(generator_);
          float size = fabs(d_angle(generator_)) * 0.01;
          std::array<Point, 2> vertices = {
              {{cosf(theta - 0.1f) * size, sinf(theta - 0.1f) * size},
               {cosf(theta + 0.1f) * size, sinf(theta + 0.1f) * size}}};
          Debris d = {
              .x = ship.x,
              .y = ship.y,
              .am = 0.f,
              .vx = static_cast<float>(sinf(theta) * 0.2f),
              .vy = static_cast<float>(cosf(theta) * 0.2f),
              .shade = static_cast<int>(10000.f * size),
              .vertices = vertices,
          };
          cave.debris.push_back(d);
        }
      }
      gameover = true;
    }
    multiplier = std::max(multiplier - dts * 3, 1.f);
  } else {
    multiplier += 0.03 * dts;
  }

  if (gameover) {
    if (gameover_slowdown > 0) {
      gameover_slowdown = std::max(gameover_slowdown - dts, 0.f);
    }
    if (gameover_countdown > 0) {
      gameover_countdown = std::max<int>(gameover_countdown - dt, 0);
    } else if (gameover_countdown == 0) {
      for (auto& [x, boulder] : cave.boulders) {
        boulder.dead = true;
        cave.explodeBoulder(boulder);
      }
      cave.boulders.clear();
      gameover_countdown = -1;
    }
  } else {
    score += multiplier * multiplier * dt;
  }
}
