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

// Analysis module

#ifndef NONIUS_ANALYSIS_HPP
#define NONIUS_ANALYSIS_HPP

#include <nonius/clock.h++>
#include <nonius/detail/duration.h++>

#include <algorithm>
#include <functional>
#include <iterator>
#include <utility>

namespace nonius {
    struct outliers {
        int samples_seen = 0;
        int low_severe = 0;     // more than 3 times IQR below Q1
        int low_mild = 0;       // 1.5 to 3 times IQR below Q1
        int high_mild = 0;      // 1.5 to 3 times IQR above Q3
        int high_severe = 0;    // more than 3 times IQR above Q3
    };

    namespace detail {
        template <typename Iterator>
        typename std::iterator_traits<Iterator>::value_type weighted_average(int k, int q, Iterator first, Iterator last) {
            int count = last - first;
            double idx = (count - 1) * k /(double) q;
            int j = (int)idx;
            double g = idx - j;
            std::nth_element(first, first+j, last);
            auto xj = first[j];
            if(g == 0) return xj;

            auto xj1 = *std::min_element(first+(j+1), last);
            return xj + g * (xj1 - xj);
        }
    } // namespace detail

    template <typename Iterator>
    outliers classify_outliers(Iterator first, Iterator last) {
        auto q1 = detail::weighted_average(1, 4, first, last);
        auto q3 = detail::weighted_average(3, 4, first, last);
        auto iqr = q3 - q1;
        auto los = q1 - (iqr * 3.0);
        auto lom = q1 - (iqr * 1.5);
        auto him = q3 + (iqr * 1.5);
        auto his = q3 + (iqr * 3.0);

        outliers o;
        for(; first != last; ++first) {
            auto&& t = *first;
            if(t < los) ++o.low_severe;
            else if(t < lom) ++o.low_mild;
            else if(t > his) ++o.high_severe;
            else if(t > him) ++o.high_mild;
            ++o.samples_seen;
        }
        return o;
    }

    template <typename Clock, typename Iterator>
    FloatDuration<Clock> analyse_mean(Iterator first, Iterator last) {
        int count = last - first;
        auto sum = std::accumulate(first, last, FloatDuration<Clock>::zero());
        return sum /(double) count;
    }
} // namespace nonius

#endif // NONIUS_ANALYSIS_HPP

