/* -----------------------------------------------------------------------------
 * Component Name: Kalman Filter
 * Parent Component: Control System
 * Author(s): Anja Möslinger, Harald Magnusson
 * Purpose: Provide the calculations required for the Kalman Filter.
 *
 * -----------------------------------------------------------------------------
 */

#define _GNU_SOURCE

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>

#include "global_utils.h"
#include "mode.h"
#include "current_target.h"
#include "kalman_filter.h"
#include "sensors.h"
#include "control_sys.h"
#include "target_selection.h"

/* Kalman filter
 *  double x_prev[2][1], x_upd[2][1], x_next[2][1];
 *  double w_meas[1][1];
 *  double Phi[2][2], Gamma[2][1], Upsilon[2][1], Upsilon2[2][1], H[1][2];
 *  double Q[1][1], Q2[1][1], R[1][1];
 *  double P_prev[2][2], P_upd[2][2], P_next[2][2], nu_next[1][1], S_next[1][1], K[2][1];
 */
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

#define AZ_AXIS_FLAG 0
#define ALT_AXIS_FLAG 1

typedef struct{
    double*** var;
    int rows;
    int cols;
} arr_t;

typedef struct{
    arr_t mem[18];

    double** x_prev;
    double** x_upd;
    double** x_next;
    double** w_meas;
    double** Phi;
    double** Gamma;
    double** Upsilon;
    double** Upsilon2;
    double** H;
    double** Q;
    double** R;
    double** Q2;
    double** P_prev;
    double** P_upd;
    double** P_next;
    double** nu_next;
    double** S_next;
    double** K;

    double* gyro_hist;
    double** x_hist;
    double*** p_hist;

    /* innovative logs */
    FILE* x_log;
    FILE* p_log;
    FILE* nu_log;
    FILE* s_log;

    /* propagation logs */
    FILE* x_prop_log;
    FILE* p_prop_log;
} axis_context_t;

static int open_logs(void);
static void init_kalman_vars(double x_init, double y_init, double z_init);
static int kf_axis(axis_context_t axis, double gyro_data, double* st_data);

static void eye(int m, double** mat);
static void madd(double** matA, double** matB, double** matC, int rows, int cols);
static void msub(double** matA, double** matB, double** matC, int rows, int cols);
static void transpose(double** mat, double** mat2, int rows, int cols);
static void mdiv(double** matA, double scal, double** matB, int rows, int cols);
static void mmult(double** matA, double** matB, double** matC, int rows1, int cols1, int rows2, int cols2);

#if 0
static void printarray2D(double** array, int rows, int cols);
#endif

static void prop_state_vec(axis_context_t axis);
static void prop_p_next(axis_context_t axis);
static void innovate_nu_next(axis_context_t axis, double* st_data);
static void update_s_next(axis_context_t axis);
static void comp_k_gain(axis_context_t axis);
static void update_state(axis_context_t axis);
static void update_covar(axis_context_t axis);

static axis_context_t x = {
    {
        {&x.x_prev,       X_PREV_ROWS,    X_PREV_COLS     },
        {&x.x_upd,        X_UPD_ROWS,     X_UPD_COLS      },
        {&x.x_next,       X_NEXT_ROWS,    X_NEXT_COLS     },
        {&x.w_meas,       W_MEAS_ROWS,    W_MEAS_COLS     },
        {&x.Phi,          PHI_ROWS,       PHI_COLS        },
        {&x.Gamma,        GAMMA_ROWS,     GAMMA_COLS      },
        {&x.Upsilon,      UPSILON_ROWS,   UPSILON_COLS    },
        {&x.Upsilon2,     UPSILON2_ROWS,  UPSILON2_COLS   },
        {&x.H,            H_ROWS,         H_COLS          },
        {&x.Q,            Q_ROWS,         Q_COLS          },
        {&x.R,            R_ROWS,         R_COLS          },
        {&x.Q2,           Q2_ROWS,        Q2_COLS         },
        {&x.P_prev,       P_PREV_ROWS,    P_PREV_COLS     },
        {&x.P_upd,        P_UPD_ROWS,     P_UPD_COLS      },
        {&x.P_next,       P_NEXT_ROWS,    P_NEXT_COLS     },
        {&x.nu_next,      NU_NEXT_ROWS,   NU_NEXT_COLS    },
        {&x.S_next,       S_NEXT_ROWS,    S_NEXT_COLS     },
        {&x.K,            K_ROWS,         K_COLS          }
    }
};

static axis_context_t y = {
    {
        {&y.x_prev,       X_PREV_ROWS,    X_PREV_COLS     },
        {&y.x_upd,        X_UPD_ROWS,     X_UPD_COLS      },
        {&y.x_next,       X_NEXT_ROWS,    X_NEXT_COLS     },
        {&y.w_meas,       W_MEAS_ROWS,    W_MEAS_COLS     },
        {&y.Phi,          PHI_ROWS,       PHI_COLS        },
        {&y.Gamma,        GAMMA_ROWS,     GAMMA_COLS      },
        {&y.Upsilon,      UPSILON_ROWS,   UPSILON_COLS    },
        {&y.Upsilon2,     UPSILON2_ROWS,  UPSILON2_COLS   },
        {&y.H,            H_ROWS,         H_COLS          },
        {&y.Q,            Q_ROWS,         Q_COLS          },
        {&y.R,            R_ROWS,         R_COLS          },
        {&y.Q2,           Q2_ROWS,        Q2_COLS         },
        {&y.P_prev,       P_PREV_ROWS,    P_PREV_COLS     },
        {&y.P_upd,        P_UPD_ROWS,     P_UPD_COLS      },
        {&y.P_next,       P_NEXT_ROWS,    P_NEXT_COLS     },
        {&y.nu_next,      NU_NEXT_ROWS,   NU_NEXT_COLS    },
        {&y.S_next,       S_NEXT_ROWS,    S_NEXT_COLS     },
        {&y.K,            K_ROWS,         K_COLS          }
    }
};

static axis_context_t z = {
    {
        {&z.x_prev,       X_PREV_ROWS,    X_PREV_COLS     },
        {&z.x_upd,        X_UPD_ROWS,     X_UPD_COLS      },
        {&z.x_next,       X_NEXT_ROWS,    X_NEXT_COLS     },
        {&z.w_meas,       W_MEAS_ROWS,    W_MEAS_COLS     },
        {&z.Phi,          PHI_ROWS,       PHI_COLS        },
        {&z.Gamma,        GAMMA_ROWS,     GAMMA_COLS      },
        {&z.Upsilon,      UPSILON_ROWS,   UPSILON_COLS    },
        {&z.Upsilon2,     UPSILON2_ROWS,  UPSILON2_COLS   },
        {&z.H,            H_ROWS,         H_COLS          },
        {&z.Q,            Q_ROWS,         Q_COLS          },
        {&z.R,            R_ROWS,         R_COLS          },
        {&z.Q2,           Q2_ROWS,        Q2_COLS         },
        {&z.P_prev,       P_PREV_ROWS,    P_PREV_COLS     },
        {&z.P_upd,        P_UPD_ROWS,     P_UPD_COLS      },
        {&z.P_next,       P_NEXT_ROWS,    P_NEXT_COLS     },
        {&z.nu_next,      NU_NEXT_ROWS,   NU_NEXT_COLS    },
        {&z.S_next,       S_NEXT_ROWS,    S_NEXT_COLS     },
        {&z.K,            K_ROWS,         K_COLS          }
    }
};

#ifdef KF_TEST
    //initial mode position
    static double ang_init = 0;
    // steps used for initial innovation
    static int init_steps;
    static int l = 0;
#endif

int init_kalman_filter(void* args){

    if(open_logs()){
        return errno;
    }

    // allocate memory
    axis_context_t* arr[3] = {&x, &y, &z};
    for(int ii=0; ii<3; ++ii){
        for(int jj=0; jj<18; ++jj){
            int rows = arr[ii]->mem[jj].rows;
            int cols = arr[ii]->mem[jj].cols;

            *arr[ii]->mem[jj].var = malloc(rows * sizeof **arr[ii]->mem[jj].var);

            if(*arr[ii]->mem[jj].var == NULL){
                logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
                return ENOMEM;
            }

            for(int kk=0; kk < rows; ++kk){
                (*arr[ii]->mem[jj].var)[kk] = malloc(cols * sizeof **arr[ii]->mem[jj].var[kk]);

                if(*arr[ii]->mem[jj].var == NULL){
                    logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
                    return ENOMEM;
                }
            }
        }

        /* gyro history */
        int sec = 60; /* seconds to save */
        int frequency = 1000*1000*1000 / GYRO_SAMPLE_TIME;
        int hist_elements = sec * frequency;
        arr[ii]->gyro_hist = malloc(hist_elements * sizeof(*arr[ii]->gyro_hist));
        if(arr[ii]->gyro_hist == NULL){
            logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
            return ENOMEM;
        }

        /* estimated state history */
        arr[ii]->x_hist = malloc(hist_elements * sizeof(*arr[ii]->x_hist));
        if(arr[ii]->x_hist == NULL){
            logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
            return ENOMEM;
        }

        for(int jj=0; jj < hist_elements; ++jj){
            arr[ii]->x_hist[jj] = malloc(X_PREV_ROWS * sizeof(*arr[ii]->x_hist[jj]));
            if(arr[ii]->x_hist[jj] == NULL){
                logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
                return ENOMEM;
            }
        }

        /* P matrix history */
        arr[ii]->p_hist = malloc(hist_elements * sizeof(*arr[ii]->p_hist));
        if(arr[ii]->p_hist == NULL){
            logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
            return ENOMEM;
        }

        for(int jj=0; jj < hist_elements; ++jj){
            arr[ii]->p_hist[jj] = malloc(P_PREV_ROWS * sizeof(*arr[ii]->p_hist[jj]));
            if(arr[ii]->p_hist[jj] == NULL){
                logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
                return ENOMEM;
            }

            for(int kk=0; kk<P_PREV_ROWS; ++kk){
                arr[ii]->p_hist[jj][kk] = malloc(P_PREV_COLS * sizeof(*arr[ii]->p_hist[jj][kk]));
                if(arr[ii]->p_hist[jj][kk] == NULL){
                    logging(ERROR, "Kalman F", "Cannot allocate memory: %m");
                    return ENOMEM;
                }

            }
        }
    }

    // initialise Kalman filter
    #ifdef KF_TEST
        init_kalman_vars(0, 0, 0);
    #else
        init_kalman_vars(0, 0, 45);
    #endif

    return SUCCESS;
}

static void init_kalman_vars(double x_init, double y_init, double z_init){

    // initialisation parameters
    double dt;
    dt = (double)GYRO_SAMPLE_TIME/1000000000; // sampling time of the system

    // gyro parameters
    double ARW, RRW, gyro_bias_0;
    ARW = (.15/180*M_PI/60)*(.15/180*M_PI/60);    // angular random walk of gyro
    gyro_bias_0 = 10./180*M_PI/3600;
    //gyro_bias_0 = M_PI*10/180;
    RRW = (1./180*M_PI/3600/sqrt(3600))*(1./180*M_PI/3600/sqrt(3600));      // rate random walk of gyro

    // initialisation mode parameters
    double init_time = 300;
    double s_init = 0.1/180*M_PI;
    init_steps = init_time/dt;


    axis_context_t* arr[3] = {&x, &y, &z};

    x.x_prev[0][0] = x_init;  // starting position
    y.x_prev[0][0] = y_init;
    z.x_prev[0][0] = z_init;

    for(int ii=0; ii<3; ++ii){
        arr[ii]->x_prev[1][0] = 0;

        arr[ii]->P_prev[0][0] = s_init*s_init;
        arr[ii]->P_prev[0][1] = 0;
        arr[ii]->P_prev[1][0] = 0;
        arr[ii]->P_prev[1][1] = gyro_bias_0*gyro_bias_0;

        // propagation matrices
        arr[ii]->Phi[0][0] = 1;
        arr[ii]->Phi[0][1] = -dt;
        arr[ii]->Phi[1][0] = 0;
        arr[ii]->Phi[1][1] = 1;

        arr[ii]->Gamma[0][0] = dt;
        arr[ii]->Gamma[1][0] = 0;

        arr[ii]->Upsilon[0][0] = dt;
        arr[ii]->Upsilon[1][0] = 0;
        arr[ii]->Upsilon2[0][0] = 0;
        arr[ii]->Upsilon2[1][0] = dt;

        // measurement matrix
        arr[ii]->H[0][0] = 1;
        arr[ii]->H[0][1] = 0;

        // tuning values
        arr[ii]->Q[0][0] = ARW/dt;
        arr[ii]->Q2[0][0] = RRW/dt;
        arr[ii]->R[0][0] = s_init*s_init;
    }
}

static int open_logs(void){

    char axes[3] = {'x', 'y', 'z'};
    char* vars[] = {"x", "p", "nu", "s", "x_prop", "p_prop"};
    axis_context_t* arr[3] = {&x, &y, &z};

    char log_fn[100];
    strcpy(log_fn, get_top_dir());
    /* dirlen is the index in log_fn where local paths start */
    int dirlen = strlen(log_fn);

    for(int ii=0; ii<3; ++ii){
        FILE** logs[6] = {&arr[ii]->x_log, &arr[ii]->p_log, &arr[ii]->nu_log,
                &arr[ii]->s_log, &arr[ii]->x_prop_log, &arr[ii]->p_prop_log};

        for(int jj=0; jj<6; ++jj){

            snprintf(&log_fn[dirlen], 100-dirlen, "output/logs/kf/%c/%s.log", axes[ii], vars[jj]);
            *logs[jj] = fopen(log_fn, "a");
            if(*logs[jj] == NULL){
                logging(ERROR, "Kalman F", "Failed to open %s log for axis %c: %m",
                        vars[jj], axes[ii]);
                return errno;
            }
        }
    }

    return SUCCESS;
}

static char first_st_flag = 1;
static size_t hist_index = 0;
int kf_update(telescope_att_t* cur_att){

    gyro_t gyro;
    get_gyro(&gyro);

    star_tracker_t st;
    #ifdef KF_TEST
        if(l % 1000 == 0){
        //if(l<init_steps){
            st.ra = 0;
            st.dec = 0;
            st.roll = 0;
            st.new_data = 1;
            st.out_of_date = 0;
        }
        else{
            st.new_data = 0;
        }
    #else
        get_star_tracker(&st);
    #endif

    if(st.new_data){

        double az = 0, alt = 0;
        /* convert ra & dec to az & alt */
        #ifndef KF_TEST
            rd_to_aa(st.ra, st.dec, &az, &alt);
        #endif

        /* extract x from star tracker */
        double sin_alt = sin(alt * M_PI / 180);
        double cos_alt = cos(alt * M_PI / 180);

        double st_x = (az + st.roll * sin_alt) / cos_alt;

        if(first_st_flag){
            init_kalman_vars(st_x, st.roll, alt);
            set_tracking_angles(az, alt);

            logging(INFO, "MODE", "Waking selection and tracking");
            pthread_mutex_lock(&mutex_cond_sel_track);
            pthread_cond_signal(&cond_sel_track);
            pthread_mutex_unlock(&mutex_cond_sel_track);

            first_st_flag = 0;
        }

        kf_axis(x, gyro.x, &st_x);
        kf_axis(y, gyro.y, &st.roll);
        kf_axis(z, gyro.z, &alt);

        hist_index = 0;
    }
    else{
        kf_axis(x, gyro.x, NULL);
        kf_axis(y, gyro.y, NULL);
        kf_axis(z, gyro.z, NULL);

        hist_index++;
    }

    double sin_z = sin(z.x_prev[0][0] * M_PI / 180);
    double cos_z = cos(z.x_prev[0][0] * M_PI / 180);

    cur_att->az = x.x_prev[0][0] * cos_z - y.x_prev[0][0] * sin_z;
    cur_att->alt = z.x_prev[0][0];

    set_telescope_att(cur_att);

    l++;

    if(l == 100000){
        return FAILURE;
    }
    return SUCCESS;
}

static int kf_axis(axis_context_t axis, double gyro_data, double* st_data){

    axis.w_meas[0][0] = gyro_data;

    // propagate state vector
    prop_state_vec(axis);

    prop_p_next(axis);

    if (st_data != NULL){

        /* go back to middle of exposure and re-propagate */
        int prop_from_index = (get_st_exp() * 1000) / (2 * GYRO_SAMPLE_TIME);

        for(int ii=0; ii<X_PREV_ROWS; ++ii){
            axis.x_next[ii][0] = axis.x_hist[prop_from_index][ii];
        }

        for(int ii=0; ii<P_PREV_ROWS; ++ii){
            for(int jj=0; jj<P_PREV_COLS; ++jj){
                axis.P_next[ii][jj] = axis.p_hist[prop_from_index][ii][jj];
            }
        }

        // innovation
        innovate_nu_next(axis, st_data);
        update_s_next(axis);

        //          log(nu_next);
        //          log(S_next);
        logging_csv(axis.nu_log, "%+.10e", **axis.nu_next);
        logging_csv(axis.s_log, "%+.10e", **axis.S_next);

        #ifdef KF_DEBUG
            logging(DEBUG, "Kalman F", "nu_next:");
            logging(DEBUG, "Kalman F", "%+.6e", axis.nu_next[0][0]);

            logging(DEBUG, "Kalman F", "S_next:");
            logging(DEBUG, "Kalman F", "%+.6e", axis.S_next[0][0]);
        #endif

        // compute the Kalman gain
        comp_k_gain(axis);

        // update of states and covariance
        update_state(axis);
        update_covar(axis);

        // save estimates (and log)
        //x_prev = x_upd;
        for(int i = 0; i < X_PREV_ROWS; i++) {
            for(int j = 0; j < X_PREV_COLS; j++) {
                axis.x_prev[i][j] = axis.x_upd[i][j];
            }
        }

        // propagate with initialisation measurement
        //P_prev = P_upd;
        for(int i = 0; i < P_PREV_ROWS; i++) {
            for(int j = 0; j < P_PREV_COLS; j++) {
                axis.P_prev[i][j] = axis.P_upd[i][j];
            }
        }
        //          log(x_prev);
        //          log(P_prev);
        logging_csv(axis.x_log, "%+.10e,%+.10e", axis.x_prev[0][0], axis.x_prev[1][0]);

        logging_csv(axis.p_log, "%+.10e,%+.10e,%+.10e,%+.10e",
            axis.P_prev[0][0], axis.P_prev[0][1], axis.P_prev[1][0], axis.P_prev[1][1]);

        /* re-propagation loop */
        for(int ii = prop_from_index; ii < hist_index; ++ii){

            /* load gyro measurements */
            axis.w_meas[0][0] = axis.gyro_hist[ii];

            prop_state_vec(axis);
            prop_p_next(axis);

            //x_prev = x_next;
            for(int i = 0; i < X_PREV_ROWS; i++) {
                for(int j = 0; j < X_PREV_COLS; j++) {
                    axis.x_prev[i][j] = axis.x_next[i][j];
                }
            }
            //P_prev = P_next;
            for(int i = 0; i < P_PREV_ROWS; i++) {
                for(int j = 0; j < P_PREV_COLS; j++) {
                    axis.P_prev[i][j] = axis.P_next[i][j];
                }
            }
            //          log(x_prev);
            //          log(P_prev);
            logging_csv(axis.x_log, "%+.10e,%+.10e", axis.x_prev[0][0], axis.x_prev[1][0]);

            logging_csv(axis.p_log, "%+.10e,%+.10e,%+.10e,%+.10e",
                axis.P_prev[0][0], axis.P_prev[0][1], axis.P_prev[1][0], axis.P_prev[1][1]);
        }
    }
    // new ST data not available
    else {

        /* TODO: if we dont get any ST measurements for 10 min
         *       call set_mode(RESET);
         *       and send message as telemetry & log
         */

        //x_prev = x_next;
        for(int i = 0; i < X_PREV_ROWS; i++) {
            for(int j = 0; j < X_PREV_COLS; j++) {
                axis.x_prev[i][j] = axis.x_next[i][j];
            }
        }
        //P_prev = P_next;
        for(int i = 0; i < P_PREV_ROWS; i++) {
            for(int j = 0; j < P_PREV_COLS; j++) {
                axis.P_prev[i][j] = axis.P_next[i][j];
            }
        }

        // save estimates
        //          log(x_next);
        //          log(P_next);
        logging_csv(axis.x_prop_log, "%+.10e,%+.10e", axis.x_prev[0][0], axis.x_prev[1][0]);

        logging_csv(axis.p_prop_log, "%+.10e,%+.10e,%+.10e,%+.10e",
                axis.P_prev[0][0], axis.P_prev[0][1], axis.P_prev[1][0], axis.P_prev[1][1]);

        /* Save gyro data, estimated state, and P matrix in history */
        axis.gyro_hist[hist_index] = gyro_data;

        for(int ii=0; ii<X_PREV_ROWS; ++ii){
            axis.x_hist[hist_index][ii] = axis.x_prev[ii][0];
        }

        for(int ii=0; ii<P_PREV_ROWS; ++ii){
            for(int jj=0; jj<P_PREV_COLS; ++jj){
                axis.p_hist[hist_index][ii][jj] = axis.P_prev[ii][jj];
            }
        }

        //          printf("x_prev for i = %d", l);
        //          printarray2D(x_prev, X_PREV_ROWS, X_PREV_COLS);
        // x_next = Phi * x_prev + Gamma * w_meas;
        #ifdef KF_DEBUG
            logging(DEBUG, "Kalman F", "step number: %d", l);
            logging(DEBUG, "Kalman F", "x_prev:");
            logging(DEBUG, "Kalman F", "%+.6e", axis.x_prev[0][0]);
            logging(DEBUG, "Kalman F", "%+.6e", axis.x_prev[1][0]);

            logging(DEBUG, "Kalman F", "P_prev:");
            logging(DEBUG, "Kalman F", "%+.6e\t%+.6e",
                    axis.P_prev[0][0], axis.P_prev[0][1]);
            logging(DEBUG, "Kalman F", "%+.6e\t%+.6e",
                    axis.P_prev[1][0], axis.P_prev[1][1]);
        #endif

    }

    return SUCCESS;
}

/*******************************************************************************
********************************************************************************
************************************KF**FUNCS***********************************
********************************************************************************
*******************************************************************************/


/* x_next = Phi * x_prev + Gamma * w_meas; */
static void prop_state_vec(axis_context_t axis){

    // definition of mmult1[PHI_ROWS][X_PREV_COLS]
    double **mmult1;
    int mmult1_rows = PHI_ROWS;
    int mmult1_cols = X_PREV_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.Phi, axis.x_prev, mmult1, PHI_ROWS, PHI_COLS, X_PREV_ROWS, X_PREV_COLS);

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
    mmult(axis.Gamma, axis.w_meas, mmult2, GAMMA_ROWS, GAMMA_COLS, W_MEAS_ROWS, W_MEAS_COLS);

    madd(mmult1, mmult2, axis.x_next, mmult1_rows, mmult1_cols);
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
static void prop_p_next(axis_context_t axis){

    // propagate P_next
    // re-definition of mmult1[length(Phi)][length(P_prev[1])];
    double** mmult1;
    int mmult1_rows = PHI_ROWS;
    int mmult1_cols = P_PREV_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.Phi, axis.P_prev, mmult1, PHI_ROWS, PHI_COLS, P_PREV_ROWS, P_PREV_COLS);
    // definition of trans[length(Phi[0])][length(Phi)];
    double **trans;
    int trans_rows = PHI_COLS;
    int trans_cols = PHI_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(axis.Phi, trans, PHI_ROWS, PHI_COLS);

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
    mmult(axis.Upsilon, axis.Q, mmult1, UPSILON_ROWS, UPSILON_COLS, Q_ROWS, Q_COLS);
    // re-definition of trans[length(Upsilon[0])][length(Upsilon)];
    trans_rows = UPSILON_COLS;
    trans_cols = UPSILON_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(axis.Upsilon, trans, UPSILON_ROWS, UPSILON_COLS);

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
    mmult(axis.Upsilon2, axis.Q2, mmult1, UPSILON2_ROWS, UPSILON2_COLS, Q2_ROWS, Q2_COLS);

    // re-definition of trans[length(Upsilon2[0])][length(Upsilon2)];
    trans_rows = UPSILON2_COLS;
    trans_cols = UPSILON2_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(axis.Upsilon2, trans, UPSILON2_ROWS, UPSILON2_COLS);

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

    madd(madd1, mmult4, axis.P_next, madd1_rows, madd1_cols);


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
static void innovate_nu_next(axis_context_t axis, double* st_data){

    // re-definition of mmult1[length(H)][length(x_next[1])];
    double** mmult1;
    int mmult1_rows = H_ROWS;
    int mmult1_cols = X_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.H, axis.x_next, mmult1, H_ROWS, H_COLS, X_NEXT_ROWS, X_NEXT_COLS);
    // definition of ang as pointer
    double **ang;
    int ang_rows = 1;
    int ang_cols = 1;

    ang = malloc(ang_rows * sizeof *ang);
    for (int i = 0; i < ang_rows; i++) {
        ang[i] = malloc(ang_cols * sizeof *ang[i]);
    }
    #ifdef KF_TEST
        ang[0][0] = ang_init;
    #else
        ang[0][0] = *st_data;
    #endif

    msub(ang, mmult1, axis.nu_next, ang_rows, ang_cols);

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
static void update_s_next(axis_context_t axis){

    // re-definition of mmult1[length(H)][length(P_next[1])];
    double** mmult1;
    int mmult1_rows = H_ROWS;
    int mmult1_cols = P_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.H, axis.P_next, mmult1, H_ROWS, H_COLS, P_NEXT_ROWS, P_NEXT_COLS);

    // definition of trans[length(H[0])][length(H)];
    double** trans;
    int trans_rows = H_COLS;
    int trans_cols = H_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(axis.H, trans, H_ROWS, H_COLS);

    // re-definition of mmult2[length(mmult1)][length(H)];
    double** mmult2;
    int mmult2_rows = mmult1_rows;
    int mmult2_cols = trans_cols;

    mmult2 = malloc(mmult2_rows * sizeof *mmult2);
    for (int i = 0; i < mmult2_rows; i++) {
        mmult2[i] = malloc(mmult2_cols * sizeof *mmult2[i]);
    }
    mmult(mmult1, trans, mmult2, mmult1_rows, mmult1_cols, trans_rows, trans_cols);

    madd(mmult2, axis.R, axis.S_next, mmult2_rows, mmult2_cols);
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
static void comp_k_gain(axis_context_t axis){
    // re-definition of trans[length(H[0])][length(H)];
    double** trans;
    int trans_rows = H_COLS;
    int trans_cols = H_ROWS;

    trans = malloc(trans_rows * sizeof *trans);
    for (int i = 0; i < trans_rows; i++) {
        trans[i] = malloc(trans_cols * sizeof *trans[i]);
    }
    transpose(axis.H, trans, H_ROWS, H_COLS);

    // re-definition of mmult1[length(P_next)][length(H)];
    double** mmult1;
    int mmult1_rows = P_NEXT_ROWS;
    int mmult1_cols = trans_cols;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.P_next, trans, mmult1, P_NEXT_ROWS, P_NEXT_COLS, trans_rows, trans_cols);
    mdiv(mmult1, axis.S_next[0][0], axis.K, mmult1_rows, mmult1_cols);


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
static void update_state(axis_context_t axis){
    //double mmult1[length(K)][length(nu_next[1])];
    // re-definition of mmult1[length(K)][length(nu_next[1])];
    double** mmult1;
    int mmult1_rows = K_ROWS;
    int mmult1_cols = NU_NEXT_COLS;

    mmult1 = malloc(mmult1_rows * sizeof *mmult1);
    for (int i = 0; i < mmult1_rows; i++) {
        mmult1[i] = malloc(mmult1_cols * sizeof *mmult1[i]);
    }
    mmult(axis.K, axis.nu_next, mmult1, K_ROWS, K_COLS, NU_NEXT_ROWS, NU_NEXT_COLS);
    madd(axis.x_next, mmult1, axis.x_upd, X_NEXT_ROWS, X_NEXT_COLS);

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
static void update_covar(axis_context_t axis){
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
    mmult(axis.K, axis.H, mmult1, K_ROWS, K_COLS, H_ROWS, H_COLS);
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
    mmult(msub1, axis.P_next, axis.P_upd, msub1_rows, msub1_cols, P_NEXT_ROWS, P_NEXT_COLS);

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

#if 0
static void printarray2D(double** array, int rows, int cols) {
    printf("\nPrint 2D array\n");
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            printf(" %+.11e",array[i][j]);
        }
        printf("\n");
    }
}
#endif

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
