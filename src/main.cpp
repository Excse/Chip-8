#include <iostream>

#include <chip8.h>

#define CHIP_SPEED    500/60
#define SCALE_FACTOR  20

int main () {
  Chip8 chip;

  chip.initialize ();
  chip.load_game ("../resources/roms/games/Pong [Paul Vervalin, 1990].ch8");

  chip.initialize_display (SCALE_FACTOR);

  uint32_t start_ticks = SDL_GetTicks ();

  auto running = true;
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent (&event)) {
      switch (event.type) {
      case SDL_QUIT: {
        running = false;
        continue;
      }
      case SDL_KEYDOWN: {
        chip.press_key (event.key.keysym.sym);
        break;
      }
      case SDL_KEYUP: {
        chip.release_key (event.key.keysym.sym);
        break;
      }
      default: break;
      }
    }

    uint32_t end_ticks = SDL_GetTicks ();
    double delta = end_ticks - start_ticks;
    if (delta > 1000.0 / 60.0) {
      start_ticks = end_ticks;

      for (auto index = 0; index < CHIP_SPEED; index++) {
        chip.cycle ();
      }

      chip.draw (SCALE_FACTOR);
    }
  }

  return EXIT_SUCCESS;
}
