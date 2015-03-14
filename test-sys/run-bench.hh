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

#include <algorithm>
#include "test-sys/bench.hh"
#include "test-sys/test-name.hh"

void run_bench(void (*func)(), const std::string& fileName){
  // Test title
  const std::string name = fileName.substr(0, fileName.size() - 4);
  set_test_name(name);
  std::cout << name << ":" << std::endl;

  // Run the bench mark
  func();

  if (BENCH_MEASURES.empty()){
    std::cout << "  No measurements." << std::endl;
    return;
  }

  double minTime = BENCH_MEASURES.front().seconds;
  for (auto& entry : BENCH_MEASURES){
    minTime = std::min(entry.seconds, minTime);
  }

  struct TimingRow{
    std::string name;
    std::string seconds;
    std::string percentage;
    std::string repetitions;
  };

  int nameColWidth = static_cast<int>(std::string("Name").size());
  int timeColWidth = static_cast<int>(std::string("Seconds").size());
  int percentageColWidth = static_cast<int>(std::string("Percentage").size());
  int repetitionsColWidth = static_cast<int>(std::string("Reps").size());

  double totalTime = 0;
  std::vector<TimingRow> rows;
  for (auto& entry : BENCH_MEASURES){
    TimingRow row;
    row.name = entry.name;
    nameColWidth = std::max(nameColWidth, static_cast<int>(row.name.size()));

    totalTime += entry.seconds;
    std::stringstream ss;
    ss << std::setprecision(4) << std::fixed << entry.seconds;
    row.seconds = ss.str();
    timeColWidth = std::max(timeColWidth, static_cast<int>(row.seconds.size()));
    ss.str("");

    if (minTime > 0){
      ss << int((entry.seconds / minTime) * 100 + 0.5);
      row.percentage = ss.str();
    }
    else {
      row.percentage = "N/A";
    }
    percentageColWidth = std::max(percentageColWidth, static_cast<int>(row.percentage.size()));

    ss.str("");
    ss << entry.repetitions;
    row.repetitions = ss.str();
    repetitionsColWidth = std::max(repetitionsColWidth, static_cast<int>(row.repetitions.size()));

    rows.push_back(row);
  }

  std::cout << "  " << std::left << std::setw(nameColWidth + 2) << "Name";
  std::cout << std::left << std::setw(timeColWidth + 2) << "Seconds";
  std::cout << std::left << std::setw(percentageColWidth + 2) << "Percentage";
  std::cout << std::left << std::setw(repetitionsColWidth) << "Reps";
  std::cout << std::endl;
  std::cout << "  " <<
    std::string(static_cast<size_t>(nameColWidth + timeColWidth + percentageColWidth + repetitionsColWidth) + 3 * 2, '-');
  std::cout << std::endl;

  for (auto& row : rows){
    std::cout << "  " << std::left << std::setw(nameColWidth + 2) << row.name;
    std::cout << std::setw(timeColWidth + 2) << row.seconds;
    std::cout << std::setw(percentageColWidth + 2) << row.percentage;
    std::cout << std::setw(repetitionsColWidth) << row.repetitions;
    std::cout << std::endl;
  }
  std::cout << "  Total: " << std::setprecision(4) << totalTime << "s" << std::endl;
  BENCH_MEASURES.clear();
}
