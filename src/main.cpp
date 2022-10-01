#include <iostream>

#include "cxxopts.hpp"

#include <chip8.h>

auto main (int argc, char **argv) noexcept -> int {
  cxxopts::Options options ("Chip-8", "A quick Chip-8 implementation to test out emulator "
                                      "development.");

  options.add_options ()
      ("i,input", "The file containing the Chip-8 instructions.", cxxopts::value<std::string> ())
      ("c,cycles", "Defines how many cycles you want to execute each frame.",
       cxxopts::value<uint64_t> ()->default_value ("10"))
      ("s,scale", "Sets the factor which the pixels will get scaled by.",
       cxxopts::value<uint64_t> ()->default_value ("20"))
      ("f,fps", "Sets the rate of frames per second.",
       cxxopts::value<uint64_t> ()->default_value ("60"));

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

  auto input_path = result["input"].as<std::string> ();
  auto scale_factor = result["scale"].as<uint64_t> ();
  auto cycles = result["cycles"].as<uint64_t> ();
  auto fps = result["fps"].as<uint64_t> ();

  Chip8 chip;
  chip.initialize ();
  chip.load_game (input_path);
  chip.initialize_display (scale_factor);

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
    if (delta > 1000.0 / fps) {
      start_ticks = end_ticks;

      for (auto index = 0u; index < cycles; index++) {
        chip.cycle ();
      }

      chip.draw (scale_factor);
    }
  }

  return EXIT_SUCCESS;
}
