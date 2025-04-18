#include "parallel.hpp"

// adapted from https://stackoverflow.com/a/49188371
void for_each(
    size_t nb_elements,
    std::function<void(size_t start, size_t end)> functor,
    bool use_threads = true
) {
  size_t nb_threads_hint = std::thread::hardware_concurrency();
  size_t thread_count = nb_threads_hint == 0 ? 8 : nb_threads_hint;

  size_t batch_size = nb_elements / thread_count;
  size_t batch_remainder = nb_elements % thread_count;

  if (!use_threads) {
    // Single thread execution (for easy debugging)
    for (size_t i = 0; i < thread_count; ++i) {
      size_t start = i * batch_size;
      functor(start, start + batch_size);
    }
    return;
  }

  std::vector<std::jthread> threads(thread_count);
  if (use_threads) {
    // Multithreaded execution
    for (unsigned i = 0; i < thread_count; ++i) {
      size_t start = i * batch_size;
      threads[i] = std::jthread(functor, start, start + batch_size);
    }
  }

  // Handle the remaining elements
  size_t start = thread_count * batch_size;
  functor(start, start + batch_remainder);

  // jthreads are automatically awaited at the end of the scope
}
