/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BRAVE_BASE_SEQUENCE_H_
#define BRAVE_VENDOR_BRAVE_BASE_SEQUENCE_H_

#include <cstdint>

#include "base/rand_util.h"

namespace brave_base {
namespace random {

// An explicitly-seeded pseudo-random sequence generator for farbling.
class DeterministicSequence {
 public:
  // Require an explict seed in the constructor so we can produce
  // a pseudorandom sequence for repeatable perturbation of data.
  //
  // This class makes no calls to obtain actual randomness,
  // e.g. from system entropy, and should never be used where
  // unpredictability is important.
  explicit DeterministicSequence(uint64_t seed);

  // Reset the sequence to the beginning.
  void reset();

  // Return the next element in the sequence.
  uint64_t operator()();
  uint64_t next();

  // Interfaces required for use like a std::random generator.
  typedef uint64_t result_type;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() { return UINT64_MAX; }
  void discard(uint64_t count);

 private:
  // Wrap the generator to block entropy seeding and allow
  // construction for our semantic use case.
  base::InsecureRandomGenerator prng;
  uint64_t seed_;
};

}  // namespace random
}  // namespace brave_base

#endif  // BRAVE_VENDOR_BRAVE_BASE_SEQUENCE_H_
