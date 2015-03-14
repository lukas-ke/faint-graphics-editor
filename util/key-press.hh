// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef FAINT_KEY_PRESS_HH
#define FAINT_KEY_PRESS_HH
#include "text/utf8-string.hh"

namespace faint{

class Key{
  // A keycode without modifiers.
public:
  constexpr explicit Key(int keyCode) : m_key(keyCode){}
  operator int() const;
private:
  int m_key;
};

class Mod{
  // A set of key modifiers (Alt, Ctrl, Shift or None).
private:
  constexpr explicit Mod(int modifiers)
    : m_modifiers(modifiers)
  {}

public:
  // Note: Minimize use of Create - prefer using the predefined
  // constants and combinations with +, e.g. Ctrl+Shift
  static constexpr Mod Create(int modifiers){
    return Mod(modifiers);
  }

  constexpr Mod() : m_modifiers(0){}
  constexpr Mod(const Mod& m) = default;

  bool Alt() const;
  bool Ctrl() const;
  bool Shift() const;
  bool None() const;
  // Returns this modifier if true, otherwise empty
  Mod If(bool) const;

  constexpr Mod operator+(const Mod& other) const{
    return Mod(m_modifiers | other.m_modifiers);
  }

  void operator+=(const Mod&);
  bool operator==(const Mod&) const;
  bool operator<(const Mod&) const;

  // Raw() Should be used only for serialization,
  int Raw() const;
private:
  int m_modifiers;
};

constexpr Mod None(Mod::Create(0x0000));
constexpr Mod Alt(Mod::Create(0x0001));
constexpr Mod Ctrl(Mod::Create(0x0002));
constexpr Mod Shift(Mod::Create(0x0004));

class KeyPress{
  // A keycode and modifiers.
public:
  KeyPress();
  KeyPress(const Mod&, const Key&);
  // Fixme: Char should be derived from modifier and key
  KeyPress(const Mod&, const Key&, const utf8_char&);
  KeyPress(const Key&);
  bool Alt() const;
  const utf8_char& Char() const;
  bool Ctrl() const;
  Key GetKeyCode() const;
  bool HasModifier() const;
  bool Is(const Key&) const; // Ignores modifiers
  bool Is(const Mod&, const Key&) const;
  Mod Modifiers() const;
  utf8_string Name() const;
  bool Shift() const;
  bool operator==(const KeyPress&) const;
  bool operator<(const KeyPress&) const;
private:
  utf8_char m_ch;
  Key m_keyCode;
  Mod m_modifiers;
};

bool affects_alphanumeric_entry(const KeyPress&);
bool affects_numeric_entry(const KeyPress&);
bool command_char(const KeyPress&);
bool numeric(const KeyPress&);

KeyPress operator+(const Mod&, const Key&);

} // namespace

#endif
