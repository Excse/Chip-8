//
// Created by timo on 24.09.22.
//

#include "chip8.h"

#include <iostream>

#include "gtest/gtest.h"

#define AFTER_INSTRUCTION_PC MEMORY_PROGRAM_START + 2

class InstructionTest : public ::testing::Test {
 public:
  InstructionTest () : chip_ () {
    this->chip_.initialize ();

    // To simulate a cycle
    this->chip_.program_counter_ += 2;
  }

 protected:
  Chip8 chip_;
};

TEST_F(InstructionTest, FullyClearsScreen) {
  for (auto &pixel : this->chip_.display_) {
    pixel = true;
  }

  this->chip_._00E0 ();

  for (const auto &pixel : this->chip_.display_) {
    EXPECT_FALSE(pixel);
  }
}

TEST_F(InstructionTest, SuccessfullyReturnsSubroutine) {
  this->chip_.stack_[this->chip_.stack_pointer_++] = this->chip_.program_counter_;
  this->chip_.program_counter_ = AFTER_INSTRUCTION_PC + 42;

  this->chip_._00EE ();

  ASSERT_EQ(this->chip_.stack_pointer_, 0);
  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, JumpsToAddress) {
  this->chip_._1nnn (AFTER_INSTRUCTION_PC + 42);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 42);
}

TEST_F(InstructionTest, SuccessfullyCalledSubroutine) {
  auto next_address = AFTER_INSTRUCTION_PC + 42;

  this->chip_._2nnn (next_address);

  ASSERT_EQ(this->chip_.program_counter_, next_address);

  ASSERT_EQ(this->chip_.stack_pointer_, 1);
  auto stack_top = this->chip_.stack_[this->chip_.stack_pointer_ - 1];
  ASSERT_EQ(stack_top, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, SkipIfXEqToConst_True) {
  this->chip_.V_[0x0] = 42;
  this->chip_._3xkk (0x0, 42);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 2);
}

TEST_F(InstructionTest, SkipIfXEqToConst_TrueFalse) {
  this->chip_.V_[0x0] = 42;
  this->chip_._3xkk (0x0, 40);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, SkipIfXNotEqToConstant_True) {
  this->chip_.V_[0x0] = 42;
  this->chip_._4xkk (0x0, 40);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 2);
}

TEST_F(InstructionTest, SkipIfXNotEqToConstant_False) {
  this->chip_.V_[0x0] = 42;
  this->chip_._4xkk (0x0, 42);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, SkipIfXEqToY_True) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 42;
  this->chip_._5xy0 (0x0, 0x1);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 2);
}

TEST_F(InstructionTest, SkipIfXEqToY_False) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 40;
  this->chip_._5xy0 (0x0, 0x1);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, LoadConstIntoX) {
  this->chip_._6xkk (0x0, 42);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, AddConstantToX) {
  this->chip_.V_[0x0] = 21;
  this->chip_._7xkk (0x0, 21);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, StoreYIntoX) {
  this->chip_.V_[0x1] = 42;
  this->chip_._8xy0 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, OrXWithY) {
  this->chip_.V_[0x0] = 0b00100010;
  this->chip_.V_[0x1] = 0b00001000;
  this->chip_._8xy1 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, AndXWithYRegister) {
  this->chip_.V_[0x0] = 0b10101110;
  this->chip_.V_[0x1] = 0b01101010;
  this->chip_._8xy2 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, XORXWithY) {
  this->chip_.V_[0x0] = 0b01001100;
  this->chip_.V_[0x1] = 0b01100110;
  this->chip_._8xy3 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, AddYToXNoCarry) {
  this->chip_.V_[0x0] = 21;
  this->chip_.V_[0x1] = 21;
  this->chip_._8xy4 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 42);
  ASSERT_EQ(this->chip_.V_[0xF], 0);
}

TEST_F(InstructionTest, AddYToXWithCarry) {
  this->chip_.V_[0x0] = 128;
  this->chip_.V_[0x1] = 128;
  this->chip_._8xy4 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 0);
  ASSERT_EQ(this->chip_.V_[0xF], 1);
}

TEST_F(InstructionTest, SubYFromXNoBorrow) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 42;
  this->chip_._8xy5 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 0);
  ASSERT_EQ(this->chip_.V_[0xF], 1);
}

TEST_F(InstructionTest, SubYFromXWithBorrow) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 43;
  this->chip_._8xy5 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 255);
  ASSERT_EQ(this->chip_.V_[0xF], 0);
}

TEST_F(InstructionTest, DivXBy2NoLSB) {
  this->chip_.V_[0x0] = 0b00101010;
  this->chip_._8xy6 (0x0);

  ASSERT_EQ(this->chip_.V_[0x0], 0b00010101);
  ASSERT_EQ(this->chip_.V_[0xF], 0);
}

TEST_F(InstructionTest, DivXBy2WithLSB) {
  this->chip_.V_[0x0] = 0b00101011;
  this->chip_._8xy6 (0x0);

  ASSERT_EQ(this->chip_.V_[0x0], 0b00010101);
  ASSERT_EQ(this->chip_.V_[0xF], 1);
}

TEST_F(InstructionTest, SubXFromYNoBorrow) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 42;
  this->chip_._8xy7 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 0);
  ASSERT_EQ(this->chip_.V_[0xF], 1);
}

TEST_F(InstructionTest, SubXFromYWithBorrow) {
  this->chip_.V_[0x0] = 43;
  this->chip_.V_[0x1] = 42;
  this->chip_._8xy7 (0x0, 0x1);

  ASSERT_EQ(this->chip_.V_[0x0], 255);
  ASSERT_EQ(this->chip_.V_[0xF], 0);
}

TEST_F(InstructionTest, MulXBy2NoMSB) {
  this->chip_.V_[0x0] = 0b01000000;
  this->chip_._8xyE (0x0);

  ASSERT_EQ(this->chip_.V_[0x0], 0b10000000);
  ASSERT_EQ(this->chip_.V_[0xF], 0);
}

TEST_F(InstructionTest, MulXBy2WithMSB) {
  this->chip_.V_[0x0] = 0b10000000;
  this->chip_._8xyE (0x0);

  ASSERT_EQ(this->chip_.V_[0x0], 0);
  ASSERT_EQ(this->chip_.V_[0xF], 1);
}

TEST_F(InstructionTest, SkipIfXNotEqToY_True) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 42;
  this->chip_._9xy0 (0x0, 0x1);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
}

TEST_F(InstructionTest, SkipIfXNotEqToY_False) {
  this->chip_.V_[0x0] = 42;
  this->chip_.V_[0x1] = 40;
  this->chip_._9xy0 (0x0, 0x1);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 2);
}

TEST_F(InstructionTest, LoadMemoryAddress) {
  this->chip_.Annn (AFTER_INSTRUCTION_PC + 42);

  ASSERT_EQ(this->chip_.I_, AFTER_INSTRUCTION_PC + 42);
}

TEST_F(InstructionTest, JumpAddressRelativeToV0) {
  this->chip_.V_[0x0] = 2;
  this->chip_.Bnnn (AFTER_INSTRUCTION_PC + 40);

  ASSERT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + 42);
}

TEST_F(InstructionTest, AndRandomNumberWithConstant) {
  // TODO: W.I.P
}

TEST_F(InstructionTest, DrawNSpritesAtXY) {
  this->chip_.I_ = AFTER_INSTRUCTION_PC;

  this->chip_.memory_[this->chip_.I_] = 0b10101010;

  this->chip_.Dxyn (0, 0, 2);

  EXPECT_TRUE(this->chip_.draw_flag_);
  EXPECT_FALSE(this->chip_.V_[0x0F]);

  for (auto index = 0u; index < 8; index++) {
    ASSERT_EQ(this->chip_.display_[index], index % 2 == 0);
  }

  this->chip_.Dxyn (0, 0, 2);

  EXPECT_TRUE(this->chip_.draw_flag_);
  EXPECT_TRUE(this->chip_.V_[0x0F]);

  for (auto index = 0u; index < 8; index++) {
    EXPECT_FALSE(this->chip_.display_[index]);
  }
}

TEST_F(InstructionTest, SkipIfXKeyIsPressed_True) {
  this->chip_.keypad_.fill (true);

  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.V_[0x0] = index;
    this->chip_.Ex9E (index);

    EXPECT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + (index + 1) * 2);
  }
}

TEST_F(InstructionTest, SkipIfXKeyIsPressed_False) {
  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.V_[0x0] = index;
    this->chip_.Ex9E (index);

    EXPECT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
  }
}

TEST_F(InstructionTest, SkipIfXKeyIsNotPressed_True) {
  this->chip_.keypad_.fill (true);

  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.V_[0x0] = index;
    this->chip_.ExA1 (index);

    EXPECT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC);
  }
}

TEST_F(InstructionTest, SkipIfXKeyIsNotPressed_False) {
  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.V_[0x0] = index;
    this->chip_.ExA1 (index);

    EXPECT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC + (index + 1) * 2);
  }
}

TEST_F(InstructionTest, StoreDelayTimerIntoX) {
  this->chip_.delay_timer_ = 42;
  this->chip_.Fx07 (0x0);

  EXPECT_EQ(this->chip_.V_[0x0], 42);
}

TEST_F(InstructionTest, WaitTillKeyPressedThenStoreIntoX_True) {
  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.keypad_[index] = true;
    this->chip_.Fx0A (0x0);

    EXPECT_EQ(this->chip_.V_[0x0], index);

    this->chip_.keypad_[index] = false;
  }
}

TEST_F(InstructionTest, WaitTillKeyPressedThenStoreIntoX_False) {
  for (auto index = 0u; index < this->chip_.keypad_.size (); index++) {
    this->chip_.Fx0A (0x0);

    EXPECT_EQ(this->chip_.program_counter_, AFTER_INSTRUCTION_PC - ((index + 1) * 2));
  }
}

TEST_F(InstructionTest, StoreXIntoDelayTimer) {
  this->chip_.V_[0x0] = 42;
  this->chip_.Fx15 (0x0);

  EXPECT_EQ(this->chip_.delay_timer_, 42);
}

TEST_F(InstructionTest, StoreXIntoSoundTimer) {
  this->chip_.V_[0x0] = 42;
  this->chip_.Fx18 (0x0);

  EXPECT_EQ(this->chip_.sound_timer_, 42);
}

TEST_F(InstructionTest, AddXToI) {
  this->chip_.I_ = 21;
  this->chip_.V_[0x0] = 21;
  this->chip_.Fx1E (0x0);

  EXPECT_EQ(this->chip_.I_, 42);
}

TEST_F(InstructionTest, SetIToNumberSprite) {
  for (auto index = 0u; index < 16; index++) {
    this->chip_.V_[0x0] = index;
    this->chip_.Fx29 (0x0);

    EXPECT_EQ(this->chip_.I_, index * 5);
  }
}

TEST_F(InstructionTest, StoreBCD) {
  this->chip_.I_ = AFTER_INSTRUCTION_PC;

  this->chip_.V_[0x0] = 142;
  this->chip_.Fx33 (0x0);

  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 0], 1);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 1], 4);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 2], 2);

  this->chip_.V_[0x0] = 42;
  this->chip_.Fx33 (0x0);

  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 0], 0);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 1], 4);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 2], 2);

  this->chip_.V_[0x0] = 2;
  this->chip_.Fx33 (0x0);

  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 0], 0);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 1], 0);
  EXPECT_EQ(this->chip_.memory_[this->chip_.I_ + 2], 2);
}

TEST_F(InstructionTest, StoreRegsToXToI) {
  this->chip_.I_ = AFTER_INSTRUCTION_PC;

  for (auto index = 0u; index < 16; index++) {
    this->chip_.V_[index] = 42 + index;
  }

  this->chip_.Fx55 (16);

  EXPECT_EQ(this->chip_.I_, AFTER_INSTRUCTION_PC + 17);

  for (auto index = 0u; index < 16; index++) {
    EXPECT_EQ(this->chip_.memory_[this->chip_.I_ - 17 + index], 42 + index);
  }
}

TEST_F(InstructionTest, StoreIToXIntoRegs) {
  this->chip_.I_ = AFTER_INSTRUCTION_PC;

  for (auto index = 0u; index < 16; index++) {
    this->chip_.memory_[this->chip_.I_ + index] = 42 + index;
  }

  this->chip_.Fx65 (16);

  EXPECT_EQ(this->chip_.I_, AFTER_INSTRUCTION_PC + 17);

  for (auto index = 0u; index < 16; index++) {
    EXPECT_EQ(this->chip_.V_[index], 42 + index);
  }
}
