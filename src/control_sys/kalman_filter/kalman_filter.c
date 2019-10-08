/* -----------------------------------------------------------------------------
 * Component Name: Kalman Filter
 * Parent Component: Control System
 * Author(s): Anja Möslinger
 * Purpose: Provide the calculations required for the Kalman Filter.
 *
 * -----------------------------------------------------------------------------
 */

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "global_utils.h"
#include "mode.h"
#include "kalman_filter.h"

#define X_PREV_ROWS 2
#define X_PREV_COLS 1
#define X_UPD_ROWS 2
#define X_UPD_COLS 1
#define X_NEXT_ROWS 2
#define X_NEXT_COLS 1
#define W_MEAS_ROWS 1
#define W_MEAS_COLS 1
#define PHI_ROWS 2
#define PHI_COLS 2
#define GAMMA_ROWS 2
#define GAMMA_COLS 1
#define UPSILON_ROWS 2
#define UPSILON_COLS 1
#define UPSILON2_ROWS 2
#define UPSILON2_COLS 1
#define H_ROWS 1
#define H_COLS 2
#define Q_ROWS 1
#define Q_COLS 1
#define R_ROWS 1
#define R_COLS 1
#define Q2_ROWS 1
#define Q2_COLS 1
#define P_PREV_ROWS 2
#define P_PREV_COLS 2
#define P_UPD_ROWS 2
#define P_UPD_COLS 2
#define P_NEXT_ROWS 2
#define P_NEXT_COLS 2
#define NU_NEXT_ROWS 1
#define NU_NEXT_COLS 1
#define S_NEXT_ROWS 1
#define S_NEXT_COLS 1
#define K_ROWS 2
#define K_COLS 1

static int open_logs(void);

static void eye(int m, double** mat);
static void madd(double** matA, double** matB, double** matC, int rows, int cols);
static void msub(double** matA, double** matB, double** matC, int rows, int cols);
static void transpose(double** mat, double** mat2, int rows, int cols);
static void mdiv(double** matA, double scal, double** matB, int rows, int cols);
static void mmult(double** matA, double** matB, double** matC, int rows1, int cols1, int rows2, int cols2);

static void printarray2D(double** array, int rows, int cols);

static void prop_state_vec(void);
static void prop_p_next(void);
static void innovate_nu_next(void);
static void update_s_next(void);
static void comp_k_gain(void);
static void update_state(void);
static void update_covar(void);

static double **x_prev;
static double **x_upd;
static double **x_next;
static double **w_meas;
static double **Phi;
static double **Gamma;
static double **Upsilon;
static double **Upsilon2;
static double **H;
static double **Q;
static double **R;
static double **Q2;
static double **P_prev;
static double **P_upd;
static double **P_next;
static double **nu_next;
static double **S_next;
static double **K;

// TODO: replace when ST stuff is available
//initial mode position
static double ang_init = 1;
// steps used for initial innovation
static int init_steps;

typedef struct{
    double*** var;
    int rows;
    int cols;
} arr_t;

/* Kalman filter
 *  double x_prev[2][1], x_upd[2][1], x_next[2][1];
 *  double w_meas[1][1];
 *  double Phi[2][2], Gamma[2][1], Upsilon[2][1], Upsilon2[2][1], H[1][2];
 *  double Q[1][1], Q2[1][1], R[1][1];
 *  double P_prev[2][2], P_upd[2][2], P_next[2][2], nu_next[1][1], S_next[1][1], K[2][1];
 */
static arr_t vars[18] = {
    {&x_prev,       X_PREV_ROWS,    X_PREV_COLS     },
    {&x_upd,        X_UPD_ROWS,     X_UPD_COLS      },
    {&x_next,       X_NEXT_ROWS,    X_NEXT_COLS     },
    {&w_meas,       W_MEAS_ROWS,    W_MEAS_COLS     },
    {&Phi,          PHI_ROWS,       PHI_COLS        },
    {&Gamma,        GAMMA_ROWS,     GAMMA_COLS      },
    {&Upsilon,      UPSILON_ROWS,   UPSILON_COLS    },
    {&Upsilon2,     UPSILON2_ROWS,  UPSILON2_COLS   },
    {&H,            H_ROWS,         H_COLS          },
    {&Q,            Q_ROWS,         Q_COLS          },
    {&R,            R_ROWS,         R_COLS          },
    {&Q2,           Q2_ROWS,        Q2_COLS         },
    {&P_prev,       P_PREV_ROWS,    P_PREV_COLS     },
    {&P_upd,        P_UPD_ROWS,     P_UPD_COLS      },
    {&P_next,       P_NEXT_ROWS,    P_NEXT_COLS     },
    {&nu_next,      NU_NEXT_ROWS,   NU_NEXT_COLS    },
    {&S_next,       S_NEXT_ROWS,    S_NEXT_COLS     },
    {&K,            K_ROWS,         K_COLS          }
};

static FILE* x_prev_log;
static FILE* p_prev_log;
static FILE* nu_next_log;
static FILE* s_next_log;

int init_kalman_filter(void* args){
    // initialisation parameters
    double dt;
    dt = (double)GYRO_SAMPLE_TIME/1000000000; // sampling time of the system

    /* TODO: remove for non testing */
    dt = 0.1; // for testing uses

    if(open_logs()){
        return errno;
    }

    // gyro parameters
    double ARW, RRW, gyro_bias_0;
    ARW = (.15/180*M_PI/60)*(.15/180*M_PI/60);    // angular random walk of gyro
    gyro_bias_0 = 10./180*M_PI/3600;
    //gyro_bias_0 = M_PI*10/180;
    RRW = (1./180*M_PI/3600/sqrt(3600))*(1./180*M_PI/3600/sqrt(3600));      // rate random walk of gyro

    // initialisation mode parameters
    double init_time = 30;
    double s_init = 0.1/180*M_PI;
    init_steps = init_time/dt;

    // allocate memory
    for(int ii = 0; ii < 18; ++ii){
        *vars[ii].var = malloc(vars[ii].rows * sizeof **vars[ii].var);
        if(*vars[ii].var == NULL){
            return ENOMEM;
        }
        for(int jj = 0; jj < vars[ii].rows; ++jj){
            (*vars[ii].var)[jj] = malloc(vars[ii].cols * sizeof **vars[ii].var[jj]);
            if((*vars[ii].var)[jj] == NULL){
                return ENOMEM;
            }
        }
    }

    // initialise Kalman filter

    //TODO: Replace with first ST measurement
    x_prev[0][0] = ang_init;  // starting position
    x_prev[1][0] = 0;

    P_prev[0][0] = s_init*s_init;
    P_prev[0][1] = 0;
    P_prev[1][0] = 0;
    P_prev[1][1] = gyro_bias_0*gyro_bias_0;

    // propagation matrices
    Phi[0][0] = 1;
    Phi[0][1] = -dt;
    Phi[1][0] = 0;
    Phi[1][1] = 1;

    Gamma[0][0] = dt;
    Gamma[1][0] = 0;

    Upsilon[0][0] = dt;
    Upsilon[1][0] = 0;
    Upsilon2[0][0] = 0;
    Upsilon2[1][0] = dt;

    // measurement matrix
    H[0][0] = 1;
    H[0][1] = 0;

    // tuning values
    Q[0][0] = ARW/dt;
    Q2[0][0] = RRW/dt;
    R[0][0] = s_init*s_init;

    return SUCCESS;
}

static int open_logs(void){

    char log_fn[100];

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/kf/x_upd.log");

    x_prev_log = fopen(log_fn, "a");
    if(x_prev_log == NULL){
        logging(ERROR, "Kalman F", "Failed to open x_upd log file: %m");
        return errno;
    }

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/kf/p_upd.log");

    p_prev_log = fopen(log_fn, "a");
    if(p_prev_log == NULL){
        logging(ERROR, "Kalman F", "Failed to open p_upd log file: %m");
        return errno;
    }

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/kf/nu_next.log");

    nu_next_log = fopen(log_fn, "a");
    if(nu_next_log == NULL){
        logging(ERROR, "Kalman F", "Failed to open nu_next log file: %m");
        return errno;
    }

    strcpy(log_fn, get_top_dir());
    strcat(log_fn, "output/logs/kf/s_next.log");

    s_next_log = fopen(log_fn, "a");
    if(s_next_log == NULL){
        logging(ERROR, "Kalman F", "Failed to open p_upd log file: %m");
        return errno;
    }

    return SUCCESS;
}

int kf_update(void){
    // TODO: not necessary anymore when ST data is incorporated
    static int l = 0;

    // get the gyro measurement
    // TODO: Replace with gyro data
    w_meas[0][0] = 0.001;

    // propagate state vector
    prop_state_vec();

    prop_p_next();

    // TODO: replace with check for if new ST data is available
    if (l < init_steps) { // update only for the first measurements where pointing is known (no motion)
        
        // innovation
        innovate_nu_next();
        update_s_next();

        // compute the Kalman gain
        comp_k_gain();

        // update of states and covariance
        update_state();
        update_covar();

        // save estimates (and log)
        //x_prev = x_upd;
        for(int i = 0; i < X_PREV_ROWS; i++) {
            for(int j = 0; j < X_PREV_COLS; j++) {
                x_prev[i][j] = x_upd[i][j];
            }
        }

        // propagate with initialisation measurement
        //P_prev = P_upd;
        for(int i = 0; i < P_PREV_ROWS; i++) {
            for(int j = 0; j < P_PREV_COLS; j++) {
                P_prev[i][j] = P_upd[i][j];
            }
        }

        //          log(x_upd);
        //          log(P_upd);
        //          log(nu_next);
        //          log(S_next);
        logging_csv(x_prev_log, "%+.10e,%+.10e", x_prev[0][0], x_prev[1][0]);

        logging_csv(p_prev_log, "%+.10e,%+.10e,%+.10e,%+.10e",
                P_prev[0][0], P_prev[0][1], P_prev[1][0], P_prev[1][1]);

        logging_csv(nu_next_log, "%+.10e", **nu_next);
        logging_csv(s_next_log, "%+.10e", **S_next);

        #ifdef KF_DEBUG
            logging(DEBUG, "Kalman F", "step number: %d", l);
            logging(DEBUG, "Kalman F", "x_upd:");
            logging(DEBUG, "Kalman F", "%+.6e", x_upd[0][0]);
            logging(DEBUG, "Kalman F", "%+.6e", x_upd[1][0]);

            logging(DEBUG, "Kalman F", "P_upd:");
            logging(DEBUG, "Kalman F", "%+.6e\t%+.6e",
                    P_upd[0][0], P_upd[0][1]);
            logging(DEBUG, "Kalman F", "%+.6e\t%+.6e",
                    P_upd[1][0], P_upd[1][1]);

            logging(DEBUG, "Kalman F", "nu_next:");
            logging(DEBUG, "Kalman F", "%+.6e", nu_next[0][0]);

            logging(DEBUG, "Kalman F", "S_next:");
            logging(DEBUG, "Kalman F", "%+.6e", S_next[0][0]);

        #endif
        
        prop_p_next();

    }
    // new ST data not available
    else { // initialisation completed - only propagate from this point

        /* TODO: if we dont get any ST measurements for 10 min
         *       call set_mode(RESET);
         *       and send message as telemetry & log
         */

        //x_prev = x_next;
        for(int i = 0; i < X_PREV_ROWS; i++) {
            for(int j = 0; j < X_PREV_COLS; j++) {
                x_prev[i][j] = x_next[i][j];
            }
        }
        //P_prev = P_next;
        for(int i = 0; i < P_PREV_ROWS; i++) {
            for(int j = 0; j < P_PREV_COLS; j++) {
                P_prev[i][j] = P_next[i][j];
            }
        }

        // save estimates
        //          log(x_next);
        //          log(P_next);
        logging_csv(x_prev_log, "%+.10e,%+.10e", x_prev[0][0], x_prev[1][0]);

        logging_csv(p_prev_log, "%+.10e,%+.10e,%+.10e,%+.10e",
                P_prev[0][0], P_prev[0][1], P_prev[1][0], P_prev[1][1]);

        //          printf("x_prev for i = %d", l);
        //          printarray2D(x_prev, X_PREV_ROWS, X_PREV_COLS);
        // x_next = Phi * x_prev + Gamma * w_meas;
        printf("x_next for i = %d", l);
        printarray2D(x_next, X_NEXT_ROWS, X_NEXT_COLS);
        printf("P_next for i = %d", l);
        printarray2D(P_next, P_NEXT_ROWS, P_NEXT_COLS);

    }

    l++;
    return SUCCESS;
}


/*******************************************************************************
********************************************************************************
************************************KF**FUNCS***********************************
********************************************************************************
*******************************************************************************/


/* x_next = Phi * x_prev + Gamma * w_meas; */
static void prop_state_vec(void){

    // definition of mmult1[PHI_ROWS][X_PREV_COLS]
    double **mmult1;
    int mmult1_rows = PHI_ROWS;
    int mmult1_cols = X_PREV_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(Phi, x_prev, mmult1, PHI_ROWS, PHI_COLS, X_PREV_ROWS, X_PREV_COLS);

    //      printf("x_prev for i = %d", l);
    //      printarray2D(x_prev, X_PREV_ROWS, X_PREV_COLS);
    // definition of mmult2[GAMMA_ROWS][W_MEAS_COLS]
    double **mmult2;
    int mmult2_rows = GAMMA_ROWS;
    int mmult2_cols = W_MEAS_COLS;

    mmult2 = malloc(mmult2_rows * sizeof *mmult2);
    for (int i = 0; i < mmult2_rows; i++) {
        mmult2[i] = malloc(mmult2_cols * sizeof *mmult2[i]);
    }
    mmult(Gamma, w_meas, mmult2, GAMMA_ROWS, GAMMA_COLS, W_MEAS_ROWS, W_MEAS_COLS);

    madd(mmult1, mmult2, x_next, mmult1_rows, mmult1_cols);
    // x_next = Phi * x_prev + Gamma * w_meas;
    //      printf("x_next for i = %d", l);
    //      printarray2D(x_next, X_NEXT_ROWS, X_NEXT_COLS);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);

    // free mmult2
    for (int i = 0; i < mmult2_rows; i++) {
        free(mmult2[i]);
    }
    free(mmult2);
}

/* P_next = Phi*P_prev*Phi' + Upsilon*Q*Upsilon' + Upsilon2*Q2*Upsilon2'; */
static void prop_p_next(void){

    // propagate P_next
    // re-definition of mmult1[length(Phi)][length(P_prev[1])];
    double** mmult1;
    int mmult1_rows = PHI_ROWS;
    int mmult1_cols = P_PREV_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(Phi, P_prev, mmult1, PHI_ROWS, PHI_COLS, P_PREV_ROWS, P_PREV_COLS);
    // definition of trans[length(Phi[0])][length(Phi)];
    double **trans;
    int trans_rows = PHI_COLS;
    int trans_cols = PHI_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(Phi, trans, PHI_ROWS, PHI_COLS);

    // re-definition of mmult2[length(mmult1)][length(Phi)];
    double **mmult2;
    int mmult2_rows = mmult1_rows;
    int mmult2_cols = trans_cols;

    mmult2 = malloc(mmult2_rows * sizeof *mmult2);
    for (int i = 0; i < mmult2_rows; i++) {
        mmult2[i] = malloc(mmult2_cols * sizeof *mmult2[i]);
    }
    mmult(mmult1, trans, mmult2, mmult1_rows, mmult1_cols, trans_rows, trans_cols);


    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);
    // free trans
    for (int i = 0; i < trans_rows; i++) {
        free(trans[i]);
    }
    free(trans);

    //              printf("Phi*P_prev*Phi for i = %d", l);
    //              printarray2D(mmult2, mmult2_rows, mmult2_cols);
    //mmult2 = Phi*P_prev*Phi'

    // re-definition of mmult1[length(Upsilon)][length(Q[1])];
    mmult1_rows = UPSILON_ROWS;
    mmult1_cols = Q_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(Upsilon, Q, mmult1, UPSILON_ROWS, UPSILON_COLS, Q_ROWS, Q_COLS);
    // re-definition of trans[length(Upsilon[0])][length(Upsilon)];
    trans_rows = UPSILON_COLS;
    trans_cols = UPSILON_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(Upsilon, trans, UPSILON_ROWS, UPSILON_COLS);

    // definition of mmult3[length(mmult1)][length(Q)];
    double **mmult3;
    int mmult3_rows = mmult1_rows;
    int mmult3_cols = trans_cols;

    mmult3 = malloc(mmult3_rows * sizeof *mmult3);
    for (int i = 0; i < mmult3_rows; i++) {
        mmult3[i] = malloc(mmult3_cols * sizeof *mmult3[i]);
    }
    mmult(mmult1, trans, mmult3, mmult1_rows, mmult1_cols, trans_rows, trans_cols);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);
    // free trans
    for (int i = 0; i < trans_rows; i++) {
        free(trans[i]);
    }
    free(trans);
    //mmult3 = Upsilon*Q*Upsilon'

    // re-definition of mmult1[length(Upsilon2)][length(Q2[1])];
    mmult1_rows = UPSILON2_ROWS;
    mmult1_cols = Q2_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(Upsilon2, Q2, mmult1, UPSILON2_ROWS, UPSILON2_COLS, Q2_ROWS, Q2_COLS);

    // re-definition of trans[length(Upsilon2[0])][length(Upsilon2)];
    trans_rows = UPSILON2_COLS;
    trans_cols = UPSILON2_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(Upsilon2, trans, UPSILON2_ROWS, UPSILON2_COLS);

    // definition of mmult4[length(mmult1)][length(Q2)];
    double **mmult4;
    int mmult4_rows = mmult1_rows;
    int mmult4_cols = trans_cols;

    mmult4 = malloc(mmult4_rows * sizeof *mmult4);
    for (int i = 0; i < mmult4_rows; i++) {
        mmult4[i] = malloc(mmult4_cols * sizeof *mmult4[i]);
    }
    mmult(mmult1, trans, mmult4, mmult1_rows, mmult1_cols, trans_rows, trans_cols);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);
    // free trans
    for (int i = 0; i < trans_rows; i++) {
        free(trans[i]);
    }
    free(trans);
    //mmult4 = Upsilon2*Q2*Upsilon2'

    // definition of madd1[length(mmult2)][length(mmult2[1])];
    double **madd1;
    int madd1_rows = mmult2_rows;
    int madd1_cols = mmult2_cols;

    madd1 = malloc(madd1_rows * sizeof *madd1);
    for (int i = 0; i < madd1_rows; i++) {
        madd1[i] = malloc(madd1_cols * sizeof *madd1[i]);
    }
    madd(mmult2, mmult3, madd1, mmult2_rows, mmult2_cols);

    madd(madd1, mmult4, P_next, madd1_rows, madd1_cols);


    // free mmult2
    for (int i = 0; i < mmult2_rows; i++) {
        free(mmult2[i]);
    }
    free(mmult2);

    // free mmult3
    for (int i = 0; i < mmult3_rows; i++) {
        free(mmult3[i]);
    }
    free(mmult3);

    // free mmult4
    for (int i = 0; i < mmult4_rows; i++) {
        free(mmult4[i]);
    }
    free(mmult4);

    // free madd1
    for (int i = 0; i < madd1_rows; i++) {
        free(madd1[i]);
    }
    free(madd1);
    //      printf("P_next for i = %d", l);
    //      printarray2D(P_next, P_NEXT_ROWS, P_NEXT_COLS);
    //P_next = Phi*P_prev*Phi' + Upsilon*Q*Upsilon' + Upsilon2*Q2*Upsilon2';
}

/* nu_next = t_meas - H*x_est_next; */
static void innovate_nu_next(void){

    // re-definition of mmult1[length(H)][length(x_next[1])];
    double** mmult1;
    int mmult1_rows = H_ROWS;
    int mmult1_cols = X_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(H, x_next, mmult1, H_ROWS, H_COLS, X_NEXT_ROWS, X_NEXT_COLS);
    // definition of ang as pointer
    double **ang;
    int ang_rows = 1;
    int ang_cols = 1;

    ang = malloc(ang_rows * sizeof *ang);
    for (int i = 0; i < ang_rows; i++) {
        ang[i] = malloc(ang_cols * sizeof *ang[i]);
    }
    // TODO: fetch ST data
    ang[0][0] = ang_init;

    msub(ang, mmult1, nu_next, ang_rows, ang_cols);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);
    // free ang
    for (int i = 0; i < ang_rows; i++) {
        free(ang[i]);
    }
    free(ang);

    // nu_next = t_meas - H*x_est_next;
    //'propagation error' (difference between estimated state and measured state
}

/* S_next = H*P_next*H' + R; */
static void update_s_next(void){

    // re-definition of mmult1[length(H)][length(P_next[1])];
    double** mmult1;
    int mmult1_rows = H_ROWS;
    int mmult1_cols = P_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(H, P_next, mmult1, H_ROWS, H_COLS, P_NEXT_ROWS, P_NEXT_COLS);

    // definition of trans[length(H[0])][length(H)];
    double** trans;
    int trans_rows = H_COLS;
    int trans_cols = H_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(H, trans, H_ROWS, H_COLS);

    // re-definition of mmult2[length(mmult1)][length(H)];
    double** mmult2;
    int mmult2_rows = mmult1_rows;
    int mmult2_cols = trans_cols;

    mmult2 = malloc(mmult2_rows * sizeof *mmult2);
    for (int i = 0; i < mmult2_rows; i++) {
        mmult2[i] = malloc(mmult2_cols * sizeof *mmult2[i]);
    }
    mmult(mmult1, trans, mmult2, mmult1_rows, mmult1_cols, trans_rows, trans_cols);

    madd(mmult2, R, S_next, mmult2_rows, mmult2_cols);
    //          printf("S_next for i = %d", l);
    //          printarray2D(S_next, S_NEXT_ROWS, S_NEXT_COLS);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);

    // free mmult2
    for (int i = 0; i < mmult2_rows; i++) {
        free(mmult2[i]);
    }
    free(mmult2);

    // free trans
    for (int i = 0; i < trans_rows; i++) {
        free(trans[i]);
    }
    free(trans);
    //S_next = H*P_next*H' + R;
}

/* K = P_next*H'/S_next; */
static void comp_k_gain(void){
    // re-definition of trans[length(H[0])][length(H)];
    double** trans;
    int trans_rows = H_COLS;
    int trans_cols = H_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(H, trans, H_ROWS, H_COLS);

    // re-definition of mmult1[length(P_next)][length(H)];
    double** mmult1;
    int mmult1_rows = P_NEXT_ROWS;
    int mmult1_cols = trans_cols;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(P_next, trans, mmult1, P_NEXT_ROWS, P_NEXT_COLS, trans_rows, trans_cols);
    mdiv(mmult1, S_next[0][0], K, mmult1_rows, mmult1_cols);


    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);

    // free trans
    for (int i = 0; i < trans_rows; i++) {
        free(trans[i]);
    }
    free(trans);

    //          printf("K for i = %d", l);
    //          printarray2D(K, K_ROWS, K_COLS);
    // K = P_next*H'/S_next;
}

/* x_upd = x_est_next + K*nu_next; */
static void update_state(void){
    //double mmult1[length(K)][length(nu_next[1])];
    // re-definition of mmult1[length(K)][length(nu_next[1])];
    double** mmult1;
    int mmult1_rows = K_ROWS;
    int mmult1_cols = NU_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(K, nu_next, mmult1, K_ROWS, K_COLS, NU_NEXT_ROWS, NU_NEXT_COLS);
    madd(x_next, mmult1, x_upd, X_NEXT_ROWS, X_NEXT_COLS);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);

    //          printf("x_upd for i = %d", l);
    //          printarray2D(x_upd, X_UPD_ROWS, X_UPD_COLS);
    // x_upd = x_est_next + K*nu_next;
}

/* P_upd = (eye(2)-K*H)*P_next; */
static void update_covar(void){
    // re-definition of mmult1[length(K)][length(H[1])];
    double** mmult1;
    int mmult1_rows = K_ROWS;
    int mmult1_cols = H_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    if(mmult1 == NULL){
        logging(ERROR, "Kalman F", "malloc: %m");
    }
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
        if(mmult1[i] == NULL){
            logging(ERROR, "Kalman F", "malloc: %m");
        }
    }
    mmult(K, H, mmult1, K_ROWS, K_COLS, H_ROWS, H_COLS);
    //printarray2D(mmult1, mmult1_rows, mmult1_cols);
    // definition of eye2[2][2];
    double **eye2;
    int eye2_rows = 2;
    int eye2_cols = 2;

    eye2 = malloc(eye2_rows * sizeof *eye2);
    if(eye2 == NULL){
        logging(ERROR, "Kalman F", "malloc: %m");
    }
    for (int i = 0; i < eye2_rows; i++) {
        eye2[i] = malloc(eye2_cols * sizeof *eye2[i]);
        if(eye2[i] == NULL){
            logging(ERROR, "Kalman F", "malloc: %m");
        }
    }
    eye(2, eye2);
    //printarray2D(eye2, 2, 2);

    //double msub1[2][2];
    // definition of msub1[2][2];
    double **msub1;
    int msub1_rows = 2;
    int msub1_cols = 2;

    msub1 = malloc(msub1_rows * sizeof *msub1);
    if(msub1 == NULL){
        logging(ERROR, "Kalman F", "malloc: %m");
    }
    for (int i = 0; i < msub1_rows; i++) {
        msub1[i] = malloc(msub1_cols * sizeof *msub1[i]);
        if(msub1[i] == NULL){
            logging(ERROR, "Kalman F", "malloc: %m");
        }
    }
    msub(eye2, mmult1, msub1, eye2_rows, eye2_cols);
    //printarray2D(msub1, msub1_rows, msub1_cols);
    mmult(msub1, P_next, P_upd, msub1_rows, msub1_cols, P_NEXT_ROWS, P_NEXT_COLS);

    // free mmult1
    for (int i = 0; i < mmult1_rows; i++) {
        free(mmult1[i]);
    }
    free(mmult1);
    // free eye2
    for (int i = 0; i < eye2_rows; i++) {
        free(eye2[i]);
    }
    free(eye2);
    // free msub1
    for (int i = 0; i < msub1_rows; i++) {
        free(msub1[i]);
    }
    free(msub1);

    //          printf("P_upd for i = %d", l);
    //          printarray2D(P_upd, P_UPD_ROWS, P_UPD_COLS);
    // P_upd = (eye(2)-K*H)*P_next;
}

/*******************************************************************************
********************************************************************************
***********************************MATH*FUNCS***********************************
********************************************************************************
*******************************************************************************/


static void printarray2D(double** array, int rows, int cols) {
    printf("\nPrint 2D array\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf(" %+.11e",array[i][j]);
        }
        printf("\n");
    }
}

static void eye(int m, double** mat) {
    for(int ii=0; ii<m; ++ii){
        for(int jj=0; jj<m; ++jj){
            if(ii==jj){
                mat[ii][jj] = 1.0;
            }
            else{
                mat[ii][jj] = 0.0;
            }
        }
    }
}

static void madd(double** matA, double** matB, double** matC, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matC[i][j] = matA[i][j] + matB[i][j];
        }
    }
}

static void msub(double** matA, double** matB, double** matC, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matC[i][j] = matA[i][j] - matB[i][j];
        }
    }
}

static void transpose(double** mat, double** mat2, int rows, int cols) {
    for (int i = 0; i<rows; i++) {
        for (int j = 0; j<cols; j++) {
            mat2[j][i] = mat[i][j];
        }
    }
}

static void mdiv(double** matA, double scal, double** matB, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            matB[i][j] = matA[i][j] / scal;
        }
    }
}

static void mmult(double** matA, double** matB, double** matC, int rows1, int cols1, int rows2, int cols2) {
    if (rows2 != cols1) {
        printf("Rows and columns don't match");
    } else {
        for(int m=0;m<rows1;m++){
            for(int k=0;k<cols2;k++){
                matC[m][k]=0;
                for(int n=0;n<cols1;n++){
                    matC[m][k]+=matA[m][n]*matB[n][k];
                }
            }
        }
    }
}
