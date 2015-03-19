#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>
#include <iostream>
#include <cstdlib>

using namespace cl;
using std::cout;
using std::cerr;
using std::endl;

void report_event_timing(const Event &ev, const char* name);

const char * kernelStr =
    "kernel void VectorAdd(global const short* a, "
    "                      global const short* b, "
    "                      global short* c) "
    "{"
    "    int id = get_global_id(0);"
    "    c[id] = a[id] + b[id];"
    "}"

    "kernel void VectorAddTask(global const short* a, "
    "                      global const short* b, "
    "                      global short* c,       "
    "                      int N) "
    "{"
    "    int i; "
    "    for (i=N; i>=0; i--)"
    "        c[i] = a[i] + b[i];"
    "}"

    "void c_vecadd (global const short* a, global const short* b, "
    "               global short*c, int N); "

    "kernel void VectorAddFacade(global const short* a, "
    "                      global const short* b, "
    "                      global short* c,       "
    "                      int N) "
    "{"
    "    c_vecadd(a,b,c,N); "
    "}"
    ;

const int NumElements     = 1024*1024;
const int NumWorkGroups   = 1; 
const int VectorElements  = 1;
const int NumVecElements  = NumElements / VectorElements;
const int WorkGroupSize   = NumVecElements / NumWorkGroups;

cl_short srcA  [NumElements];
cl_short srcB  [NumElements];
cl_short dst   [NumElements];
cl_short Golden[NumElements];

int main(int argc, char *argv[])
{
   cl_int err     = CL_SUCCESS;
   int    bufsize = sizeof(srcA);

   for (int i=0; i < NumElements; ++i) 
   { 
       srcA[i]   = srcB[i] = i; 
       Golden[i] = srcB[i] + srcA[i]; 
       dst[i]    = 0;
   }

   try 
   {
     Context             context(CL_DEVICE_TYPE_ACCELERATOR); 
     std::vector<Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();

     Buffer bufA   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufB   (context, CL_MEM_READ_ONLY,  bufsize);
     Buffer bufDst (context, CL_MEM_WRITE_ONLY, bufsize);

     Program::Sources    source(1, std::make_pair(kernelStr,strlen(kernelStr)));
     Program             program = Program(context, source);
     program.build(devices, "stdc_vecadd.obj"); 

     Kernel kernel(program, "VectorAdd");
     kernel.setArg(0, bufA);
     kernel.setArg(1, bufB);
     kernel.setArg(2, bufDst);

     Kernel tkernel(program, "VectorAddTask");
     tkernel.setArg(0, bufA);
     tkernel.setArg(1, bufB);
     tkernel.setArg(2, bufDst);
     tkernel.setArg(3, NumElements);

     Kernel fkernel(program, "VectorAddFacade");
     fkernel.setArg(0, bufA);
     fkernel.setArg(1, bufB);
     fkernel.setArg(2, bufDst);
     fkernel.setArg(3, NumElements);

     Event ev1,ev2,ev3,ev4, ev5, ev6;

     CommandQueue Q(context, devices[0], CL_QUEUE_PROFILING_ENABLE);

     Q.enqueueWriteBuffer(bufA, CL_TRUE, 0, bufsize, srcA, NULL, &ev1);
     Q.enqueueWriteBuffer(bufB, CL_TRUE, 0, bufsize, srcB, NULL, &ev2);
     Q.enqueueNDRangeKernel(kernel, NullRange, NDRange(NumVecElements), 
                            NDRange(WorkGroupSize), NULL, &ev3);
     Q.enqueueTask(tkernel, NULL, &ev5);
     Q.enqueueTask(fkernel, NULL, &ev6);
     Q.enqueueReadBuffer (bufDst, CL_TRUE, 0, bufsize, dst, NULL, &ev4);

     report_event_timing(ev1, "Write BufA ");
     report_event_timing(ev2, "Write BufB ");
     report_event_timing(ev3, "Kernel Exec");
     report_event_timing(ev5, "Task Exec");
     report_event_timing(ev6, "Facade Exec");
     report_event_timing(ev4, "Read BufDst");
   }
   catch (Error err) 
   { cerr << "ERROR: " << err.what() << "(" << err.err() << ")" << endl; }

   int failcnt = 10;
   for (int i=0; i < NumElements; ++i)
       if (Golden[i] != dst[i]) 
       { 
           cout << "Failed at Element " << i 
                << ": " << Golden[i] << " != "
                << dst[i] << endl; 
           if (--failcnt == 0) return 1; 
       }

   cout << "Success!" << endl; 
}

/******************************************************************************
* REPORT_EVENT_TIMING - Given an OpenCL Event, report to stdout the profiling
*    info associated with the event
******************************************************************************/
void report_event_timing(const Event &ev, const char* name)
{
     cl_ulong t_que, t_sub, t_strt, t_end;
     
     ev.getProfilingInfo(CL_PROFILING_COMMAND_QUEUED, &t_que);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_SUBMIT, &t_sub);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_START,  &t_strt);
     ev.getProfilingInfo(CL_PROFILING_COMMAND_END,    &t_end);

     if (!name) name = "";

     cout<< name << " : Queue  to Submit: " << t_sub-t_que  << " us" << endl;
     cout<< name << " : Submit to Start : " << t_strt-t_sub << " us" << endl;
     cout<< name << " : Start  to End   : " << t_end-t_strt << " us" << endl;
     cout<< endl;
}
