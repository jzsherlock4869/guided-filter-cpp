# Guided Filter (C++ version)

implementation of Guided Filter [Kaiming He, ECCV10] using C++

Currently support guide image with 1 or 3 channels (uint8 type), and input image with any channels (uint8 type).

Try start by running the `run.sh` file, and the results are saved in `./build/result_dir`. You can check the input and output images for differences.

parameters in `main_test.cpp`

```
int radius = 3;
float epsilon = 0.1;
```

can be tuned to check the influence to the output result.

Example result:

![input](./assets/inputImage.jpg) ![output](./assets/outputImage.jpg)