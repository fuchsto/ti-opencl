EXE        = dgemm
CXXFLAGS  = -O3 
BOTHCBLAS  = -Llibcblas_atlas_arm -lcblas -latlas -Llibcblas_dgemm_dsp -lcblas_dgemm_dsp -lOpenCL -locl_util -lpthread

include ../make.inc

dgemm: main.o libcblas_dgemm_dsp/libcblas_dgemm_dsp.a
	$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(BOTHCBLAS) -lrt -o $@

libcblas_dgemm_dsp/libcblas_dgemm_dsp.a:
	$(MAKE) -C libcblas_dgemm_dsp

clean::
	$(MAKE) -C libcblas_dgemm_dsp clean
	rm -f $(EXES) *.o .a* 

