#ifndef DPAD_CHOLESKY_H_INCLUDED
#define DPAD_CHOLESKY_H_INCLUDED

#define GAIA//MAIA

#ifdef MAIA
#define DAPD_RXX_ROW                   55        /* max correlation matrix size */
#else
#ifdef GAIA
#define DAPD_RXX_ROW                   125
#endif
#endif

int dapdCholdc(int size,           /*!<in    : (used) size of the matrix acm */
        double (*acm)[DAPD_RXX_ROW],    /*!<in/out: autocorrelation matrix / Cholesky factor */
        double *inv_diag   /*!<out   : inverse main diagonal of the Cholesky factor */
        );

void dapdCholsl(int chol_size,                   /*!<in : size of the Cholesky factor*/
        double (*lt)[DAPD_RXX_ROW],                   /*!<in : upper triang. part is Chol. factor L^T of autocorr. matrix*/
        double const *ccv,              /*!<in : cross-correlation vector*/
        double const *inv_diag,         /*!<in : inverse main diagonal of the Cholesky factor*/
        int max_coeffs,                  /*!<in : max number of coefficients to be computed*/
        double *coeffs                   /*!<out: LS coefficients*/
        );


#endif // DPAD_CHOLESKY_H_INCLUDED
