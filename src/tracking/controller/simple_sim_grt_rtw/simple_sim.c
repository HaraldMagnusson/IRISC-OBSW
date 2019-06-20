/*
 * simple_sim.c
 *
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * Code generation for model "simple_sim".
 *
 * Model version              : 1.15
 * Simulink Coder version : 9.1 (R2019a) 23-Nov-2018
 * C source code generated on : Mon Jun 17 21:25:05 2019
 *
 * Target selection: grt.tlc
 * Note: GRT includes extra infrastructure and instrumentation for prototyping
 * Embedded hardware selection: Intel->x86-64 (Windows64)
 * Code generation objective: Debugging
 * Validation result: Not run
 */

#include "simple_sim.h"
#include "simple_sim_private.h"

/* External inputs (root inport signals with default storage) */
ExtU_simple_sim_T simple_sim_U;

/* External outputs (root outports fed by signals with default storage) */
ExtY_simple_sim_T simple_sim_Y;

/* Real-time model */
RT_MODEL_simple_sim_T simple_sim_M_;
RT_MODEL_simple_sim_T *const simple_sim_M = &simple_sim_M_;

/* Model step function */
void simple_sim_step(void)
{
  /* Outport: '<Root>/Out1' incorporates:
   *  Gain: '<S1>/Multiply'
   *  Inport: '<Root>/In1'
   */
  simple_sim_Y.Out1 = simple_sim_U.In1 << 1;

  /* Matfile logging */
  rt_UpdateTXYLogVars(simple_sim_M->rtwLogInfo, (&simple_sim_M->Timing.taskTime0));

  /* signal main to stop simulation */
  {                                    /* Sample time: [0.2s, 0.0s] */
    if ((rtmGetTFinal(simple_sim_M)!=-1) &&
        !((rtmGetTFinal(simple_sim_M)-simple_sim_M->Timing.taskTime0) >
          simple_sim_M->Timing.taskTime0 * (DBL_EPSILON))) {
      rtmSetErrorStatus(simple_sim_M, "Simulation finished");
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   * Timer of this task consists of two 32 bit unsigned integers.
   * The two integers represent the low bits Timing.clockTick0 and the high bits
   * Timing.clockTickH0. When the low bit overflows to 0, the high bits increment.
   */
  if (!(++simple_sim_M->Timing.clockTick0)) {
    ++simple_sim_M->Timing.clockTickH0;
  }

  simple_sim_M->Timing.taskTime0 = simple_sim_M->Timing.clockTick0 *
    simple_sim_M->Timing.stepSize0 + simple_sim_M->Timing.clockTickH0 *
    simple_sim_M->Timing.stepSize0 * 4294967296.0;
}

/* Model initialize function */
void simple_sim_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* initialize real-time model */
  (void) memset((void *)simple_sim_M, 0,
                sizeof(RT_MODEL_simple_sim_T));
  rtmSetTFinal(simple_sim_M, 10.0);
  simple_sim_M->Timing.stepSize0 = 0.2;

  /* Setup for data logging */
  {
    static RTWLogInfo rt_DataLoggingInfo;
    rt_DataLoggingInfo.loggingInterval = NULL;
    simple_sim_M->rtwLogInfo = &rt_DataLoggingInfo;
  }

  /* Setup for data logging */
  {
    rtliSetLogXSignalInfo(simple_sim_M->rtwLogInfo, (NULL));
    rtliSetLogXSignalPtrs(simple_sim_M->rtwLogInfo, (NULL));
    rtliSetLogT(simple_sim_M->rtwLogInfo, "tout");
    rtliSetLogX(simple_sim_M->rtwLogInfo, "");
    rtliSetLogXFinal(simple_sim_M->rtwLogInfo, "");
    rtliSetLogVarNameModifier(simple_sim_M->rtwLogInfo, "rt_");
    rtliSetLogFormat(simple_sim_M->rtwLogInfo, 4);
    rtliSetLogMaxRows(simple_sim_M->rtwLogInfo, 0);
    rtliSetLogDecimation(simple_sim_M->rtwLogInfo, 1);
    rtliSetLogY(simple_sim_M->rtwLogInfo, "");
    rtliSetLogYSignalInfo(simple_sim_M->rtwLogInfo, (NULL));
    rtliSetLogYSignalPtrs(simple_sim_M->rtwLogInfo, (NULL));
  }

  /* external inputs */
  simple_sim_U.In1 = 0U;

  /* external outputs */
  simple_sim_Y.Out1 = 0U;

  /* Matfile logging */
  rt_StartDataLoggingWithStartTime(simple_sim_M->rtwLogInfo, 0.0, rtmGetTFinal
    (simple_sim_M), simple_sim_M->Timing.stepSize0, (&rtmGetErrorStatus
    (simple_sim_M)));
}

/* Model terminate function */
void simple_sim_terminate(void)
{
  /* (no terminate code required) */
}
