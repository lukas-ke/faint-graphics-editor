// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_TOOL_SETTING_CTRL_HH
#define FAINT_TOOL_SETTING_CTRL_HH
#include "wx/panel.h"
#include "wx/event.h"
#include "util/settings.hh"

namespace faint{

class ToolSettingCtrl : public wxPanel{
public:
  ToolSettingCtrl(wxWindow* parent);
  virtual void SendChangeEvent() = 0;
  virtual bool UpdateControl(const Settings&) = 0;
};

class IntSettingCtrl : public ToolSettingCtrl{
public:
  IntSettingCtrl(wxWindow* parent, const IntSetting&);
  virtual IntSetting GetSetting() const;
  virtual int GetValue() const = 0;
  virtual void SetValue(int) = 0;
  virtual bool UpdateControl(const Settings&) override;
  virtual void SendChangeEvent() override;
private:
  IntSetting m_setting;
};

class FloatSettingControl : public ToolSettingCtrl{
public:
  FloatSettingControl(wxWindow* parent, const FloatSetting&);
  virtual FloatSetting GetSetting() const;
  virtual coord GetValue() const = 0;
  virtual void SetValue(coord) = 0;
  virtual bool UpdateControl(const Settings&) override;
  virtual void SendChangeEvent() override;
private:
  FloatSetting m_setting;
};

class BoolSettingControl : public ToolSettingCtrl{
public:
  BoolSettingControl(wxWindow* parent, const BoolSetting&);
  virtual BoolSetting GetSetting() const;
  virtual bool GetValue() const = 0;
  virtual void SetValue(bool) = 0;
  virtual bool UpdateControl(const Settings&) override;
  virtual void SendChangeEvent() override;
private:
  BoolSetting m_setting;
};

class StringSettingControl : public ToolSettingCtrl{
public:
  StringSettingControl(wxWindow* parent, const StringSetting&);
  virtual StringSetting GetSetting() const;
  virtual utf8_string GetValue() const = 0;
  virtual void SetValue(const utf8_string&) = 0;
  virtual bool UpdateControl(const Settings&) override;
  virtual void SendChangeEvent() override;
private:
  StringSetting m_setting;
};

} // namespace

#endif
