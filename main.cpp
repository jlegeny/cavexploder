#include <SDL.h>
#include <SDL_FontCache.h>

#include <iostream>
#include <unordered_set>

#include "game.h"
#include "renderer.h"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr float UNIT = 10;

int main() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) !=
      0) {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");

  SDL_Window* window = SDL_CreateWindow(
      "Cave Horizotal Scrolling Shooter", SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
      /*SDL_WINDOW_RESIZABLE |*/ SDL_WINDOW_ALLOW_HIGHDPI);
  if (window == NULL) {
    printf("Error creating window: %s\n", SDL_GetError());
    return -2;
  }

  SDL_Renderer* sdl_renderer = SDL_CreateRenderer(
      window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (sdl_renderer == NULL) {
    printf("Error creating renderer: %s\n", SDL_GetError());
    return -3;
  }
  SDL_RenderSetScale(sdl_renderer, 2, 2);

  std::unique_ptr<FC_Font, void (*)(FC_Font*)> font(FC_CreateFont(),
                                                    FC_FreeFont);
  FC_LoadFont(font.get(), sdl_renderer, "IBMPlexMono-Medium.ttf", 10,
              FC_MakeColor(0, 255, 0, 255), TTF_STYLE_NORMAL);

  Game game;
  Renderer renderer(sdl_renderer, WINDOW_WIDTH, WINDOW_HEIGHT, UNIT);

  uint32_t last_ticks = SDL_GetTicks();
  std::unordered_set<Command> commands;

  int frame = 0;

  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        done = true;
      }
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(window)) {
        done = true;
      }
    }

    // Clear screen
    SDL_SetRenderDrawColor(sdl_renderer, 0x03, 0x03, 0x03, 0xFF);
    SDL_RenderClear(sdl_renderer);

    commands.clear();
    const Uint8* kbd_state = SDL_GetKeyboardState(NULL);
    if (kbd_state[SDL_SCANCODE_UP] ^ kbd_state[SDL_SCANCODE_DOWN]) {
      commands.insert(kbd_state[SDL_SCANCODE_UP] ? Command::THRUST_UP
                                                 : Command::THRUST_DOWN);
    }
    if (kbd_state[SDL_SCANCODE_RIGHT] ^ kbd_state[SDL_SCANCODE_LEFT]) {
      commands.insert(kbd_state[SDL_SCANCODE_RIGHT] ? Command::THRUST_FORWARD
                                                    : Command::THRUST_BACKWARD);
    }
    if (kbd_state[SDL_SCANCODE_D]) {
      commands.insert({Command::FIRE, Command::FIRE_4});
    }
    game.commands(commands);

    uint32_t ticks = SDL_GetTicks();
    uint32_t dt = ticks - last_ticks;
    game.update(dt);
    last_ticks = ticks;

    renderer.draw(game);

    int stri = 0;
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "FPS: %.1f",
            1000.f / dt);

    std::string commands_str = "";
    commands_str += commands.contains(Command::THRUST_BACKWARD) ? "<" : " ";
    commands_str += commands.contains(Command::THRUST_UP) ? "^" : " ";
    commands_str += commands.contains(Command::THRUST_DOWN) ? "v" : " ";
    commands_str += commands.contains(Command::THRUST_FORWARD) ? ">" : " ";
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Commands: %s",
            commands_str.c_str());

    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Vertical Thrust: %f",
            game.ship.vy);

    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Boulders: %zu",
            game.cave.boulders.size());
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Bullets: %zu",
            game.cave.bullets.size());
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Spits: %zu",
            game.cave.spits.size());
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Debris: %zu",
            game.cave.debris.size());
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Floor spiders: %zu",
            game.cave.floor_spiders.size());

    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Envelope points: %zu",
            game.cave.floor_envelope.size());
    FC_Draw(font.get(), sdl_renderer, 20, ++stri * 12, "Collisions: %zu",
            game.collisions.size());
    SDL_RenderPresent(sdl_renderer);
    ++frame;
  }

  SDL_DestroyRenderer(sdl_renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
