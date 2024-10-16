// adapted from https://stackoverflow.com/a/49188371

#include <functional>
#include <ranges>
#include <thread>
#include <vector>

namespace parallel {

/// @param nb_elements : size of your for loop
/// @param functor(start, end) :
/// your function processing a sub chunk of the for loop.
/// "start" is the first index to process (included) until the index "end"
/// (excluded)
/// @code
///     for(int i = start; i < end; ++i)
///         computation(i);
/// @endcode
/// @param use_threads : enable / disable threads (for debugging purposes).
///
/// @safety Each thread only recieves it's own range of input values, but it's
/// up to the user provided function to not access elements outside of that
/// range, or to not invalidate any references or pointers, etc.
void for_each(size_t nb_elements,
              std::function<void(size_t start, size_t end)> functor,
              bool use_threads = true);

/// @param span : a span containing target elements
/// @param functor(index, item) :
/// your function processing a single item from the span.
/// @param use_threads : enable / disable threads (for debugging purposes).
///
/// This doesn't spawn as many threads as there are items in the span. It spawns
/// just enough threads to to fully utilize the CPU and distributes the work
/// among them.
///
/// @safety Each thread only recieves it's own range of input values, but it's
/// up to the user provided function to not access elements outside of that
/// range, or to not invalidate any references or pointers, etc.
template <typename T>
void for_each(const std::span<T> span,
              std::function<void(size_t index, T &value)> functor,
              bool use_threads = true) {
  size_t thread_count_hint = std::thread::hardware_concurrency();
  size_t thread_count = thread_count_hint == 0 ? 8 : thread_count_hint;

  size_t batch_size = span.size() / thread_count;

  if (!use_threads) {
    // Single thread execution (for easy debugging)
    for (auto i = 0uz; i < span.size(); ++i)
      functor(i, span[i]);

    return;
  }

  // Multithread execution
  auto threads = std::views::iota(0uz, thread_count) |
                 std::views::transform([&](auto i) {
                   return std::jthread(
                       [&span, &functor, start = i * batch_size, batch_size]() {
                         for (auto i = start; i < start + batch_size; ++i) {
                           functor(i, span[i]);
                         }
                       });
                 }) |
                 std::ranges::to<std::vector>();

  // Handle the remaining elements
  size_t start = thread_count * batch_size;
  for (T &item : span.subspan(start))
    functor(start++, item);

  // jthreads are automatically avaited at the end of the scope
}

} // namespace parallel
