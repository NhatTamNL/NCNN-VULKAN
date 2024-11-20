# g++ -o FastestDet FastestDet.cpp -I include/ncnn -I include/glslang lib/libncnn.a `pkg-config --libs --cflags opencv4` -fopenmp

g++ -o FastestDet FastestDet.cpp -I include/ncnn -I include/glslang \
    lib/libncnn.a \
    lib/libglslang.a \
    lib/libMachineIndependent.a \
    lib/libGenericCodeGen.a \
    lib/libOGLCompiler.a \
    lib/libOSDependent.a \
    lib/libSPIRV.a \
    lib/libglslang-default-resource-limits.a \
    -ldl -fopenmp `pkg-config --libs --cflags opencv4`

# g++ -o FastestDet FastestDet_org.cpp -I include/ncnn -I include/glslang \
#     lib/libncnn.a \
#     lib/libglslang.a \
#     lib/libMachineIndependent.a \
#     lib/libGenericCodeGen.a \
#     lib/libOGLCompiler.a \
#     lib/libOSDependent.a \
#     lib/libSPIRV.a \
#     lib/libglslang-default-resource-limits.a \
#     -ldl -fopenmp `pkg-config --libs --cflags opencv4`

