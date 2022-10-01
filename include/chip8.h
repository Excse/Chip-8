//
// Created by timo on 24.09.22.
//

#ifndef _CHIP8_H_
#define _CHIP8_H_

#include <memory>
#include <string>
#include <set>

#include <gtest/gtest_prod.h>
#include <SDL2/SDL.h>

#define RAM_SIZE      4096
#define STACK_SIZE    16
#define KEYPAD_SIZE   16
#define V_REGISTERS   16

#define MEMORY_PROGRAM_START 0x200

#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH  64

inline std::set<uint8_t> KEY_MAP = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v,
};

inline std::array<uint8_t, 80> FONTSET = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

/**
 * @brief Stores the data of a single instruction.
 */
struct Instruction {
  uint16_t opcode;
  uint16_t nnn: 12;
  uint8_t x: 4;
  uint8_t y: 4;
  uint8_t kk;
  uint8_t n: 4;
};

/**
 * @brief The main class used for the entire Chip-8 emulation.
 */
class Chip8 {
  friend class InstructionTest;
 public:
  Chip8 ();

  virtual ~Chip8 ();

  /**
   * Sets all the variables to the default state.
   */
  void initialize ();

  /**
   * Initializes the SDL window and renderer. The scaling factor is needed to compute the real
   * size of the window.
   *
   * @param scaling_factor The factor used by which the pixels are getting scaled.
   */
  void initialize_display (uint8_t scaling_factor);

  /**
   * Loads a game from a file by copying the bytes into the RAM.
   *
   * @param [in] path The location of the file to load the instructions from.
   */
  void load_game (const std::string &path);

  /**
   * Draws the entire display array to the window. As 64x32 pixels is pretty small everything is
   * getting scaled by the given factor.
   *
   * @param [in] scaling_factor Scales up the pixels by this factor.
   */
  void draw (uint8_t scaling_factor);

  /**
   * It will perform a full cycle of the Chip-8. It will fetch, decode and execute an instruction.
   */
  void cycle ();

  /**
   * Using this method a key will be marked as pressed (true) if it wasn't already.
   *
   * @param [in] keysym The key which was released.
   */
  void press_key (uint8_t keysym);

  /**
   * Using this method a key will be marked as released (false) if it was pressed already.
   *
   * @param [in] keysym The key which was released.
   */
  void release_key (uint8_t keysym);

 private:
  /**
   * Executes the instruction based on its opcode.
   *
   * @param [in] instruction The instruction with the opcode and all its fields (x, y, nnn, n, kk).
   */
  void execute (const Instruction &instruction);

  /**
   * Used to clear the screen entirely.
   */
  void _00E0 ();
  FRIEND_TEST(InstructionTest, FullyClearsScreen);

  /**
   * This instruction will set the program counter to the popped return address from the stack.
   */
  void _00EE ();
  FRIEND_TEST(InstructionTest, SuccessfullyReturnsSubroutine);

  /**
   * The program counter will be set to the given memory location.
   *
   * @param [in] address The absolute memory location to jump to.
   */
  void _1nnn (uint16_t address);
  FRIEND_TEST(InstructionTest, JumpsToAddress);

  /**
   * Calls the subroutine at the given address. But before doing that the current program counter
   * will be pushed to the stack.
   *
   * @param [in] address The absolute memory location where the subroutine begins.
   */
  void _2nnn (uint16_t address);
  FRIEND_TEST(InstructionTest, SuccessfullyCalledSubroutine);

  /**
   * Skips the next instruction if the contents of the provided register (Vx) is equal to the
   * constant.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   * @param [in] constant   The immediate which is used to compare to the registers value.
   */
  void _3xkk (uint8_t x_register, uint8_t constant);
  FRIEND_TEST(InstructionTest, SkipIfXEqToConst_True);
  FRIEND_TEST(InstructionTest, SkipIfXEqToConst_TrueFalse);

  /**
   * Other than the 3xkk instruction this one will skip the next instruction if the contents of the
   * provided register (Vx) is NOT equal to the constant.
   *
   * @param x_register The index for the register in a range from 0x0 to 0xF.
   * @param constant   The immediate which is used to compare to the registers value.
   */
  void _4xkk (uint8_t x_register, uint8_t constant);
  FRIEND_TEST(InstructionTest, SkipIfXNotEqToConstant_True);
  FRIEND_TEST(InstructionTest, SkipIfXNotEqToConstant_False);

  /**
   * Skips the next instruction if the value of register x (Vx) is equal to the value of
   * register y (Vy).
   *
   * @param x_register The index for the x register in a range from 0x0 to 0xF.
   * @param y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _5xy0 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, SkipIfXEqToY_True);
  FRIEND_TEST(InstructionTest, SkipIfXEqToY_False);

  /**
   * Stores the given constant in the register (Vx).
   *
   * @param x_register The index for the register in a range from 0x0 to 0xF.
   * @param constant   The immediate value which is then stored in the register.
   */
  void _6xkk (uint8_t x_register, uint8_t constant);
  FRIEND_TEST(InstructionTest, LoadConstIntoX);

  /**
   * Adds the constant to the value in the register (Vx) and then stores the result in the register.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   * @param [in] constant   The immediate value which is added to the registers value.
   */
  void _7xkk (uint8_t x_register, uint8_t constant);
  FRIEND_TEST(InstructionTest, AddConstantToX);

  /**
   * Stores the value inside register y (Vy) in the register x (Vx).
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy0 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, StoreYIntoX);

  /**
   * This instruction will perform a logical or of register x (Vx) with register y (Vy).
   * Afterwards the result will be stored in register x.
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy1 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, OrXWithY);

  /**
   * This instruction will perform a logical and of register x (Vx) with register y (Vy).
   * Afterwards the result will be stored in register x.
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy2 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, AndXWithYRegister);

  /**
   * This instruction will perform a XOR of register x (Vx) with register y (Vy).
   * Afterwards the result will be stored in register x.
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy3 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, XORXWithY);

  /**
   * Adds the contents of register y (Vy) to register x (Vx) and will set the register f (Vf) if
   * a carry occurred. Afterwards the result will be stored in register x (Vx).
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy4 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, AddYToXNoCarry);
  FRIEND_TEST(InstructionTest, AddYToXWithCarry);

  /**
   * Subtracts the contents of register y (Vy) from register x (Vx) and will set the register f (Vf)
   * if no borrow occurred. Afterwards the result will be stored in register x (Vx).
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy5 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, SubYFromXNoBorrow);
  FRIEND_TEST(InstructionTest, SubYFromXWithBorrow);

  /**
   * Divides the contents of register x (Vx) by 2 using a shift right operation. The least
   * significant bit will tell whether you can divide the number evenly. Thus register f (Vf)
   * will be set to this value.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void _8xy6 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, DivXBy2NoLSB);
  FRIEND_TEST(InstructionTest, DivXBy2WithLSB);

  /**
   * Subtracts the contents of register x (Vx) from register y (Vy) and will set the register f (Vf)
   * if no borrow occurred. Afterwards the result will be stored in register x (Vx).
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _8xy7 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, SubXFromYNoBorrow);
  FRIEND_TEST(InstructionTest, SubXFromYWithBorrow);

  /**
   * Multiplies the contents of register x (Vx) by 2 using a shift left operation. The most
   * significant bit will tell whether this operation will result in 0 (overflow). Thus register f
   * (Vf) will be set to this value.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void _8xyE (uint8_t x_register);
  FRIEND_TEST(InstructionTest, MulXBy2NoMSB);
  FRIEND_TEST(InstructionTest, MulXBy2WithMSB);

  /**
   * Skips the next instruction if the value of register x (Vx) is not equal to the value of
   * register y (Vy).
   *
   * @param [in] x_register The index for the x register in a range from 0x0 to 0xF.
   * @param [in] y_register The index for the y register in a range from 0x0 to 0xF.
   */
  void _9xy0 (uint8_t x_register, uint8_t y_register);
  FRIEND_TEST(InstructionTest, SkipIfXNotEqToY_True);
  FRIEND_TEST(InstructionTest, SkipIfXNotEqToY_False);

  /**
   * Stores the given address in register I.
   *
   * @param [in] address The absolute memory location which is stored in register I.
   */
  void Annn (uint16_t address);
  FRIEND_TEST(InstructionTest, LoadMemoryAddress);

  /**
   * Jumps to the given address relative to the register 0 (V0).
   *
   * @param [in] address The relative memory location which is added to register 0.
   */
  void Bnnn (uint16_t address);
  FRIEND_TEST(InstructionTest, JumpAddressRelativeToV0);

  /**
   * Generates a random number which is then logical ANDed with the given constant. The result
   * will then be stored in register x (Vx).
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   * @param [in] constant   The immediate value which is used to AND the random number.
   */
  void Cxkk (uint8_t x_register, uint8_t constant);
  FRIEND_TEST(InstructionTest, AndRandomNumberWithConstant);

  /**
   * Display a n-byte sprite located at memory location I. The register x (Vx) will be used as x
   * position and register y (Vy) for the y position. If a collision occured register f (Vf) will
   * be set.
   *
   * @param [in] x_register The value contained in this register (a value in range from 0x0 to 0xF)
   *                        is the x position on the screen.
   * @param [in] y_register The value contained in this register (a value in range from 0x0 to 0xF)
   *                        is the y position on the screen.
   * @param [in] bytes      Defines how many bytes will be read relative to register I.
   */
  void Dxyn (uint8_t x_register, uint8_t y_register, uint8_t bytes);
  FRIEND_TEST(InstructionTest, DrawNSpritesAtXY);

  /**
   * Skips the next instruction if the key equals to the value of register x (Vx).
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Ex9E (uint8_t x_register);
  FRIEND_TEST(InstructionTest, SkipIfXKeyIsPressed_True);
  FRIEND_TEST(InstructionTest, SkipIfXKeyIsPressed_False);

  /**
   * Skips the next instruction if the key doesn't equals to the value of register x (Vx).
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void ExA1 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, SkipIfXKeyIsNotPressed_True);
  FRIEND_TEST(InstructionTest, SkipIfXKeyIsNotPressed_False);

  /**
   * Stores the value of the delay timer register into the provided register x (Vx).
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx07 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreDelayTimerIntoX);

  /**
   * Waits until a key is pressed. This is achieved by going back a instruction if no key has
   * been pressed. But if a key was pressed the keymap index will be stored in the register x (Vx).
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx0A (uint8_t x_register);
  FRIEND_TEST(InstructionTest, WaitTillKeyPressedThenStoreIntoX_True);
  FRIEND_TEST(InstructionTest, WaitTillKeyPressedThenStoreIntoX_False);

  /**
   * Stores the value inside register x (Vx) into the delay timer register.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx15 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreXIntoDelayTimer);

  /**
   * Stores the value inside register x (Vx) into the sound timer register.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx18 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreXIntoSoundTimer);

  /**
   * Adds the contents of register x (Vx) to register I and also stores the result in it.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx1E (uint8_t x_register);
  FRIEND_TEST(InstructionTest, AddXToI);

  /**
   * Sets register I to the memory location where the sprite for the number in register x (Vx) is
   * located at.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx29 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, SetIToNumberSprite);

  /**
   * Stores a BCD representation of the number stored in register x (Vx) in the first three
   * memory locations relative to register I.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx33 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreBCD);

  /**
   * Stores all registers from 0 to x (V0-Vx) at the first x memory locations relative to
   * register I.
   * Afterwards register I will be increased by x + 1.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx55 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreRegsToXToI);

  /**
   * Stores the first x bytes located relative to register I in memory into all registers from 0
   * to x (V0-Vx).
   * Afterwards register I will be increased by x + 1.
   *
   * @param [in] x_register The index for the register in a range from 0x0 to 0xF.
   */
  void Fx65 (uint8_t x_register);
  FRIEND_TEST(InstructionTest, StoreIToXIntoRegs);

 private:
  SDL_Renderer *renderer_;
  SDL_Window *window_;

  std::array<bool, SCREEN_WIDTH * SCREEN_HEIGHT> display_;
  bool draw_flag_;

  std::array<uint8_t, KEYPAD_SIZE> keypad_;

  std::array<uint8_t, RAM_SIZE> memory_;
  uint16_t program_counter_;

  std::array<uint16_t, STACK_SIZE> stack_;
  uint8_t stack_pointer_;

  std::array<uint8_t, V_REGISTERS> V_;
  uint8_t delay_timer_, sound_timer_;
  uint16_t I_: 12;
};

#endif //_CHIP8_H_
