// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "commands/calibrate-cmd.hh"
#include "commands/command.hh"
#include "geo/calibration.hh"
#include "text/utf8-string.hh"
#include "util/image.hh"
#include "util/optional.hh"

namespace faint{

class CalibrateCommand : public Command{
public:
  CalibrateCommand(const Calibration& c, const Optional<Calibration>& old)
    : Command(CommandType::OBJECT),
      m_calibration(c),
      m_old(old)
  {}

  void Do(CommandContext& ctx) override{
    ctx.GetFrame().SetCalibration(option(m_calibration));
  }

  void Undo(CommandContext& ctx) override{
    ctx.GetFrame().SetCalibration(m_old);
  }

  utf8_string Name() const override{
    return "Calibrate";
  }

private:
  Calibration m_calibration;
  Optional<Calibration> m_old;
};

Command* calibrate_command(const Calibration& calibration,
  const Optional<Calibration>& old)
{
  return new CalibrateCommand(calibration, old);
}

} // namespace
