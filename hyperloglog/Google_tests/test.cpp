#include <gtest/gtest.h>

#include "hyperloglog.hpp"

#include <chrono>
#include <iostream>
#include <set>
#include <vector>

#include <random>

class random_generator_iterator
{
    // this class behaves like an iterator
    // you can increment it and dereference it
    // basically we can generate the stream at the same time as we process it
    std::uniform_real_distribution<double> *dist = nullptr;
    std::mt19937 *gen = nullptr;
    size_t counter = 0;

public:
    random_generator_iterator(std::uniform_real_distribution<double> &dist,
                              std::mt19937 &gen)
        : dist(&dist), gen(&gen)
    {}

    // only used for end iterators. should not dereference
    random_generator_iterator(size_t end_idx) : counter(end_idx)
    {}

    using pointer = void;
    using reference = double;

    double operator*()
    { return (*dist)(*gen); }

    random_generator_iterator &operator++()
    {
        counter++;
        return *this;
    }

    random_generator_iterator operator++(int)
    {
        // to be honest this is broken, but I don't care
        // the iterator has no memory so dereferencing the old value results in
        // a new random number and not the same as the original
        // You mustn't use it like
        //  it1 = it2++;
        //  *it1;
        random_generator_iterator it(*this);
        counter++;
        return it;
    }

    bool operator==(const random_generator_iterator &it) const
    {
        return counter == it.counter;
    }

    bool operator!=(const random_generator_iterator &it) const
    {
        return counter != it.counter;
    }
};

template<typename ForwardIterator>
size_t reference(ForwardIterator begin, ForwardIterator end)
{
    std::set<typename std::iterator_traits<ForwardIterator>::value_type> checker;
    for (; begin != end; ++begin) {
        checker.insert(*begin);
    }
    return checker.size();
}


TEST(HyperLogLog, OneM_element_no_duplicates)
{
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist;
    constexpr int log_stream_size = 20;
    std::vector<double> stream(
        1ul << log_stream_size);
    for (size_t i = 0; i < 1ul << log_stream_size; ++i) {
        double rand = dist(gen);
        stream[i] = rand;
    }
    size_t res = hyperloglog(std::begin(stream), std::end(stream));
    ASSERT_EQ(true, 1048576 * 1.02 > res && 1048576 * 0.98 < res);
}

TEST(HyperLogLog, TwoM_element_half_of_which_is_duplicate)
{
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist;
    constexpr int log_stream_size = 20;
    std::vector<double> stream(
        1ul << (log_stream_size + 1));
    for (size_t i = 0; i < 1ul << log_stream_size; ++i) {
        double rand = dist(gen);
        stream[i] = rand;
        stream[i + (1ul << log_stream_size)] = rand;
    }
    size_t res = hyperloglog(std::begin(stream), std::end(stream));
    ASSERT_EQ(true, 1048576 * 1.02 > res && 1048576 * 0.98 < res);
}

TEST(HyperLogLog, Measure_big_test)
{
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dist;
    constexpr size_t log_stream_size = 27;
    constexpr size_t stream_size = 1ul << log_stream_size;
    auto start = std::chrono::high_resolution_clock::now();
    size_t res = hyperloglog(random_generator_iterator(dist, gen),
                      random_generator_iterator(stream_size));
    ASSERT_EQ(true, 134217728 * 1.02 > res && 134217728 * 0.98 < res);
}