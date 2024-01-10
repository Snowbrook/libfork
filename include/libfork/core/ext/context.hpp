#ifndef D66BBECE_E467_4EB6_B74A_AAA2E7256E02
#define D66BBECE_E467_4EB6_B74A_AAA2E7256E02

// Copyright © Conor Williams <conorwilliams@outlook.com>

// SPDX-License-Identifier: MPL-2.0

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <functional> // for function
#include <utility>    // for move
#include <version>    // for __cpp_lib_move_only_function

#include "libfork/core/ext/deque.hpp"    // for deque, steal_t
#include "libfork/core/ext/handles.hpp"  // for task_handle, submit_handle
#include "libfork/core/ext/list.hpp"     // for intrusive_list, intruded_list
#include "libfork/core/impl/utility.hpp" // for non_null, immovable
#include "libfork/core/macro.hpp"        // for LF_ASSERT

/**
 * @file context.hpp
 *
 * @brief Provides the hierarchy of worker thread contexts.
 */

namespace lf {

// ------------------ Context ------------------- //

inline namespace ext {

class context;        // Semi-User facing, (for submitting tasks).
class worker_context; // API for worker threads.

} // namespace ext

namespace impl {

class full_context;      // Internal API
struct switch_awaitable; // Forward decl for friend.

} // namespace impl

inline namespace ext {

/**
 * @brief A type-erased function object that takes no arguments.
 */
#ifdef __cpp_lib_move_only_function
using nullary_function_t = std::move_only_function<void()>;
#else
using nullary_function_t = std::function<void()>;
#endif

/**
 * @brief Core-visible context for users to query and submit tasks to.
 *
 * This class is technically part of the extension API however it is documented in the core API as a pointer
 * to a context (obtained via the .context() method of a first argument) can be ``co_awaited`` to explicitly
 * transfer control/execution of a coroutine.
 *
 * Each worker thread stores a context object, this is managed by the library. Internally a context manages
 * the work stealing queue and the submission queue. Submissions to the submission queue trigger a
 * user-supplied notification.
 */
class context : impl::immovable<context> {
 public:
  /**
   * @brief Construct a context for a worker thread.
   *
   * Notify is a function that may be called concurrently by other workers to signal to the worker
   * owning this context that a task has been submitted to a private queue.
   */
  explicit context(nullary_function_t notify) noexcept : m_notify(std::move(notify)) { LF_ASSERT(m_notify); }

 private:
  friend class ext::worker_context;
  friend class impl::full_context;
  friend struct impl::switch_awaitable;

  /**
   * @brief Submit pending/suspended tasks to the context.
   *
   * This is for use by the implementer of the scheduler, this will trigger the notification function.
   */
  void submit(intruded_list<submit_handle> jobs) {
    m_submit.push(non_null(jobs));
    m_notify();
  }

  deque<task_handle> m_tasks;             ///< All non-null.
  intrusive_list<submit_handle> m_submit; ///< All non-null.

  nullary_function_t m_notify; ///< The user supplied notification function.
};

/**
 * @brief Context for (extension) schedulers to interact with.
 *
 * Additionally exposes submit and pop functions.
 */
class worker_context : public context {
 public:
  using context::context;

  using context::submit;

  /**
   * @brief Fetch a linked-list of the submitted tasks.
   *
   * If there are no submitted tasks, then returned pointer will be null.
   */
  [[nodiscard]] auto try_pop_all() noexcept -> intruded_list<submit_handle> { return m_submit.try_pop_all(); }

  /**
   * @brief Attempt a steal operation from this contexts task deque.
   */
  [[nodiscard]] auto try_steal() noexcept -> steal_t<task_handle> { return m_tasks.steal(); }
};

} // namespace ext

namespace impl {

/**
 * @brief Context for internal use, contains full-API for push/pop.
 */
class full_context : public worker_context {
 public:
  using worker_context::worker_context;

  /**
   * @brief Add a task to the work queue.
   */
  void push(task_handle task) noexcept { m_tasks.push(non_null(task)); }

  /**
   * @brief Remove a task from the work queue
   */
  [[nodiscard]] auto pop() noexcept -> task_handle {
    return m_tasks.pop([]() -> task_handle {
      return nullptr;
    });
  }

  /**
   * @brief Test if the work queue is empty.
   */
  [[nodiscard]] auto empty() const noexcept -> bool { return m_tasks.empty(); }
};

} // namespace impl

} // namespace lf

#endif /* D66BBECE_E467_4EB6_B74A_AAA2E7256E02 */
