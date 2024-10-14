#include "parallel.hpp"
#include <thread>
#include <vector>

void parallel::for_each(size_t nb_elements,
                        std::function<void(size_t start, size_t end)> functor,
                        bool use_threads) {
  size_t nb_threads_hint = std::thread::hardware_concurrency();
  size_t nb_threads = nb_threads_hint == 0 ? 8 : nb_threads_hint;

  size_t batch_size = nb_elements / nb_threads;
  size_t batch_remainder = nb_elements % nb_threads;

  std::vector<std::jthread> my_threads(nb_threads);

  if (use_threads) {
    // Multithread execution
    for (unsigned i = 0; i < nb_threads; ++i) {
      size_t start = i * batch_size;
      my_threads[i] = std::jthread(functor, start, start + batch_size);
    }
  } else {
    // Single thread execution (for easy debugging)
    for (unsigned i = 0; i < nb_threads; ++i) {
      size_t start = i * batch_size;
      functor(start, start + batch_size);
    }
  }

  // Deform the elements left
  size_t start = nb_threads * batch_size;
  functor(start, start + batch_remainder);

  // jthreads are automatically avaited at the end of the scope
}
