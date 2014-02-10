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

/** \file

    \brief This application measures the efficiency of sparse coding
 */

#include <glib.h>
#include <rnc>
#include <rnc-lib/rlc.h>
#include <iostream>
#include <time.h>
#include <exception>
#include <stdlib.h>
#include <stdio.h>
#include <mkstr>
#include <auto_arr_ptr>
#include <memory>
#include <sys/time.h>
#include <test.h>

using namespace std;
using namespace rnc;
using namespace rnc::coding;
using namespace rnc::matrix;
using namespace rnc::test;

rnc::random::mt_state rnd_state;

void chkSystem(int retval)
{
        if (retval < 0)
                throw string("error: fork failed");
        else if (retval)
                throw string("error: command execution failed");

}

int getint(string str)
{
        stringstream ss(str);
        int i;
        ss >> i;
        return i;
}

double getdouble(string str)
{
        stringstream ss(str);
        double i;
        ss >> i;
        return i;
}

double tdiff(const struct timeval& t1,
             const struct timeval& t2)
{
        double retval = 0;
        retval = t2.tv_sec - t1.tv_sec;
        if (t1.tv_usec < t2.tv_usec)
        {
                retval += 1-(t1.tv_usec - t2.tv_usec)*1e-6;
                retval -= 1;
        }
        else
        {
                retval += (t2.tv_usec - t1.tv_usec)*1e-6;
        }
        return retval;
}

char* throughput(off_t size,
                 const struct timeval& t1,
                 const struct timeval& t2)
{
        double s = size;
        static char buf[32];
        sprintf(buf, "%.4fMB/s", s/tdiff(t1, t2)/(1<<20));
        return buf;
}

char* timediff(const struct timeval& t1,
                const struct timeval& t2)
{
        static char buf[32];
        sprintf(buf, "%f", tdiff(t1, t2));
        return buf;
}

void p(const BlockList &blocks)
{
        Matrix *C, *D;
        blocks.to_matrices(&C, &D);

        auto_ptr<Matrix> apC(C), apD(D);

        p(*C, *D, cout);
}

int main(int argc, char **argv)
try
{
        fq::init();

        random::init(&rnd_state, time(NULL));

        if (argc < 8)
                throw string(MKStr() << "usage: " << argv[0] <<
                             " <input filename> "
                             "<N : block count> "
                             "<A : sparse coding factor> "
                             "<T : redundancy threshold> "
                             "<R : redundancy target> "
                             "<F : failure probability> "
                             "<id>");

        const string fname = argv[1];
        const int N = getint(argv[2]);
        const double A = getdouble(argv[3]);
        const int T = getint(argv[4]);
        const int R = getint(argv[5]);
        const double F = getdouble(argv[6]);
        const int id = getint(argv[7]);

        printf("%02d MEM file=%s q=%d mode=sim N=%d A=%f T=%d R=%d F=%f\n",
               id, fname.c_str(), fq_size, N, A, T, R, F);

        File infile(fname, N);

        Matrix orig(infile.data(), infile.nrows(), infile.ncols());
        Matrix identity(N, N);
        set_identity(identity);
        BlockList blocks = infile.block_list(identity.rows);

        printf("block count: %lu\n", blocks.count());

        Matrix *C, *D;
        blocks.to_matrices(&C, &D);

        printf("C: %lu x %lu\n", C->nrows, C->ncols);
        printf("D: %lu x %lu\n", D->nrows, D->ncols);
        p(orig);

        p(blocks);

        BlockList sample = blocks.random_sample(15, &rnd_state);
        printf("### Random sample:\n");
        p(sample);
        sample.drop(1);
        printf("### Dropped @[1]:\n");
        p(sample);
        sample.random_drop(0.3, 15, &rnd_state);
        printf("### Random drop:\n");
        p(sample);

        /*
                off_t fsize;
                Matrix m1(N, N);
                auto_arr_ptr<Element> mi_data;

                struct timeval begin_gen, end_gen;
                int sing = 0;

                {
                        Matrix minv(N, N);
                        gettimeofday(&begin_gen, 0);
                        do {
                                ++sing;
                                rand_matr(m1, p, &rnd_state);
                        } while (!invert(m1, minv));
                        gettimeofday(&end_gen, 0);
                }

                {
                        FileMap infile(fname);
                        fsize = infile.size();

                        if (fsize % N)
                                throw string(MKStr()
                                             << "File size (" << fsize
                                             << ") is not dividable by block size ("
                                             << N << ")");

                        mi_data = new Element[fsize / sizeof(Element)];
                        memcpy(mi_data, infile.addr(), fsize);
                }

                const size_t ncols = fsize/N/sizeof(Element);
                Matrix mi(mi_data, N, ncols);
                Matrix mc(N, ncols);

                struct timeval begin, end;
                gettimeofday(&begin, 0);
                pmul(m1, mi, mc);
                gettimeofday(&end, 0);

                printf("matrgen=%s ", timediff(begin_gen, end_gen));
                printf("t=%s ", timediff(begin, end));
                printf("tp=%s\n", throughput(fsize, begin, end));

                {
                        FileMap fm(fmatr, O_SAVE, N*N*sizeof(Element));
                        copy(m1, fm.addr());
                }
                {
                        FileMap fm(fout, O_SAVE, fsize);
                        copy(mc, fm.addr());
                }

                if (sing > 1)
                        printf("# Singular matrices generated: %d\n", sing-1);
        }

        bool singular=false;
        if (mode == "d")
        {
                printf("MEM file=%s mode=d q=%d N=%d CPUs=%d BS=%d ",
                       fname.c_str(), fq_size, N, NCPUS, BLOCK_SIZE);

                off_t fsize;
                auto_arr_ptr<Element> m1_data;
                auto_arr_ptr<Element> mi_data;

                {
                        FileMap matr(fmatr);
                        m1_data = new Element[N*N];
                        memcpy(m1_data, matr.addr(), matr.size());
                }
                {
                        FileMap infile(fout);
                        fsize = infile.size();

                        if (fsize % N)
                                throw string(MKStr()
                                             << "File size (" << fsize
                                             << ") is not dividable by block size ("
                                             << N << ")");

                        mi_data = new Element[fsize / sizeof(Element)];
                        memcpy(mi_data, infile.addr(), fsize);
                }

                const size_t ncols = fsize/N/sizeof(Element);
                Matrix m1(m1_data, N, N);
                Matrix mi(mi_data, N, ncols);
                Matrix minv(N, N);

                struct timeval begin_inv, end_inv;
                gettimeofday(&begin_inv, 0);
                if (!invert(m1, minv))
                {
                        singular=true;
                        goto __break;
                }
                gettimeofday(&end_inv, 0);

                Matrix md(N, ncols);

                struct timeval begin, end;
                gettimeofday(&begin, 0);
                pmul(minv, mi, md);
                gettimeofday(&end, 0);

                printf("matrinv=%s ", timediff(begin_inv, end_inv));
                printf("t=%s ", timediff(begin, end));
                printf("tp=%s\n", throughput(fsize, begin, end));

                {
                        FileMap fm(fdec, O_SAVE, fsize);
                        copy(md, fm.addr());
                }
        }

__break:
        if (singular)
                throw string("Generated matrix was singular.");
        */

        return 0;
}
catch (exception &ex)
{
        cerr << "Error: " << ex.what() << endl;
}
catch (const string &msg)
{
        cerr << "Error: " << msg << endl;
}
