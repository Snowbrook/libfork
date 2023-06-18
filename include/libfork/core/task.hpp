#ifndef C9921D3E_28E4_4577_BB9C_E7CA55766E92
#define C9921D3E_28E4_4577_BB9C_E7CA55766E92

// Copyright © Conor Williams <conorwilliams@outlook.com>

// SPDX-License-Identifier: MPL-2.0

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include "libfork/macro.hpp"

/**
 * @file task.hpp
 *
 * @brief The ``lf::task`` class.
 */

namespace lf {

/**
 * @brief The return type for libfork's async functions/coroutines.
 */
template <typename T = void>
  requires(!std::is_reference_v<T> && !std::is_const_v<T>)
class task {
public:
  using value_type = T; ///< The type of the value returned by the coroutine (cannot be a reference, use ``std::reference_wrapper``).

  explicit constexpr task(void *handle) noexcept : handle{handle} {
    LIBFORK_ASSERT(handle != nullptr);
  }

  void *handle = nullptr; ///< The handle to the coroutine.
};

} // namespace lf

#endif /* C9921D3E_28E4_4577_BB9C_E7CA55766E92 */
