/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

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

uint64_t DeterministicSequence::next() {
  return prng.RandUint64();
}

uint64_t DeterministicSequence::operator()() {
  return next();
}

void DeterministicSequence::reset() {
  // Reset the sequence to the beginning.
  prng.SeedForTesting(seed_);
}

}  // namespace random
}  // namespace brave_base
