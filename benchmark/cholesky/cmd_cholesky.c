#include <command.h>
#include <common.h>

#include "dpad_cholesky.h"


#include "rxx.c"
#include "rxz.c"

#define PRECISE_F double

unsigned long sqrt_timer = 0;

PRECISE_F sqrt(double x)
{
    PRECISE_F   y;
    PRECISE_F   delta;
    PRECISE_F   maxError;
    

    unsigned long sqrt_start_time, sqrt_end_time;
    
    sqrt_start_time = get_timer(0);
    
    
    if ( x <= 0 ) 
    {
        return 0;
    }
    
    // initial guess
    y = x / 2;
    
    // refine
    maxError = x * 0.001;
    
    do 
    {
        delta = ( y * y ) - x;
        y -= delta / ( 2 * y );
    } while ( delta > maxError || delta < -maxError );

    sqrt_end_time = get_timer(0);
    
    sqrt_timer += (sqrt_end_time - sqrt_start_time);
    
    return y;
}

/******************************************************************************
 * Performs the Cholesky decomposition of the auto-correlation matrix acm
 *
 *  acm = L * L^T.

 * Since acm is symmetric by definition, only the lower triangular part is read.
 * The upper triangular part (including the main diagonal) is overwritten with
 * the Cholesky factor L^T. In order to save some divisions in the routine
 * fhw_cholsl, the inverse main diagonal of the Cholesky factor is stored separately
 * in the vector inv_diag. If acm is not positive definite (presumably due to
 * numerical errors), the routine returns the size of the greatest positive
 * definite block submatrix starting at acm[0][0].
 *
 * Computational complexity: O(size^3)
 *****************************************************************************/
int dapdCholdc(int size,           /*!<in    : (used) size of the matrix acm */
        double (*acm)[DAPD_RXX_ROW],    /*!<in/out: autocorrelation matrix / Cholesky factor */
        double *inv_diag   /*!<out   : inverse main diagonal of the Cholesky factor */
        )
{
    int i = 0, j = 0, k = 0;
    int end_of_computation = 0; /*Indicates whether the matrix is not positive definite*/
    double sum = 0.0, ppp = 0.0;
	
    //printf("%s enter,acm=%p\n", __func__,acm);
	//sleep(2);

    while ((i < size) && (end_of_computation == 0))
    {
        for (j = i; j < size; ++j)
        {
            sum = acm[j][i];

            for (k = i - 1; k >= 0; --k)
            {
                sum -= acm[k][i] * acm[k][j];
            }
            if (i == j)
            {
                if (sum <= 0.0)
                {
                    /* autocorrelation matrix is not positive definite*/
                    /* set i to the next smaller size (imag component)*/
                    /* and break cholesky decomposition*/
                    i = (i - 1) / 2;
                    i = i * 2;
                    end_of_computation = 1;
                }
                else
                {
                    /* This sqrt has to be calculated in double precision*/
                    /* otherwise rounding problems will cause difficulties!*/
                    inv_diag[i] = 1.0 / sqrt(sum);                       /*Inverse of main diagonal element*/
                    // inv_diag[i] = 1.0 / math_sqrt(sum);                       /*Inverse of main diagonal element*/
                    ppp = (3.0 - (sum * inv_diag[i] * inv_diag[i])) * 0.5; /*ppp is a stabilization factor*/
                    inv_diag[i] *= ppp;
                }
            }
            else
            {
                acm[i][j] = sum * inv_diag[i];
            }
        }
        i++;
    }
    for(j = i; j < DAPD_RXX_ROW; ++j)
    {
        inv_diag[j] = 1.0;  /* prevent divisions by zero */
    }

    /* Return the size up to which the autocorrelation matrix is pos. def.*/
    return i;

} /*End of dapdCholdc*/


/******************************************************************************
 * Solves a Least Squares (LS) problem using the pre-computed Cholesky factor
 *  of the autocorrelation matrix
 *
 *    L * L^T * coeffs = ccv,
 *
 *  where L * L^T is the Cholesky decomposition of the auto-correlation matrix.
 *
 *  Only the first min(chol_size, max_coeffs) coefficients are computed.
 *
 * Computational complexity: O(min(chol_size, max_coeffs)^2)
 *****************************************************************************/
void dapdCholsl(int chol_size,                   /*!<in : size of the Cholesky factor*/
        double (*lt)[DAPD_RXX_ROW],                   /*!<in : upper triang. part is Chol. factor L^T of autocorr. matrix*/
        double const *ccv,              /*!<in : cross-correlation vector*/
        double const *inv_diag,         /*!<in : inverse main diagonal of the Cholesky factor*/
        int max_coeffs,                  /*!<in : max number of coefficients to be computed*/
        double *coeffs                   /*!<out: LS coefficients*/
        )
{
    int i =0 , k = 0, max_row = 0; /*max_row limits the back substitutions*/
    double sum = 0.0;
    double y[DAPD_RXX_ROW];

    max_row = (max_coeffs <= chol_size) ? max_coeffs : chol_size;
    max_row |= 1; /*no even values for max_row*/

    /* 1st back substitution
       L^T * y = xcv       */
    for (i = 0; i < max_row; i++)
    {
        sum = ccv[i];
        for(k = i - 1; k >= 0; k--)
        {
            sum -= y[k] * lt[k][i];
        }
        y[i] = sum * inv_diag[i];
    }

    /* 2nd back substitution
       L^T * coeff = y     */
    for (i = max_row - 1; i >= 0; i--)
    {
        sum = y[i];
        for(k= i + 1; k < max_row; k++)
        {
            sum -=  coeffs[k] * lt[i][k];
        }
        coeffs[i] = sum * inv_diag[i];
    }

    /* Zero padding from max_row up to n-1*/
    for(i = max_row; i < DAPD_RXX_ROW; i++)
    {
        coeffs[i] = 0.0f;
    }

} /*End of dapdCholsl*/





void test_for_fun(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int i;
    volatile unsigned int j = 0;
    
    unsigned long start_time, end_time;
    
    start_time = get_timer(0);

    
    for (i = 0; i < 0xFFFFFF; i++)
    {
        j++;
    }
    end_time = get_timer(0);
    
    printf("****** for Start... ******, cur_time is: %d\n", start_time);
    printf("****** for end... ******, cur_time is: %d\n", end_time);
    
    printf("****** for time: %dms...******\n", (end_time - start_time));
    
     
}



void cholesky(void)
{
    double invDiag[DAPD_RXX_ROW];
	double coeffs[DAPD_RXX_ROW];
	int outNumCoeffs;	
    int numCoeffs = DAPD_RXX_ROW;  /**/	
    unsigned long start_time, end_time;
    
    
    start_time = get_timer(0);    
	outNumCoeffs = dapdCholdc(numCoeffs, rxx, invDiag);	
	dapdCholsl(DAPD_RXX_ROW, rxx, rxz, invDiag, outNumCoeffs, coeffs);
    end_time = get_timer(0);

    #if 1
    printf("****** cholesky Start... ******, cur_time is: %d\n", start_time);
    printf("****** cholesky End... ******, cur_time is: %d\n", end_time);
    
    printf("****** cholesky need time: %dms..., sqrt need time: %dms ******\n", 
        (end_time - start_time), sqrt_timer);
    #endif
}



U_BOOT_CMD (dapd_cholesky, 3, 1, cholesky,
	    "cholesky test in ram.",        
        "[void] [void]\n"
        );

    
U_BOOT_CMD (test_for, 3, 1, test_for_fun,
        "test_for functiont.",        
        "[void] [void]\n"
        );
