#ifndef C8EE9A0A_3B9F_4FFE_8FF5_910645E0C7CC
#define C8EE9A0A_3B9F_4FFE_8FF5_910645E0C7CC

// Copyright © Conor Williams <conorwilliams@outlook.com>

// SPDX-License-Identifier: MPL-2.0

// Self Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "libfork/core/ext/context.hpp"  // for nullary_function_t, worker_...
#include "libfork/core/ext/handles.hpp"  // for submit_handle
#include "libfork/core/ext/resume.hpp"   // for resume
#include "libfork/core/ext/tls.hpp"      // for worker_init, finalize
#include "libfork/core/impl/utility.hpp" // for immovable

/**
 * @file unit_pool.hpp
 *
 * @brief A scheduler that runs all tasks inline on the current thread.
 */

namespace lf {

/**
 * @brief A scheduler that runs all tasks inline on the current thread.
 *
 * This is useful for testing/debugging/benchmarking.
 */
class unit_pool : impl::immovable<unit_pool> {
 public:
  /**
   * @brief Run a job inline.
   */
  static void schedule(submit_handle job) { resume(job); }

  ~unit_pool() noexcept { lf::finalize(m_context); }

 private:
  lf::worker_context *m_context = lf::worker_init(lf::nullary_function_t{[]() {}});
};

} // namespace lf

#endif /* C8EE9A0A_3B9F_4FFE_8FF5_910645E0C7CC */
