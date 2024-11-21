TARGET_NAME = FastestDet

OUTPUT_DIR = build

CXX = g++
CXXFLAGS = -ldl -fopenmp `pkg-config --libs --cflags opencv4`

SRC = FastestDet.cpp

INCLUDE_NCNN = -I include/ncnn
INCLUDE_GLS = -I include/glslang

LIBS = 	lib/libncnn.a \
		lib/libglslang.a \
    	lib/libMachineIndependent.a \
    	lib/libGenericCodeGen.a \
    	lib/libOGLCompiler.a \
    	lib/libOSDependent.a \
    	lib/libSPIRV.a \
    	lib/libglslang-default-resource-limits.a 

all: $(TARGET_NAME)
$(TARGET_NAME): $(SRC) 
	$(CXX) -o $(TARGET_NAME) $(SRC) $(INCLUDE_NCNN) $(INCLUDE_GLS) $(LIBS) $(CXXFLAGS)

	@rm -rf ${OUTPUT_DIR}
	@mkdir ${OUTPUT_DIR}
	@mv $(TARGET_NAME) ${OUTPUT_DIR}
	@echo "Build $(TARGET_NAME) successfully!"

clean:
	echo "Cleaning up..." 
	@rm -f $(TARGET_NAME)

