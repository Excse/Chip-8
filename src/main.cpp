#include <iostream>

#include "cxxopts.hpp"

#include <chip8.h>

#define CHIP_SPEED    500/60
#define SCALE_FACTOR  20

auto main (int argc, char **argv) noexcept -> int {
  cxxopts::Options options ("Chip-8", "A quick Chip-8 implementation to test out emulator "
                                      "development.");

  options.add_options ()
      ("i,input", "The file containing the Chip-8 instructions.", cxxopts::value<std::string> ());

  options.custom_help ("[options]");
  options.parse_positional ({"input"});
  options.positional_help ("<input>");

  cxxopts::ParseResult result;
  try {
    result = options.parse (argc, argv);
  }
  catch (...) {
    std::cout << options.help () << std::endl;
    exit (0);
  }

  if (result.count ("help") || !result.count ("input")) {
    std::cout << options.help () << std::endl;
    exit (0);
  }

  Chip8 chip;
  chip.initialize ();

  auto input_path = result["input"].as<std::string> ();
  chip.load_game (input_path);

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
