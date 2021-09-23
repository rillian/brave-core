/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_CHROMIUM_SRC_BASE_RAND_UTIL_H_
#define BRAVE_CHROMIUM_SRC_BASE_RAND_UTIL_H_

namespace brave_base {
namespace random {

// An explicitly-seeded pseudo-random sequence generator for farbling.
class DeterministicSequence;

}  // namespace random
}  // namespace brave_base

// Insert a `friend` declaration in `base::InsecureRandomGenerator`
// so the class can access private members.
#define SeedForTesting Unused() const {}\
  friend class brave_base::random::DeterministicSequence;\
  void SeedForTesting

#include "../../../base/rand_util.h"

#undef SeedForTesting

#endif // BRAVE_CHROMIUM_SRC_BASE_RAND_UTIL_H_
