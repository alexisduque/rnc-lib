/* -*- mode: c++; coding: utf-8-unix -*-
 *
 * Copyright 2013 MTA SZTAKI
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

/**
   \file
   \brief Test finite field operations.
*/

#include <test.h>
#include <rnc>
#include <iostream>
#include <list>

using namespace std;
using namespace rnc::test;
using namespace rnc::fq;

fq_t getrand_notnull()
{
        fq_t a;
        do { a = random_element(); } while (a==0);
        return a;
}

template <bool (*pt)(ostream*buffer)>
class FQ_TestCase : public TestCase
{
public:
        FQ_TestCase(const string &tname, size_t n)
                : TestCase(string("fq::") + tname, n) {}
        bool performTest(ostream *buffer) const
        {
                return pt(buffer);
        }
};

#define new_TC(a, n) new FQ_TestCase<a>(#a, n)

#define PRINT(a)                     \
        if (buffer) {                \
                (*buffer) << #a "="; \
                p(a, *buffer);       \
        }
#define PRINT2(a, b)                            \
        if (buffer) {                           \
                (*buffer) << #a "=";            \
                p(a, *buffer);                  \
                (*buffer) << ", " #b "=";       \
                p(b, *buffer);                  \
        }
#define PRINT3(a, b, c)                           \
        if (buffer) {                           \
                (*buffer) << #a "=";            \
                p(a, *buffer);                  \
                (*buffer) << ", " #b "=";       \
                p(b, *buffer);                  \
                (*buffer) << ", " #c "=";       \
                p(c, *buffer);                  \
        }

bool inv_1(ostream *buffer)
{
        PRINT(log_table[1]);
        return inv(1) == 1;
}
bool inv_2(ostream *buffer)
{
        fq_t a = getrand_notnull();
        PRINT(a);
        return 1 == mul(a, inv(a));
}
bool invert_1(ostream *buffer)
{
        fq_t a = getrand_notnull();
        fq_t t = a;
        invert(t);
        PRINT(a);
        return inv(a) == t;
}

bool mul_1(ostream *buffer)
{
        fq_t a = random_element();
        PRINT(a);
        return a == mul(1, a);
}

bool mul_2(ostream *buffer)
{
        fq_t a = getrand_notnull();
        fq_t b = getrand_notnull();
        PRINT2(a, b);
        return mul(a,b) == mul(a, b);
}

bool mul_3(ostream *buffer)
{
        fq_t a = getrand_notnull();
        fq_t b = getrand_notnull();
        fq_t c = getrand_notnull();
        PRINT3(a, b, c);
        return mul(a, mul(b, c)) == mul(mul(a, b), c);
}

bool addto_1(ostream *buffer)
{
        fq_t a = random_element();
        fq_t b = random_element();
        fq_t t = a;
        PRINT2(a, b);
        addto(t, b);
        return t == add(a,b);
}

bool addto_mul_1(ostream *buffer)
{
        fq_t d = random_element();
        fq_t a = random_element();
        fq_t b = random_element();
        PRINT3(d, a, b);
        fq_t t = d;
        addto_mul(d, a, b);
        return t == add(d, mul(a, b));
}

bool div_1(ostream *buffer)
{
        fq_t a = random_element();
        fq_t b = getrand_notnull();
        PRINT2(a, b);
        return a == mul(b, div(a,b));
}

bool divby_1(ostream *buffer)
{
        fq_t a = random_element();
        fq_t b = getrand_notnull();
        PRINT2(a, b);
        fq_t t = a;
        divby(t, b);
        return t == div(a,b);
}

int main(int, char **)
{
        init_random();
        cout << "Seed=" << get_seed() << endl;

        init();

        cout << "Q=" << fq_size << endl;

        typedef list<TestCase*> case_list;
        case_list cases;
        cases.push_back(new_TC(inv_1, 1));
        cases.push_back(new_TC(inv_2, 5));
        cases.push_back(new_TC(invert_1, 5));
        cases.push_back(new_TC(mul_1, 5));
        cases.push_back(new_TC(mul_2, 5));
        cases.push_back(new_TC(mul_3, 5));
        cases.push_back(new_TC(addto_1, 5));
        cases.push_back(new_TC(addto_mul_1, 5));
        cases.push_back(new_TC(div_1, 5));
        cases.push_back(new_TC(divby_1, 5));

        int failed = 0;
        for (case_list::const_iterator i = cases.begin();
             i!=cases.end(); ++i)
        {
                failed += (*i)->execute(cout);
        }

        return failed > 0;
}
