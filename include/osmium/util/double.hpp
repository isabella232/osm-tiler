#ifndef OSMIUM_UTIL_DOUBLE_HPP
#define OSMIUM_UTIL_DOUBLE_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2016 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <iterator>
#include <string>

namespace osmium {

    namespace util {

        constexpr int max_double_length = 20; // should fit any double

        /**
         * Write double to iterator, removing superfluous '0' characters at
         * the end. The decimal dot will also be removed if necessary.
         *
         * @tparam T iterator type
         * @param iterator output iterator
         * @param value the value that should be written
         * @param precision max number of digits after the decimal point (must be <= 17)
         */
        template <typename T>
        inline T double2string(T iterator, double value, int precision) {
            assert(precision <= 17);

            char buffer[max_double_length];

#ifndef _MSC_VER
            int len = snprintf(buffer, max_double_length, "%.*f", precision, value);
#else
            int len = _snprintf(buffer, max_double_length, "%.*f", precision, value);
#endif
            assert(len > 0 && len < max_double_length);

            while (buffer[len-1] == '0') {
                --len;
            }
            if (buffer[len-1] == '.') {
                --len;
            }

            return std::copy_n(buffer, len, iterator);
        }

        /**
         * Write double to string, removing superfluous '0' characters at
         * the end. The decimal dot will also be removed if necessary.
         *
         * @param out string
         * @param value the value that should be written
         * @param precision max number of digits after the decimal point
         */
        inline void double2string(std::string& out, double value, int precision) {
            double2string(std::back_inserter(out), value, precision);
        }

    } // namespace util

} // namespace osmium

#endif // OSMIUM_UTIL_DOUBLE_HPP
