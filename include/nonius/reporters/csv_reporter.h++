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

// CSV raw data reporter

#ifndef NONIUS_REPORTERS_CSV_REPORTER_HPP
#define NONIUS_REPORTERS_CSV_REPORTER_HPP

#include <nonius/reporter.h++>
#include <nonius/configuration.h++>
#include <nonius/sample_analysis.h++>
#include <nonius/execution_plan.h++>
#include <nonius/environment.h++>

#include <boost/chrono.hpp>
#include <boost/variant.hpp>

#include <ratio>
#include <ios>
#include <iostream>
#include <iomanip>
#include <fstream>

#include <string>
#include <limits>
#include <unordered_map>
#include <vector>

namespace nonius {
    struct csv_reporter : reporter {
    private:
        void do_configure(configuration cfg) override {
            n_samples = cfg.samples;
        }

        void do_benchmark_start(std::string const& name) override {
            current = name;
        }

        void do_measurement_complete(std::vector<fp_seconds> const& samples) override {
            data[current] = samples;
        }

        void do_suite_complete() override {
            stream() << std::fixed;
            stream().precision(std::numeric_limits<double>::digits10);
            bool first = true;
            for(auto&& kv : data) {
                if(!first) stream() << ", ";
                stream() << "\"" << kv.first << "\""; // TODO escape
                first = false;
            }
            stream() << "\n";
            for(int i = 0; i < n_samples; ++i) {
                first = true;
                for(auto&& kv : data) {
                    if(!first) stream() << ", ";
                    stream() << kv.second[i].count();
                    first = false;
                }
                stream() << "\n";
            }
        }

    private:
        int n_samples;
        std::string current;
        std::unordered_map<std::string, std::vector<fp_seconds>> data;
    };
} // namespace nonius

#endif // NONIUS_REPORTERS_CSV_REPORTER_HPP
