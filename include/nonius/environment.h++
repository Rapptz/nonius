// Nonius - C++ benchmarking tool
//
// Written in 2013-2014 by Martinho Fernandes <martinho.fernandes@gmail.com>
//
// To the extent possible under law, the author(s) have dedicated all copyright and related
// and neighboring rights to this software to the public domain worldwide. This software is
// distributed without any warranty.
//
// You should have received a copy of the CC0 Public Domain Dedication along with this software.
// If not, see <http://creativecommons.org/publicdomain/zero/1.0/>

// Environment module

#ifndef NONIUS_ENVIRONMENT_HPP
#define NONIUS_ENVIRONMENT_HPP

#include <nonius/clock.h++>
#include <nonius/measurement.h++>
#include <nonius/analysis.h++>
#include <nonius/detail/duration.h++>
#include <nonius/detail/transform_if.h++>

#include <wheels/op/minus.h++>

#include <chrono>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <vector>
#include <cmath>

namespace nonius {
    namespace detail {
        template <typename Clock = default_clock>
        std::tuple<std::vector<Duration<Clock>>, int> resolution(int k) {
            std::vector<TimePoint<Clock>> times;
            times.reserve(k+1);
            std::generate_n(std::back_inserter(times), k+1, &Clock::now);

            std::vector<Duration<Clock>> deltas;
            deltas.reserve(k);
            detail::transform_if(std::next(times.begin()), times.end(), times.begin(),
                              std::back_inserter(deltas),
                              wheels::op::minus{},
                              [](Duration<Clock> d) { return d > Duration<Clock>::zero(); });

            return std::make_tuple(std::move(deltas), times.size());
        }
    } // namespace detail
    template <typename Clock = default_clock>
    void warmup(int& seed) {
        run_for_at_least<Clock>(Duration<Clock>(std::chrono::milliseconds(100)), seed, detail::resolution<Clock>);
    }
    template <typename Clock = default_clock>
    FloatDuration<Clock> estimate_clock_resolution(int seed) {
        auto r = run_for_at_least<Clock>(Duration<Clock>(std::chrono::milliseconds(500)), seed, detail::resolution<Clock>).result;
        return analyse_mean(std::get<0>(r).begin(), std::get<0>(r).end(), std::get<1>(r));
    }
    template <typename Clock = default_clock>
    FloatDuration<Clock> estimate_clock_cost(FloatDuration<Clock> resolution) {
        auto time_limit = std::min(resolution * 100000, FloatDuration<Clock>(std::chrono::seconds(1)));
        auto time_clock = [](int k) {
            return time<Clock>([k]() -> int {
                for(int i = 0; i < k; ++i) {
                    volatile auto ignored = Clock::now();
                    (void)ignored;
                }
                return 0;
            }).elapsed;
        };
        time_clock(1);
        int iters = 10000;
        auto elapsed = run_for_at_least<Clock>(Duration<Clock>(std::chrono::milliseconds(10)), iters, time_clock).elapsed;
        std::vector<FloatDuration<Clock>> times;
        int n = std::ceil(time_limit / elapsed);
        times.reserve(n);
        std::generate_n(std::back_inserter(times), n, [time_clock, iters]{ return time_clock(iters) / iters; });
        return analyse_mean(times.begin(), times.end(), times.size());
    }
} // namespace nonius

#endif // NONIUS_ENVIRONMENT_HPP

