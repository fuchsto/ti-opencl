EXE       = null
CXXFLAGS = -O3

include ../make.inc

$(EXE): main.o 
	@$(CXX) $(CXXFLAGS) main.o $(LDFLAGS) $(LIBS) -lrt -o $@
