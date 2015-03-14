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

#ifndef FAINT_BENCH_HH
#define FAINT_BENCH_HH
#include <chrono>
#include <vector>
#include <iostream>

struct Measure{
  Measure(const std::string& name, double seconds, int repetitions)
    : name(name), seconds(seconds), repetitions(repetitions)
  {}
  std::string name;
  double seconds;
  int repetitions;
};

extern std::vector<Measure> BENCH_MEASURES;

template<void(*func)()>
void timed(const std::string& name, int repetitions){
  auto start = std::chrono::system_clock::now();
  for (int i = 0; i != repetitions; i++){
    func();
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = end - start;
  BENCH_MEASURES.emplace_back(Measure(name, seconds.count(), repetitions));
}

template<typename FUNC>
void timed(const std::string& name, int repetitions, FUNC func){
  auto start = std::chrono::system_clock::now();
  for (int i = 0; i != repetitions; i++){
    func();
  }
  auto end = std::chrono::system_clock::now();
  std::chrono::duration<double> seconds = end - start;
  BENCH_MEASURES.emplace_back(Measure(name, seconds.count(), repetitions));
}

#define TIMED(FUNC, repetitions)timed<FUNC>(#FUNC, repetitions);

#endif
