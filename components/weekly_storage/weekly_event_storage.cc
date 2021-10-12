/* Copyright 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/weekly_storage/weekly_event_storage.h"

#include <numeric>
#include <type_traits>
#include <utility>

#include "base/thread_annotations.h"
#include "base/time/clock.h"
#include "base/time/default_clock.h"
#include "base/time/time.h"
#include "base/values.h"
#include "components/prefs/pref_service.h"
#include "components/prefs/scoped_user_pref_update.h"

namespace {
constexpr size_t kDaysInWeek = 7;
}

template <typename T>
WeeklyEventStorage<T>::WeeklyEventStorage(PrefService* prefs,
                                          const char* pref_name)
    : prefs_(prefs),
      pref_name_(pref_name),
      clock_(std::make_unique<base::DefaultClock>()) {
  DCHECK(pref_name);
  if (prefs) {
    Load();
  }
}

// Accept an explicit clock so tests can manipulate the passage of time.
template <typename T>
WeeklyEventStorage<T>::WeeklyEventStorage(PrefService* prefs,
                                          const char* pref_name,
                                          std::unique_ptr<base::Clock> clock)
    : prefs_(prefs), pref_name_(pref_name), clock_(std::move(clock)) {
  DCHECK(prefs);
  DCHECK(pref_name);
  Load();
}

template <typename T>
WeeklyEventStorage<T>::~WeeklyEventStorage() = default;

template <typename T>
void WeeklyEventStorage<T>::Add(T value) {
  FilterToWeek();
  // Round the timestamp to the nearest day to make correlation harder.
  base::Time day = clock_->Now().LocalMidnight();
  events_.push_front({day, value});
  Save();
}

template <typename T>
absl::optional<T> WeeklyEventStorage<T>::GetLatest() {
  auto result = absl::nullopt;
  if (HasEvent()) {
    // Assume the front is the most recent event.
    result = absl::optional<T>(events_.front().value);
  }
  return result;
}

template <typename T>
bool WeeklyEventStorage<T>::HasEvent() {
  FilterToWeek();
  return !events_.empty();
}

template <typename T>
void WeeklyEventStorage<T>::FilterToWeek() {
  if (events_.empty()) {
    return;
  }

  // Remove all events older than a week.
  auto cutoff = clock_->Now() - base::TimeDelta::FromDays(kDaysInWeek);
  events_.remove_if([cutoff](Event event) { return event.day <= cutoff; });
}

template <typename T>
void WeeklyEventStorage<T>::Load() {
  DCHECK(events_.empty());
  const base::ListValue* list = prefs_->GetList(pref_name_);
  if (!list) {
    return;
  }
  for (auto& it : list->GetList()) {
    const base::Value* day = it.FindKey("day");
    const base::Value* value = it.FindKey("value");
    if (!day || !value || !day->is_double() || !value->is_double()) {
      continue;
    }
    events_.push_front({base::Time::FromDoubleT(day->GetDouble()),
                        static_cast<uint64_t>(value->GetDouble())});
  }
}

template <typename T>
void WeeklyEventStorage<T>::Save() {
  ListPrefUpdate update(prefs_, pref_name_);
  base::ListValue* list = update.Get();
  // TODO(iefremov): Optimize if needed.
  list->ClearList();
  for (const auto& u : events_) {
    base::DictionaryValue value;
    value.SetKey("day", base::Value(u.day.ToDoubleT()));
    value.SetDoubleKey("value", u.value);
    list->Append(std::move(value));
  }
}
