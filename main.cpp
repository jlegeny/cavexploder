#define ALLEGRO_NO_MAGIC_MAIN
#include <allegro5/allegro5.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_ttf.h>

#include <iostream>
#include <memory>
#include <unordered_set>

#include "game.h"
#include "renderer.h"

constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;

int real_main(int argc, char** argv) {
  al_init();
  al_install_keyboard();

  ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
  ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

  al_set_new_display_flags(ALLEGRO_RESIZABLE);
  ALLEGRO_DISPLAY* display = al_create_display(WINDOW_WIDTH, WINDOW_HEIGHT);

  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));

  al_register_event_source(queue, al_get_timer_event_source(timer));

  al_init_primitives_addon();
  al_init_ttf_addon();

  ALLEGRO_FONT* font = al_load_ttf_font("IBMPlexMono-Medium.ttf", 18, 0);
  ALLEGRO_FONT* big_font = al_load_ttf_font("IBMPlexMono-Medium.ttf", 30, 0);

  Game game;
  Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);

  uint32_t last_ticks = al_get_time() * 1000;
  std::unordered_set<Command> commands;

  ALLEGRO_COLOR text_color = al_map_rgb(0, 255, 0);
  ALLEGRO_EVENT event;
  ALLEGRO_KEYBOARD_STATE ks;

  char strbuff[200];
  bool redraw = true;
  int frame = 0;

  al_start_timer(timer);

  bool done = false;
  while (!done) {
    al_wait_for_event(queue, &event);

    if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
      int width = al_get_display_width(display);
      int height = width * 720. / 1280.;
      al_resize_display(display, width, height);
      al_acknowledge_resize(display);
      renderer.reset(width, height);
    } else if (event.type == ALLEGRO_EVENT_TIMER) {
      redraw = true;
    } else if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
      done = true;
    } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
      if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
        game.ship.health = 0;
      }
      if (event.keyboard.keycode == ALLEGRO_KEY_F1) {
        game.debug = !game.debug;
      }
      if (!game.started && event.keyboard.keycode == ALLEGRO_KEY_SPACE) {
        game.started = true;
      }
    }

    al_get_keyboard_state(&ks);

    commands.clear();

    if (al_key_down(&ks, ALLEGRO_KEY_UP) ^ al_key_down(&ks, ALLEGRO_KEY_DOWN) ^
        al_key_down(&ks, ALLEGRO_KEY_W) ^ al_key_down(&ks, ALLEGRO_KEY_S)) {
      commands.insert(al_key_down(&ks, ALLEGRO_KEY_UP) ||
                              al_key_down(&ks, ALLEGRO_KEY_W)
                          ? Command::THRUST_UP
                          : Command::THRUST_DOWN);
    }
    if (al_key_down(&ks, ALLEGRO_KEY_RIGHT) ^
        al_key_down(&ks, ALLEGRO_KEY_LEFT) ^ al_key_down(&ks, ALLEGRO_KEY_A) ^
        al_key_down(&ks, ALLEGRO_KEY_D)) {
      commands.insert(al_key_down(&ks, ALLEGRO_KEY_RIGHT) ||
                              al_key_down(&ks, ALLEGRO_KEY_D)
                          ? Command::THRUST_FORWARD
                          : Command::THRUST_BACKWARD);
    }
    if (al_key_down(&ks, ALLEGRO_KEY_SPACE)) {
      commands.insert({Command::FIRE});
    }
    if (!game.gameover) {
      game.commands(commands);
    }

    uint32_t ticks = al_get_time() * 1000;
    uint32_t dt = ticks - last_ticks;
    game.update(dt);
    last_ticks = ticks;

    if (redraw && al_is_event_queue_empty(queue)) {
      al_clear_to_color(al_map_rgb(0, 0, 0));
      renderer.draw(game);
      if (!game.started) {
        int line = 0;
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "           Welcome!");
        ++line;
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "  ASDF or Arrow Keys to move");
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "        Space to fire");
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "      Escape to give up");
        ++line;
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "Press Space to start the game.");
        ++line;
        al_draw_text(big_font, text_color, 400, 150 + ++line * 30, 0,
                     "          Good Luck");
      }
      if (game.gameover && game.cave.boulders.empty() &&
          game.cave.debris.empty()) {
        al_draw_text(big_font, text_color, 550, 250, 0, "Game Over");
        snprintf(strbuff, sizeof(strbuff), "Final Score: %" PRId64, game.score);
        al_draw_text(big_font, text_color, 550, 310, 0, strbuff);
      }

      if (game.debug) {
        const int fontsize = 18;
        int stri = 0;
        snprintf(strbuff, sizeof(strbuff), "Score: %" PRId64, game.score);
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        // snprintf(strbuff, sizeof(strbuff), "FPS: %.1f", 1000.f / dt);
        snprintf(strbuff, sizeof(strbuff), "HP: %d", game.ship.health);
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Multiplier: %.3f", game.multiplier);
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);

        std::string commands_str = "";
        commands_str += commands.count(Command::THRUST_BACKWARD) ? "<" : " ";
        commands_str += commands.count(Command::THRUST_UP) ? "^" : " ";
        commands_str += commands.count(Command::THRUST_DOWN) ? "v" : " ";
        commands_str += commands.count(Command::THRUST_FORWARD) ? ">" : " ";
        snprintf(strbuff, sizeof(strbuff), "Commands: %s",
                 commands_str.c_str());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);

        snprintf(strbuff, sizeof(strbuff), "Vertical Thrust: %f", game.ship.vy);
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);

        snprintf(strbuff, sizeof(strbuff), "Boulders: %zu",
                 game.cave.boulders.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Bullets: %zu",
                 game.cave.bullets.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Spits: %zu",
                 game.cave.spits.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Debris: %zu",
                 game.cave.debris.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Floor spiders: %zu",
                 game.cave.floor_spiders.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);

        snprintf(strbuff, sizeof(strbuff), "Envelope points: %zu",
                 game.cave.floor_envelope.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
        snprintf(strbuff, sizeof(strbuff), "Collisions: %zu",
                 game.collisions.size());
        al_draw_text(font, text_color, 20, ++stri * fontsize, 0, strbuff);
      }

      al_flip_display();

      redraw = false;
    }

    ++frame;
  }

  al_destroy_font(font);
  al_destroy_display(display);
  al_destroy_timer(timer);
  al_destroy_event_queue(queue);

  return 0;
}

int main(int argc, char** argv) {
  return al_run_main(argc, argv, real_main);
}
