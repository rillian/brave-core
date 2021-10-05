/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <limits>

#include "brave_base/sequence.h"

namespace brave_base {
namespace random {

// An explicitly-seeded pseudo-random sequence generator for farbling.
// This class wraps `base::InsecureRandomGenerator` to provide non-testing
// uses with explicit seeding, ensuring the sequence is always initialized
// with an explicit seed, rather than system entropy.
DeterministicSequence::DeterministicSequence(uint64_t seed) {
  seed_ = seed;
  reset();
}

void DeterministicSequence::reset() {
  // Reset the sequence to the beginning.
  prng.SeedForTesting(seed_);
}

uint64_t DeterministicSequence::next() {
  // Return the next number in the sequence.
  return prng.RandUint64();
}

uint64_t DeterministicSequence::operator()() {
  // Short cut for calling next().
  return next();
}

void DeterministicSequence::discard(uint64_t count) {
  // Advance `count` numbers through the sequence.
  for (uint64_t i = 0; i < count; ++i) {
    next();
  }
}

// Verify properties used by std::algorithm.
static_assert(sizeof(DeterministicSequence::result_type) == sizeof(uint64_t),
              "result_type should be 64 bits");
static_assert(DeterministicSequence::min() == 0,
              "expected an unsigned result with a minimum value of 0");
static_assert(
    DeterministicSequence::max() ==
        std::numeric_limits<DeterministicSequence::result_type>::max(),
    "expected an unsigned result with a the same maximum value as result_type");

}  // namespace random
}  // namespace brave_base
