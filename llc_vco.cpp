// Automatically generated C++ file on Tue Nov 24 14:30:33 2026
//
// To build with Digital Mars C++ Compiler:
//
//    dmc -mn -WD -o llc_vco.cpp kernel32.lib

#include <malloc.h>

extern "C" __declspec(dllexport) void (*Display)(const char *format, ...)       = 0; // works like printf()
extern "C" __declspec(dllexport) void (*EXIT)(const char *format, ...)          = 0; // print message like printf() but exit(0) afterward
extern "C" __declspec(dllexport) const double *DegreesC                         = 0; // pointer to current circuit temperature
extern "C" __declspec(dllexport) const int *StepNumber                          = 0; // pointer to current step number
extern "C" __declspec(dllexport) const int *NumberSteps                         = 0; // pointer to estimated number of steps
extern "C" __declspec(dllexport) const char* const *InstanceName                = 0; // pointer to address of instance name
extern "C" __declspec(dllexport) const char *QUX                                = 0; // path to QUX.exe
extern "C" __declspec(dllexport) const bool *ForKeeps                           = 0; // pointer to whether being evaluated non-hypothetically
extern "C" __declspec(dllexport) const bool *HoldICs                            = 0; // pointer to whether instance initial conditions are being held
extern "C" __declspec(dllexport) const void *GUI_HWND                           = 0; // pointer to Window handle of QUX.exe
extern "C" __declspec(dllexport) const double *CKTtime                          = 0;
extern "C" __declspec(dllexport) const double *CKTdelta                         = 0;
extern "C" __declspec(dllexport) const int *IntegrationOrder                    = 0;
extern "C" __declspec(dllexport) const char *InstallDirectory                   = 0;
extern "C" __declspec(dllexport) double (*EngAtof)(const char **string)         = 0;
extern "C" __declspec(dllexport) const char *(*BinaryFormat)(unsigned int data)                          = 0; // BinaryFormat(0x1C) returns "0b00011100"
extern "C" __declspec(dllexport) const char *(*EngFormat   )(double x, const char *units, int numDgts)   = 0; // EngFormat(1e-6, "s", 6) returns "1µs"
extern "C" __declspec(dllexport) int (*DFFT)(struct sComplex *u, bool inv, unsigned int N, double scale) = 0; // Discrete Fast Fourier Transform
extern "C" __declspec(dllexport) void (*bzero)(void *ptr, unsigned int count)   = 0;

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

// #undef pin names lest they collide with names in any header file(s) you might include.
#undef ctrl
#undef q1
#undef q2
#undef freq

enum states
{
  DEAD_A,
  TOP_ON,
  DEAD_B,
  BOTTOM_ON
};

struct sLLC_VCO
{
  enum states state;
  double freq;
  double on_time;
  double next_edge;
};

extern "C" __declspec(dllexport) void llc_vco(struct sLLC_VCO **opaque, double t, union uData *data)
{
   double  ctrl   = data[0].d; // input
   double  f_0    = data[1].d; // input parameter
   double  K_vco  = data[2].d; // input parameter
   double  f_min  = data[3].d; // input parameter
   double  f_max  = data[4].d; // input parameter
   double  t_dead = data[5].d; // input parameter
   double &q1     = data[6].d; // output
   double &q2     = data[7].d; // output
   double &freq   = data[8].d; // output

   if(!*opaque)
   {
      *opaque = (struct sLLC_VCO *) malloc(sizeof(struct sLLC_VCO));
      bzero(*opaque, sizeof(struct sLLC_VCO));
   }
   struct sLLC_VCO *inst = *opaque;

// Implement module evaluation code here:
  // State transition logic
  if(t >= inst->next_edge)
  {
    switch(inst->state)
    {
      case DEAD_A:
      {
        // Update frequency once per full cycle (guarantees 50% symmetry)
        double f = K_vco * ctrl + f_0;

        // bounds check
        if(f > f_max) f = f_max;
        if(f < f_min) f = f_min;
        inst->freq = f;

        inst->on_time = (0.5 / inst->freq) - t_dead;
        if(inst->on_time < 10e-9) inst->on_time = 10e-9; // bounds check

        inst->state       = TOP_ON;
        inst->next_edge = t + inst->on_time;
        break;
      }
      case TOP_ON:
        inst->state       = DEAD_B;
        inst->next_edge = t + t_dead;
        break;

      case DEAD_B:
        inst->state       = BOTTOM_ON;
        inst->next_edge = t + inst->on_time;
        break;

      case BOTTOM_ON:
        inst->state       = DEAD_A;
        inst->next_edge = t + t_dead;
        break;
    }
  }

  // Output driving logic
  switch(inst->state)
  {
    case TOP_ON:
      q1 = 5.0;
      q2 = 0.0;
      break;

    case BOTTOM_ON:
      q1 = 0.0;
      q2 = 5.0;
      break;

    case DEAD_A:
    case DEAD_B:
    default:
      q1 = 0.0;
      q2 = 0.0;
      break;
  }

  freq = inst->freq;

}

extern "C" __declspec(dllexport) double MaxExtStepSize(struct sLLC_VCO *inst, double t)
{
   return 1e308; // implement a good choice of max timestep size that depends on struct sLLC_VCO

   if (!inst) return 1e-7;

  double dt = inst->next_edge - t;

  if (dt > 5e-9) return dt - 2e-9; // try to land 2ns before next pulse
  return 2e-9;                     // Step over the pulse if we are within 2ns.
}

extern "C" __declspec(dllexport) void Trunc(struct sLLC_VCO *inst, double t, union uData *data, double *timestep)
{ // limit the timestep to a tolerance if the circuit causes a change in struct sLLC_VCO
   const double ttol = 1e-9; // 1ns default tolerance
   if(*timestep > ttol)
   {
      struct sLLC_VCO tmp = *inst;
      llc_vco(&(&tmp), t, data);

      if(tmp.state != inst->state) // check if the state changed.
         *timestep = ttol;
   }
}

extern "C" __declspec(dllexport) void Destroy(struct sLLC_VCO *inst)
{
   free(inst);
}
