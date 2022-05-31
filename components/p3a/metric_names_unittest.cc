// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

#include <string>

#include "base/logging.h"
#include "brave/components/p3a/metric_names.h"
#include "testing/gtest/include/gtest/gtest.h"

// npm run test -- brave_unit_tests --filter=P3AMetrics*

namespace brave::p3a {

TEST(P3AMetrics, Searchable) {
  // Check that some expected elements test as valid.
  //
  // These examples were chosen as likely to be long-lived;
  // they will need to be updated if any of them are retired.
  EXPECT_TRUE(IsValidMetric("Brave.Core.IsDefault"));
  EXPECT_TRUE(IsValidMetric("Brave.P3A.SentAnswersCount"));
  EXPECT_TRUE(IsValidMetric("Brave.P2A.TotalAdOpportunities"));
  // Verify set membership testing isn't returning true unconditionally.
  EXPECT_FALSE(IsValidMetric("Not.A.Brave.Metric"));
  EXPECT_FALSE(IsValidMetric("antimetric"));
}

TEST(P3AMetrics, Enumerable) {
  const size_t size = kCollectedHistograms.size();
  size_t count = 0;
  std::string last = {};

  // Check that we can loop through the set members.
  DLOG(INFO) << "Set of collected metrics has " << size << " members:";
  for (auto item : kCollectedHistograms) {
    DLOG(INFO) << "  " << item;
    // Each item should be a set member.
    EXPECT_TRUE(kCollectedHistograms.contains(item));
    // Each item should be different from the previous one.
    EXPECT_NE(item, last);
    last = std::string(item);
    // Count the number of items.
    count++;
  }

  // Verify we saw all of the members.
  EXPECT_EQ(count, size);
  EXPECT_EQ(size, kCollectedHistograms.size());
}

}  // namespace brave::p3a
