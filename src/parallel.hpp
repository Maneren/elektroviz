// from https://stackoverflow.com/a/49188371

#include <functional>

namespace parallel {

/// @param[in] nb_elements : size of your for loop
/// @param[in] functor(start, end) :
/// your function processing a sub chunk of the for loop.
/// "start" is the first index to process (included) until the index "end"
/// (excluded)
/// @code
///     for(int i = start; i < end; ++i)
///         computation(i);
/// @endcode
/// @param use_threads : enable / disable threads.
///
///
void for_each(unsigned nb_elements,
              std::function<void(int start, int end)> functor,
              bool use_threads = true);

} // namespace parallel
