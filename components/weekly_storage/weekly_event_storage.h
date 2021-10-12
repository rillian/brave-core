/* Copyright 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_WEEKLY_STORAGE_WEEKLY_EVENT_STORAGE_H_
#define BRAVE_COMPONENTS_WEEKLY_STORAGE_WEEKLY_EVENT_STORAGE_H_

#include <list>
#include <memory>

#include "base/time/time.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace base {
class Clock;
}

class PrefService;

// WeeklyStorage variant holding a list of events over the past week.
//
// Mostly used by various P3A recorders to report whether an event happened
// during the measurement period.
//
// New event values are recorded by calling `Add()` and are forgotten
// after approximately a week.
//
// Parametarized over an `enum class` to get some type checking for the caller.
// Requires |pref_name| to be already registered.
template <typename T> class WeeklyEventStorage {
 public:
  WeeklyEventStorage(PrefService* prefs, const char* pref_name);

  // For tests.
  WeeklyEventStorage(PrefService* user_prefs,
                     const char* pref_name,
                     std::unique_ptr<base::Clock> clock);
  ~WeeklyEventStorage();

  WeeklyEventStorage(const WeeklyEventStorage&) = delete;
  WeeklyEventStorage& operator=(const WeeklyEventStorage&) = delete;

  void Add(T value);
  absl::optional<T> GetLatest();
  bool HasEvent();

 private:
  struct Event {
    base::Time day;
    T value = T(0);
  };
  void FilterToWeek();
  void Load();
  void Save();

  PrefService* prefs_ = nullptr;
  const char* pref_name_ = nullptr;
  std::unique_ptr<base::Clock> clock_;

  std::list<Event> events_;
};

#endif  // BRAVE_COMPONENTS_WEEKLY_STORAGE_WEEKLY_EVENT_STORAGE_H_
