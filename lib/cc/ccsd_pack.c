/*
 *
 */

#include <stdlib.h>
#include <assert.h>
//#include <omp.h>
#include "config.h"
#include "np_helper/np_helper.h"
#include "vhf/fblas.h"

void CCunpack_tril(double *tril, double *mat, int count, int n)
{
        NPdunpack_tril_2d(count, n, tril, mat, HERMITIAN);
}

void CCpack_tril(double *tril, double *mat, int count, int n)
{
        NPdpack_tril_2d(count, n, tril, mat);
}

/*
 * a * v1 + b * v2.transpose(0,2,1,3)
 */
void CCmake_0213(double *out, double *v1, double *v2, int count, int m,
                 double a, double b)
{
#pragma omp parallel default(none) \
        shared(count, m, out, v1, v2, a, b)
{
        int i, j, k, l, n;
        size_t d2 = m * m;
        size_t d1 = m * m * m;
        double *pv1, *pv2, *pout;
#pragma omp for schedule (static)
        for (i = 0; i < count; i++) {
                for (n = 0, j = 0; j < m; j++) {
                for (k = 0; k < m; k++) {
                        pout = out + d1*i + d2*j + m*k;
                        pv1  = v1  + d1*i + d2*j + m*k;
                        pv2  = v2  + d1*i + d2*k + m*j;
                        for (l = 0; l < m; l++, n++) {
                                pout[l] = pv1[l] * a + pv2[l] * b;
                        }
        } } }
}
}

/*
 * out = v1 + v2.transpose(0,2,1)
 */
void CCsum021(double *out, double *v1, double *v2, int count, int m)
{
#pragma omp parallel default(none) \
        shared(count, m, out, v1, v2)
{
        int i, j, k, n;
        size_t mm = m * m;
        double *pout, *pv1, *pv2;
#pragma omp for schedule (static)
        for (i = 0; i < count; i++) {
                pout = out + mm * i;
                pv1  = v1  + mm * i;
                pv2  = v2  + mm * i;
                for (n = 0, j = 0; j < m; j++) {
                for (k = 0; k < m; k++, n++) {
                        pout[n] = pv1[n] + pv2[k*m+j];
                } }
        }
}
}

/*
 * g2 = a * v1 + b * v2.transpose(0,2,1)
 */
void CCmake_021(double *out, double *v1, double *v2, int count, int m,
                double a, double b)
{
        if (a == 1 && b == 1) {
                return CCsum021(out, v1, v2, count, m);
        }

#pragma omp parallel default(none) \
        shared(count, m, out, v1, v2, a, b)
{
        int i, j, k, n;
        size_t mm = m * m;
        double *pout, *pv1, *pv2;
#pragma omp for schedule (static)
        for (i = 0; i < count; i++) {
                pout = out + mm * i;
                pv1  = v1  + mm * i;
                pv2  = v2  + mm * i;
                for (n = 0, j = 0; j < m; j++) {
                for (k = 0; k < m; k++, n++) {
                        pout[n] = pv1[n] * a + pv2[k*m+j] * b;
                } }
        }
}
}

/*
 * if matrix B is symmetric for the contraction A_ij B_ij,
 * Tr(AB) ~ A_ii B_ii + (A_ij + A_ji) B_ij where i > j
 * This function extract the A_ii and the lower triangluar part of A_ij + A_ji
 */
void CCprecontract(double *out, double *in, int count, int m, double diagfac)
{
#pragma omp parallel default(none) \
        shared(count, m, in, out, diagfac)
{
        int i, j, k, n;
        size_t mm = m * m;
        size_t m2 = m * (m+1) / 2;
        double *pout, *pin;
#pragma omp for schedule (static)
        for (i = 0; i < count; i++) {
                pout = out + m2 * i;
                pin  = in  + mm * i;
                for (n = 0, j = 0; j < m; j++) {
                        for (k = 0; k < j; k++, n++) {
                                pout[n] = pin[j*m+k] + pin[k*m+j];
                        }
                        pout[n] = pin[j*m+j] * diagfac;
                        n++;
                }
        }
}
}

