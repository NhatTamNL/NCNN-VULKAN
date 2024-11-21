# Build for Linux
* Install required build dependencies:
```
sudo apt install build-essential git cmake libprotobuf-dev protobuf-compiler libomp-dev libvulkan-dev vulkan-tools libopencv-dev
```

* Git clone ncnn repo with submodule
```
git clone https://github.com/Tencent/ncnn.git
cd ncnn
git submodule update --init
```

```
cd ncnn
mkdir -p build_pc
cd build_pc
cmake -DCMAKE_BUILD_TYPE=Release -DNCNN_VULKAN=ON -DNCNN_BUILD_EXAMPLES=ON ..
make -j$(nproc)
make install
```

* Run app gpu net.opt.use_vulkan_compute = true;
```
[0 NVIDIA GeForce RTX 3060]  queueC=2[8]  queueG=0[16]  queueT=1[2]
[0 NVIDIA GeForce RTX 3060]  bugsbn1=0  bugbilz=0  bugcopc=0  bugihfa=0
[0 NVIDIA GeForce RTX 3060]  fp16-p/s/u/a=1/1/1/1  int8-p/s/u/a=1/1/1/1
[0 NVIDIA GeForce RTX 3060]  subgroup=32  basic/vote/ballot/shuffle=1/1/1/1
[0 NVIDIA GeForce RTX 3060]  fp16-8x8x16/16x8x8/16x8x16/16x16x16=0/1/1/1
[1 llvmpipe (LLVM 12.0.0, 256 bits)]  queueC=0[1]  queueG=0[1]  queueT=0[1]
[1 llvmpipe (LLVM 12.0.0, 256 bits)]  bugsbn1=0  bugbilz=0  bugcopc=0  bugihfa=0
[1 llvmpipe (LLVM 12.0.0, 256 bits)]  fp16-p/s/u/a=1/1/1/0  int8-p/s/u/a=1/1/1/0
[1 llvmpipe (LLVM 12.0.0, 256 bits)]  subgroup=8  basic/vote/ballot/shuffle=1/1/1/0
[1 llvmpipe (LLVM 12.0.0, 256 bits)]  fp16-8x8x16/16x8x8/16x8x16/16x16x16=0/0/0/0

output: 85, 22, 22
Time:   5.97 ms
x1:1152 y1:547 x2:1437 y2:1101  person:80.35%
x1:435 y1:283 x2:599 y2:472  potted plant:69.96%
x1:1486 y1:452 x2:1649 y2:738  person:65.72%
```

* Run app cpu net.opt.use_vulkan_compute = false;
```
output: 85, 22, 22
Time:   5.97 ms
x1:1152 y1:547 x2:1437 y2:1101  person:80.35%
x1:435 y1:283 x2:599 y2:472  potted plant:69.96%
x1:1486 y1:452 x2:1649 y2:738  person:65.72%
```

* Train 
```
python3 train.py --yaml my_config/person.yml
```

* Test and Export ONNX
```
python3 test.py --yaml configs/coco.yaml --weight checkpoint/... --img data/3.jpg --onnx
```
* Convert 
```
./bin/onnx2ncnn ../FastestDet.onnx FastestDet.param FastestDet.bin
./bin/ncnnoptimize FastestDet.param FastestDet.bin FastestDet-opt.param FastestDet-opt.bin 1

```

* Compile without Makefile
```
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
```