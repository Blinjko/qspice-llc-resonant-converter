// Automatically generated C++ file on Mon Sep  2 00:56:05 2024
//
// To build with Digital Mars C++ Compiler:
//
//    dmc -mn -WD pid_controller.cpp kernel32.lib

#include <malloc.h>

extern "C" __declspec(dllexport) int (*Display)(const char *format, ...) = 0; // works like printf()
extern "C" __declspec(dllexport) const double *DegreesC                  = 0; // pointer to current circuit temperature

union uData
{
   bool b;
   char c;
   unsigned char uc;
   short s;
   unsigned short us;
   int i;
   unsigned int ui;
   float f;
   double d;
   long long int i64;
   unsigned long long int ui64;
   char *str;
   unsigned char *bytes;
};

// int DllMain() must exist and return 1 for a process to load the .DLL
// See https://docs.microsoft.com/en-us/windows/win32/dlls/dllmain for more information.
int __stdcall DllMain(void *module, unsigned int reason, void *reserved) { return 1; }

void bzero(void *ptr, unsigned int count)
{
   unsigned char *first = (unsigned char *) ptr;
   unsigned char *last  = first + count;
   while(first < last)
      *first++ = '\0';
}

// #undef pin names lest they collide with names in any header file(s) you might include.
#undef setpt
#undef ctrl
#undef clk
#undef fb

#include <math.h>

struct sPID_CONTROLLER
{
  bool   clk_n1;
  bool   initialized;
  double error_n1;
  double fb_n1;
  double errorI_n1;
  double deriv_n1;
  double last_ctrl;
  double lastT;
};

extern "C" __declspec(dllexport) void pid_controller(struct sPID_CONTROLLER **opaque, double t, union uData *data)
{
  double  setpt = data[0].d; // input
  bool    clk   = data[1].b; // input
  double  fb    = data[2].d; // input
  double  Kp    = data[3].d; // input parameter
  double  Ki    = data[4].d; // input parameter
  double  Kd    = data[5].d; // input parameter
  double  Kv    = data[6].d; // input parameter
  bool    Itype = data[7].b; // input parameter
  double &ctrl  = data[8].d; // output

  if(!*opaque)
  {
    *opaque = (struct sPID_CONTROLLER *) malloc(sizeof(struct sPID_CONTROLLER));
    bzero(*opaque, sizeof(struct sPID_CONTROLLER));
    Display("pid_controller: Kp=%f, Ki=%f, Kd=%f, Kv=%f, Itype=%s\n",
            Kp, Ki, Kd, Kv, Itype ? "Trapezoidal" : "Rectangular");
  }
  struct sPID_CONTROLLER *inst = *opaque;

  const double u_min = -10.0; // corresponds to f_max = 200 kHz (f_0=100k, K_vco=-10k)
  const double u_max =  4.5;  // corresponds to f_min = 55 kHz
  const double Tf    = 100e-6; // ~3.2 kHz low-pass filter on D term

  if (clk && !inst->clk_n1)   // rising edge
  {
    double error = Kv * (setpt - fb);

    if (!inst->initialized)
    {
      inst->lastT       = t;
      inst->error_n1    = error;
      inst->fb_n1       = fb;
      inst->errorI_n1   = 0.0;
      inst->deriv_n1    = 0.0;
      inst->last_ctrl   = 0.0;
      inst->initialized = true;
    }
    else
    {
      double Ts = t - inst->lastT;
      inst->lastT = t;

      if (Ts > 1e-12)
      {
        double e_abs = fabs(error);
        double boost = 1.0 + 1.5 * ((e_abs > 1.5) ? 1.0 : (e_abs / 1.5)); // 1x steady-state -> 2.5x during transient
        double P = (Kp * boost) * error;

        double new_errorI;
        if (Itype)
          new_errorI = (error + inst->error_n1) * Ts * 0.5 + inst->errorI_n1;
        else
          new_errorI = error * Ts + inst->errorI_n1;

        // Filtered derivative on measurement (-fb): no setpoint kick, rejects ripple
        double D = (Tf * inst->deriv_n1 - Kd * Kv * (fb - inst->fb_n1)) / (Tf + Ts);

        double u_unsat = P + Ki * new_errorI + D;
        double u_sat = u_unsat;
        if (u_sat > u_max) u_sat = u_max;
        if (u_sat < u_min) u_sat = u_min;

        // Anti-windup: stop integrating when saturated in the error's direction
        bool sat_hi = (u_unsat > u_max) && (error > 0.0);
        bool sat_lo = (u_unsat < u_min) && (error < 0.0);
        if (!sat_hi && !sat_lo)
          inst->errorI_n1 = new_errorI;

        inst->deriv_n1  = D;
        inst->error_n1  = error;
        inst->fb_n1     = fb;
        inst->last_ctrl = u_sat;
      }
    }
  }

  ctrl = inst->last_ctrl;
  inst->clk_n1 = clk;
}

extern "C" __declspec(dllexport) void Destroy(struct sPID_CONTROLLER *inst)
{
   free(inst);
}
