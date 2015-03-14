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

#include "wx/defs.h"
#include <set>
#include <sstream>
#include "text/char-constants.hh"
#include "util/convenience.hh"
#include "util/key-press.hh"
#include "util-wx/key-codes.hh"

namespace faint{

Key::operator int() const{
  return m_key;
}

static std::set<KeyPress> init_excluded_alpha_numeric_entry(){
  std::set<KeyPress> codes;
  codes.insert(KeyPress(key::F1));
  codes.insert(KeyPress(key::F2));
  codes.insert(KeyPress(key::F3));
  codes.insert(KeyPress(key::F4));
  codes.insert(KeyPress(key::F5));
  codes.insert(KeyPress(key::F6));
  codes.insert(KeyPress(key::F7));
  codes.insert(KeyPress(key::F8));
  codes.insert(KeyPress(key::F9));
  codes.insert(KeyPress(key::F10));
  codes.insert(KeyPress(key::F11));
  codes.insert(KeyPress(key::F12));
  return codes;
}

const std::set<KeyPress>& excluded_alpha_numeric_entry(){
  static const auto codes = init_excluded_alpha_numeric_entry();
  return codes;
}

bool affects_alphanumeric_entry(const KeyPress& key){
  const std::set<KeyPress>& excluded(excluded_alpha_numeric_entry());
  return excluded.find(key) == excluded.end();
}

static bool navigation(const KeyPress& k){
  return k.Is(key::left) ||
    k.Is(key::right) ||
    k.Is(key::up) ||
    k.Is(key::down) ||
    k.Is(key::home) ||
    k.Is(key::end);
}

bool affects_numeric_entry(const KeyPress& key){
  return command_char(key) || numeric(key) || navigation(key);
}

bool command_char(const KeyPress& key){
  return key.GetKeyCode() == key::back || key.GetKeyCode() == key::del;
}

bool numeric(const KeyPress& key){
  return !key.HasModifier() &&
    key::zero <= key.GetKeyCode() && key.GetKeyCode() <= key::nine;
}

static bool alpha_numeric(int k){
  return ((97 <= k && k <= 122) ||
    (65 <= k && k <= 90) ||
    (key::zero <= k && k <= key::nine));
}

static utf8_string name(int k){
  if (alpha_numeric(k)){
    std::string str(1, char(k));
    return utf8_string(str);
  }
  if (k == key::alt){
    return "Alt";
  }
  if (k == key::asterisk){
    return "*";
  }
  if (k == key::back){
    return "Backspace";
  }
  if (k == key::ctrl){
    return "Ctrl";
  }
  if (k == key::del){
    return "Del";
  }
  if (k == key::down){
    return "Down";
  }
  if (k == key::end) {
    return "End";
  }
  if (k == key::enter){
    return "Enter";
  }
  if (k == key::esc){
    return "Esc";
  }
  if (k == key::F1){
    return "F1";
  }
  if (k == key::F2){
    return "F2";
  }
  if (k == key::F3){
    return "F3";
  }
  if (k == key::F4){
    return "F4";
  }
  if (k == key::F5){
    return "F5";
  }
  if (k == key::F6){
    return "F6";
  }
  if (k == key::F7){
    return "F7";
  }
  if (k == key::F8){
    return "F8";
  }
  if (k == key::F9){
    return "F9";
  }
  if (k == key::F10){
    return "F10";
  }
  if (k == key::F11){
    return "F11";
  }
  if (k == key::F12){
    return "F12";
  }
  if (k == key::home){
    return "Home";
  }
  if (k == key::left){
    return "Left";
  }
  if (k == key::minus){
    return "-";
  }
  if (k == key::num_minus){
    return "-";
  }
  if (k == key::num_plus){
    return "+";
  }
  if (k == key::paragraph){
    return "Paragraph";
  }
  if (k == key::pgdn){
    return "PgDn";
  }
  if (k == key::pgup){
    return "PgUp";
  }
  if (k == key::plus){
    return "+";
  }
  if (k == key::right){
    return "Right";
  }
  if (k == key::shift){
    return "Shift";
  }
  if (k == key::space){
    return "Space";
  }
  if (k == key::up){
    return "Up";
  }
  std::stringstream ss;
  ss << "<" << k << ">";
  return utf8_string(ss.str());
}

// Fixme: Duplicates Python (envsetup.py)
static utf8_string modifier_text(const Mod& modifiers){
  if (modifiers.None()){
    return "";
  }

  std::string label = "";
  if (modifiers.Ctrl()){
    label += "Ctrl+";
  }
  if (modifiers.Shift()){
    label += "Shift+";
  }
  if (modifiers.Alt()){
    label += "Alt+";
  }
  return utf8_string(label);
}

static utf8_string as_text(int keycode, const Mod& modifiers){
  return modifier_text(modifiers) + name(keycode);
}

KeyPress::KeyPress()
  : m_ch(replacement_character),
    m_keyCode(0),
    m_modifiers(None)
{}

KeyPress::KeyPress(const Mod& modifiers, const Key& keyCode) :
  m_ch(replacement_character),
  m_keyCode(keyCode),
  m_modifiers(modifiers)
{}

KeyPress::KeyPress(const Mod& modifiers,
  const Key& keyCode,
  const utf8_char& ch)
  : m_ch(ch),
    m_keyCode(keyCode),
    m_modifiers(modifiers)
{}

KeyPress::KeyPress(const Key& keyCode) :
  m_ch(replacement_character),
  m_keyCode(keyCode)
{}

bool KeyPress::Alt() const{
  return m_modifiers.Alt();
}

const utf8_char& KeyPress::Char() const{
  return m_ch;
}

bool KeyPress::Ctrl() const{
  return m_modifiers.Ctrl();
}

Key KeyPress::GetKeyCode() const{
  return m_keyCode;
}

bool KeyPress::HasModifier() const{
  return !m_modifiers.None();
}

bool KeyPress::Is(const Key& keyCode) const{
  return m_keyCode == keyCode;
}

bool KeyPress::Is(const Mod& modifiers, const Key& keyCode) const{
  return m_keyCode == keyCode && m_modifiers == modifiers;
}

Mod KeyPress::Modifiers() const{
  return m_modifiers;
}

utf8_string KeyPress::Name() const{
  return as_text(m_keyCode, m_modifiers);
}

bool KeyPress::Shift() const{
  return m_modifiers.Shift();
}

bool KeyPress::operator==(const KeyPress& other) const{
  return m_keyCode == other.m_keyCode &&
    m_modifiers == other.m_modifiers;
}

bool KeyPress::operator<(const KeyPress& other) const{
  return m_keyCode < other.m_keyCode ||
    (m_keyCode == other.m_keyCode &&
      m_modifiers < other.m_modifiers);
}

void Mod::operator+=(const Mod& other) {
  m_modifiers |= other.m_modifiers;
}

bool Mod::operator<(const Mod& other) const{
  return m_modifiers < other.m_modifiers;
}

bool Mod::operator==(const Mod& other) const{
  return m_modifiers == other.m_modifiers;
}

bool Mod::Alt() const{
  return fl(wxMOD_ALT, m_modifiers);
}

bool Mod::Ctrl() const{
  return fl(wxMOD_CONTROL, m_modifiers);
}

Mod Mod::If(bool cond) const{
  return cond ? *this : Mod();
}

int Mod::Raw() const{
  return m_modifiers;
}

bool Mod::Shift() const{
  return fl(wxMOD_SHIFT, m_modifiers);
}

bool Mod::None() const{
  return m_modifiers == 0;
}

KeyPress operator+(const Mod& mod, const Key& key){
  return KeyPress(mod, key);
}

} // namespace
