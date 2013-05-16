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

#ifndef MATRIX_H
#define MATRIX_H

#include <rnc-lib/fq.h>

namespace rnc
{
/** \brief Matrix operations over \f$\mathbb{F}_q\f$.
 */
namespace matrix
{
        using namespace fq;

        /** \brief Number of threads to use.

            Matrix multiplication will use this many threads.

            The matrix will be partitioned; each partition will be processed by
            a glib thread pool maintaining NCPUS thread.

            If NCPUS is 1, the thread pool is not created.
         */
        extern int NCPUS;
        /** \brief Block size for matrix multiplication

            Blocked matrix multiplication will be performed using square blocks
            of size BLOCK_SIZE.

            If BLOCK_SIZE is 1, non-blocked multiplication is performed.
         */
        extern int BLOCK_SIZE;

        /// \addtogroup matr Matrix functions
        /// @{

        /// \addtogroup matr_output Output functions
        /// @{

        /// Print a finite field element
        void p(const fq_t v);
        /// Print a matrix of size \c rows x \c cols
        void p(const fq_t *m, const int rows, const int cols);
        /// Print two matrices of the same size side-by-side
        void p(const fq_t *m1, const fq_t *m2, const int rows, const int cols);

        /// @}

        /// \addtogroup matr_ops Matrix operations
        /// @{

        /** \brief Initialize a matrix to identity. */
        void set_identity(fq_t *m, const int rows, const int cols) throw();
        /** \brief Copy a matrix
            @param m Matrix to be copied
            @param md Destination address
         */
        void copy(const fq_t *m, fq_t *md, const int rows, const int cols) throw();
        /** \brief Invert a matrix

            @param m_in Matrix to be inverted
            @param res Result address

            \test A = mul(A, mul(A, invert(A))) | \f$\exists A^{-1}\f$
        */
        bool invert(const fq_t *m_in, fq_t *res,
                    const int rows, const int cols) throw ();

        // (rows1 x cols1) * (cols1 x cols2) = (rows1 x cols2)
        /** \brief Matrix multiplication: \f$md:=m1*m2\f$

            Multiplies \c m1 by \c m2, stores the result in \c md.

            @param m1 Left-hand side matrix of size \c rows1 x \c cols1
            @param m2 Right-hand side matrix of size \c cols1 x \c cols2
            @param md Result address capable of storing a matrix of size \c
            rows1 x \c cols2
            @param rows1 Number of rows of \c m1; the result will have this many rows
            @param cols1 Number of columns of \c m1. The right-hand side matrix
            must have this many \c rows.
            @param cols2 Number of columns of \c m2; the result will have this
            many columns too.

            \remark Matrices multiplied must be compatible. That is, if their
            sizes are \c rows1 x \c cols1 and \c rows2 x \c cols2, then \c cols1
            and \c rows2 must be equal. Therefore, this function only needs
            three size parameters about the matrices.

            \remark \c md must point to an address capable of storing \c rows1 x
            \c cols2 elements.
         */
        void mul(const fq_t *m1, const fq_t *m2, fq_t *md,
                 const int rows1, const int cols1,int const cols2);
        /** \brief Parallelized version of #mul.

            If NCPUS is 1, this function will simply call #mul.

            If NCPUS is greater than 1, this function will spawn NCPUS threads
            to perform the multiplication.
         */
        void pmul(const fq_t *m1, const fq_t *m2, fq_t *md,
                 const int rows1, const int cols1,int const cols2);

        /** \brief Generates a random matrix.

            @param m Result address, capable of storing a matrix of size \c rows
            x \c cols
            @param rows Number of rows
            @param cols Number of columns
         */
        void rand_matr(fq_t *m, const int rows, const int cols);

        /// @} @}
}
}

#endif //MATRIX_H
