#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <mutex>
#include <thread>
#include <vector>

using Int = std::uint32_t;
using Thread = std::thread;
template <typename T> using Vector = std::vector<T>;

std::mutex mut;

struct RangeSpecifier {
  const Int start = 0;
  const Int end;
  const Int step = 1;
};

Vector<Int> make_range(const RangeSpecifier spec) {
  auto ret = Vector<Int>();
  for (auto i = spec.start; i < spec.end; i += spec.step) {
    ret.push_back(i);
  }
  return ret;
};

template <typename T> void print_vector(const Vector<T> &v) {
  const auto lock = std::lock_guard<std::mutex>(mut);
  std::cout << "[";
  for (const auto elem : v) {
    std::cout << " " << elem;
  }
  std::cout << " ]" << std::endl;
};

bool is_prime(const Int x) {
  auto sq = std::floor(std::sqrt(x));
  for (auto i = 2; i <= sq; i++) {
    if (x % i == 0)
      return false;
  }
  return true;
}

Vector<Int> &worker(Vector<Int> &vec) {
  auto it = std::remove_if(vec.begin(), vec.end(),
                           [](auto x) { return !is_prime(x); });
  vec.erase(it, vec.end());
  return vec;
}

constexpr auto N_THREADS = 4;
constexpr auto LIMIT = 30000000;

int main() {
  Thread threads[N_THREADS];
  Vector<Int> vectors[N_THREADS];

  auto start = std::chrono::steady_clock::now();
  auto vec = make_range(RangeSpecifier{.start = 1, .end = LIMIT});
  auto it = std::remove_if(vec.begin(), vec.end(),
                           [](auto x) { return !is_prime(x); });
  vec.erase(it, vec.end());

  auto end = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "Single-threaded: " << duration << "ms elapsed" << std::endl;

  start = std::chrono::steady_clock::now();

  for (Int i = 0; i < N_THREADS; i++) {
    vectors[i] = make_range(
        RangeSpecifier{.start = 1 + i, .end = LIMIT, .step = N_THREADS});
    threads[i] = Thread(worker, std::ref(vectors[i]));
  }
  for (auto &t : threads) {
    t.join();
  }
  auto result = Vector<Int>();
  for (const auto &v : vectors) {
    result.insert(result.end(), v.begin(), v.end());
  };
  end = std::chrono::steady_clock::now();
  duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
                 .count();
  std::cout << "Multithreaded: " << duration << "ms elapsed" << std::endl;
  return 0;
}
