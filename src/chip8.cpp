//
// Created by timo on 24.09.22.
//

#include "chip8.h"

#include <iostream>
#include <fstream>

Chip8::Chip8 () :
    renderer_ (), window_ (), display_ (), draw_flag_ (), keypad_ (), memory_ (),
    program_counter_ (), stack_ (), stack_pointer_ (), V_ (), delay_timer_ (),
    sound_timer_ (), I_ () {}

Chip8::~Chip8 () {
  SDL_DestroyRenderer (this->renderer_);
  SDL_DestroyWindow (this->window_);
  SDL_Quit ();
}

void Chip8::initialize () {
  this->program_counter_ = MEMORY_PROGRAM_START;
  this->stack_pointer_ = 0;
  this->I_ = 0;

  this->display_.fill (false);
  this->memory_.fill (0);
  this->stack_.fill (0);
  this->V_.fill (0);

  this->delay_timer_ = 0;
  this->sound_timer_ = 0;

  for (auto index = 0u; index < FONTSET.size (); index++) {
    this->memory_[index] = FONTSET[index];
  }
}

void Chip8::load_game (const std::string &path) {
  std::ifstream game_file (path, std::ios::in | std::ios::binary);
  for (size_t index = MEMORY_PROGRAM_START; game_file.good (); index++) {
    this->memory_[index] = game_file.get ();
  }
}

void Chip8::initialize_display (uint8_t scaling_factor) {
  if (SDL_Init (SDL_INIT_EVERYTHING) < 0) {
    std::cerr << "SDL couldn't be initialized! SDL_Error: " << SDL_GetError () << std::endl;
    exit (1);
  }

  this->window_ = SDL_CreateWindow ("CHIP-8",
                                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                    SCREEN_WIDTH * scaling_factor, SCREEN_HEIGHT * scaling_factor,
                                    SDL_WINDOW_SHOWN);
  if (this->window_ == nullptr) {
    std::cerr << "Window couldn't be created! SDL_Error: " << SDL_GetError () << std::endl;
    exit (1);
  }

  this->renderer_ = SDL_CreateRenderer (this->window_, -1, SDL_RENDERER_ACCELERATED);
  SDL_RenderSetLogicalSize (this->renderer_,
                            SCREEN_WIDTH * scaling_factor, SCREEN_HEIGHT * scaling_factor);
}

void Chip8::draw (uint8_t scaling_factor) {
  SDL_SetRenderDrawColor (this->renderer_, 0, 0, 0, 255);
  SDL_RenderClear (this->renderer_);

  SDL_SetRenderDrawColor (this->renderer_, 255, 255, 255, 255);

  SDL_Rect scaled_pixel;
  for (auto index = 0u; index < SCREEN_WIDTH * SCREEN_HEIGHT; index++) {
    if (!this->display_[index]) {
      continue;
    }

    auto pixel_x = (int)(index % SCREEN_WIDTH);
    auto pixel_y = (int)(index / SCREEN_WIDTH);

    scaled_pixel.x = pixel_x * scaling_factor;
    scaled_pixel.y = pixel_y * scaling_factor;
    scaled_pixel.w = scaling_factor;
    scaled_pixel.h = scaling_factor;

    SDL_RenderFillRect (this->renderer_, &scaled_pixel);
  }

  SDL_RenderPresent (this->renderer_);
}

void Chip8::cycle () {
  uint16_t opcode = memory_[this->program_counter_] << 8 | memory_[this->program_counter_ + 1];

  Instruction instruction{
      opcode,
      (uint16_t)(opcode & 0x0FFF),
      (uint8_t)((opcode & 0x0F00) >> 8),
      (uint8_t)((opcode & 0x00F0) >> 4),
      (uint8_t)(opcode & 0x00FF),
      (uint8_t)(opcode & 0x00F)
  };

  this->program_counter_ += 2;
  this->execute (instruction);

  if (this->delay_timer_ > 0) {
    this->delay_timer_--;
  }

  if (this->sound_timer_ > 0) {
    // TODO: Make a sound
    this->sound_timer_--;
  }
}

void Chip8::press_key (uint8_t keysym) {
  auto found = KEY_MAP.find (keysym);
  if (found == KEY_MAP.end ()) {
    return;
  }

  auto index = (size_t)std::distance (KEY_MAP.begin (), found);
  this->keypad_[index] = true;
}

void Chip8::release_key (uint8_t keysym) {
  auto found = KEY_MAP.find (keysym);
  if (found == KEY_MAP.end ()) {
    return;
  }

  auto index = (size_t)std::distance (KEY_MAP.begin (), found);
  this->keypad_[index] = false;
}

void Chip8::execute (const Instruction &instruction) {
  const auto &[opcode, nnn, x, y, kk, n] = instruction;
  switch (opcode >> 12) {
  case 0x0: {
    switch (instruction.opcode) {
    case 0x00E0: return this->_00E0 ();
    case 0x00EE: return this->_00EE ();
    }
  }
  case 0x1: return this->_1nnn (nnn);
  case 0x2: return this->_2nnn (nnn);
  case 0x3: return this->_3xkk (x, kk);
  case 0x4: return this->_4xkk (x, kk);
  case 0x5: return this->_5xy0 (x, y);
  case 0x6: return this->_6xkk (x, kk);
  case 0x7: return this->_7xkk (x, kk);
  case 0x8: {
    switch (opcode & 0x000F) {
    case 0x0: return this->_8xy0 (x, y);
    case 0x1: return this->_8xy1 (x, y);
    case 0x2: return this->_8xy2 (x, y);
    case 0x3: return this->_8xy3 (x, y);
    case 0x4: return this->_8xy4 (x, y);
    case 0x5: return this->_8xy5 (x, y);
    case 0x6: return this->_8xy6 (x);
    case 0x7: return this->_8xy7 (x, y);
    case 0xE: return this->_8xyE (x);
    }
  }
  case 0x9: return this->_9xy0 (x, y);
  case 0xA: return this->Annn (nnn);
  case 0xB: return this->Bnnn (nnn);
  case 0xC: return this->Cxkk (x, kk);
  case 0xD: return this->Dxyn (x, y, n);
  case 0xE: {
    switch (opcode & 0x00FF) {
    case 0x9E: return this->Ex9E (x);
    case 0xA1: return this->ExA1 (x);
    }
  }
  case 0xF: {
    switch (opcode & 0x00FF) {
    case 0x07: return this->Fx07 (x);
    case 0x0A: return this->Fx0A (x);
    case 0x15: return this->Fx15 (x);
    case 0x18: return this->Fx18 (x);
    case 0x1E: return this->Fx1E (x);
    case 0x29: return this->Fx29 (x);
    case 0x33: return this->Fx33 (x);
    case 0x55: return this->Fx55 (x);
    case 0x65: return this->Fx65 (x);
    }
  }
  }

  std::cerr << "This instruction is not implemented! " << std::hex << (int)opcode << std::endl;
  exit (1);
}

void Chip8::_00E0 () {
  this->display_.fill (false);
}

void Chip8::_00EE () {
  auto return_address = this->stack_[--this->stack_pointer_];
  this->program_counter_ = return_address;
}

void Chip8::_1nnn (uint16_t address) {
  this->program_counter_ = address;
}

void Chip8::_2nnn (uint16_t address) {
  auto return_address = this->program_counter_;
  this->stack_[this->stack_pointer_++] = return_address;

  this->program_counter_ = address;
}

void Chip8::_3xkk (uint8_t x_register, uint8_t constant) {
  auto x_value = this->V_[x_register];
  if (x_value == constant) {
    this->program_counter_ += 2;
  }
}

void Chip8::_4xkk (uint8_t x_register, uint8_t constant) {
  auto x_value = this->V_[x_register];
  if (x_value != constant) {
    this->program_counter_ += 2;
  }
}

void Chip8::_5xy0 (uint8_t x_register, uint8_t y_register) {
  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];
  if (x_value == y_value) {
    this->program_counter_ += 2;
  }
}

void Chip8::_6xkk (uint8_t x_register, uint8_t constant) {
  this->V_[x_register] = constant;
}

void Chip8::_7xkk (uint8_t x_register, uint8_t constant) {
  this->V_[x_register] += constant;
}

void Chip8::_8xy0 (uint8_t x_register, uint8_t y_register) {
  auto y_value = this->V_[y_register];
  this->V_[x_register] = y_value;
}

void Chip8::_8xy1 (uint8_t x_register, uint8_t y_register) {
  auto y_value = this->V_[y_register];
  this->V_[x_register] |= y_value;
}

void Chip8::_8xy2 (uint8_t x_register, uint8_t y_register) {
  auto y_value = this->V_[y_register];
  this->V_[x_register] &= y_value;
}

void Chip8::_8xy3 (uint8_t x_register, uint8_t y_register) {
  auto y_value = this->V_[y_register];
  this->V_[x_register] ^= y_value;
}

void Chip8::_8xy4 (uint8_t x_register, uint8_t y_register) {
  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];

  auto set_carry = y_value > 0xFF - x_value;
  this->V_[0xF] = set_carry;

  this->V_[x_register] += y_value;
}

void Chip8::_8xy5 (uint8_t x_register, uint8_t y_register) {
  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];

  auto set_borrow = x_value < y_value;
  this->V_[0xF] = !set_borrow;

  this->V_[x_register] -= y_value;
}

void Chip8::_8xy6 (uint8_t x_register) {
  auto x_value = this->V_[x_register];

  uint8_t lsb_x = x_value & 0b1;
  this->V_[0xF] = lsb_x;

  this->V_[x_register] >>= 1;
}

void Chip8::_8xy7 (uint8_t x_register, uint8_t y_register) {
  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];

  auto set_borrow = y_value < x_value;
  this->V_[0xF] = !set_borrow;

  this->V_[x_register] = y_value - x_value;
}

void Chip8::_8xyE (uint8_t x_register) {
  auto x_value = this->V_[x_register];

  uint8_t msb_x = x_value >> 7;
  this->V_[0xF] = msb_x;

  this->V_[x_register] <<= 1;
}

void Chip8::_9xy0 (uint8_t x_register, uint8_t y_register) {
  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];
  if (x_value != y_value) {
    this->program_counter_ += 2;
  }
}

void Chip8::Annn (uint16_t address) {
  this->I_ = address;
}

void Chip8::Bnnn (uint16_t address) {
  auto V0_value = this->V_[0x0];

  this->program_counter_ = V0_value + address;
}

void Chip8::Cxkk (uint8_t x_register, uint8_t constant) {
  uint8_t random_number = std::rand () % 256;
  this->V_[x_register] = random_number & constant;
}

void Chip8::Dxyn (uint8_t x_register, uint8_t y_register, uint8_t bytes) {
  this->V_[0xF] = 0;

  auto x_value = this->V_[x_register];
  auto y_value = this->V_[y_register];

  for (auto sprite_index = 0u; sprite_index < bytes; sprite_index++) {
    auto sprite = this->memory_[this->I_ + sprite_index];
    for (auto bit_index = 0u; bit_index < 8; bit_index++) {
      auto selected_bit = sprite & (0x80 >> bit_index);
      if (selected_bit == 0) {
        continue;
      }

      auto index = ((x_value + bit_index) + ((y_value + sprite_index) * SCREEN_WIDTH))
                   % (SCREEN_WIDTH * SCREEN_HEIGHT);
      if (this->display_[index]) {
        this->V_[0xF] = 1;
      }

      this->display_[index] ^= 1;
    }
  }

  this->draw_flag_ = true;
}

void Chip8::Ex9E (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  if (this->keypad_[x_value]) {
    this->program_counter_ += 2;
  }
}

void Chip8::ExA1 (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  if (!this->keypad_[x_value]) {
    this->program_counter_ += 2;
  }
}

void Chip8::Fx07 (uint8_t x_register) {
  this->V_[x_register] = this->delay_timer_;
}

void Chip8::Fx0A (uint8_t x_register) {
  auto found_key = -1;
  for (auto index = 0u; index < this->keypad_.size (); index++) {
    const auto &key = this->keypad_.at (index);
    if (key) {
      found_key = (int)index;
      break;
    }
  }

  if (found_key == -1) {
    this->program_counter_ -= 2;
  } else {
    this->V_[x_register] = found_key;
  }
}

void Chip8::Fx15 (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  this->delay_timer_ = x_value;
}

void Chip8::Fx18 (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  this->sound_timer_ = x_value;
}

void Chip8::Fx1E (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  this->I_ += x_value;
}

void Chip8::Fx29 (uint8_t x_register) {
  auto x_value = this->V_[x_register];
  this->I_ = x_value * 5;
}

void Chip8::Fx33 (uint8_t x_register) {
  auto x_value = this->V_[x_register];

  this->memory_[this->I_ + 0] = x_value / 100;
  this->memory_[this->I_ + 1] = (x_value / 10) % 10;
  this->memory_[this->I_ + 2] = x_value % 10;
}

void Chip8::Fx55 (uint8_t x_register) {
  for (auto index = 0u; index <= x_register; index++) {
    this->memory_[this->I_ + index] = this->V_[index];
  }

  this->I_ += x_register + 1;
}

void Chip8::Fx65 (uint8_t x_register) {
  for (auto index = 0u; index <= x_register; index++) {
    this->V_[index] = this->memory_[this->I_ + index];
  }

  this->I_ += x_register + 1;
}
