#pragma OPENCL EXTENSION cl_khr_fp64: enable

unsigned int rbs(uint x, uint log2N)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    x = ((x >> 16) | (x << 16));
    x = x >> (32 - log2N);
    return x;
}

__kernel void
getSinArrayTwoPi_F(__global float *sins)
{
    int i = get_global_id(0);
    int sinLength = get_global_size(0);
    sinLength >>= 1;
    float sn = sin(M_PI * (((float)i+0.5)) / (1<<18));
    if (i == 0 || i == sinLength)
        sn = 0;
    sins[i] = sn;
}

float2
mulComplex_F(float2 x, float2 y)
{
    float txs0 = x.s0;
    x.s0 = txs0*y.s0 - x.s1*y.s1;
    x.s1 = txs0*y.s1 + x.s1*y.s0;
    return x;
}

float2
mulComplexConj_F(float2 x, float2 y)
{
    float txs0 = x.s0;
    x.s0 = txs0*y.s0 + x.s1*y.s1;
    x.s1 = -txs0*y.s1 + x.s1*y.s0;
    return x;
}

__kernel void
signPrep_F(
    __global int *inSignBuffer,
    __global float2 *outSignBuffer,
    const uint log2N,
    const int nihs)
{
    uint i = get_global_id(0);
    uint ind = rbs(i, log2N);
    int arg = 0.;
    if (i < nihs)
        arg = inSignBuffer[i];
    outSignBuffer[ind].s0 = arg;
    outSignBuffer[ind].s1 = 0.;
}

__kernel void
signFFT_F(
    __global float2 *signArray,
    __global float *sins,
    const uint log2N,
    const uint groupLog2)
{
    int thread = get_local_id(0);  // groupsize
    int iner = 1 << (log2N - groupLog2 - 1);

    for (int i = 0; i < log2N; i++) {
        int add = 1 << i;
        for (int j = 0; j < iner; j++) {
            int ji = thread + (j << groupLog2);
            int div = ji >> i;
            int rem = ji & (add-1);
            int ind = (div << (i+1)) + rem;
            int arg_i = (1<<(18-i)) * rem;
            int arg_r = (arg_i + (1<<17)) & ((1<<19) - 1);
            float2 W;
            W.s0 = sins[arg_r];
            W.s1 = -sins[arg_i];

            float2 sign0 = signArray[ind];
            float2 sign1 = signArray[ind + add];
            sign1 = mulComplex_F(sign1, W);

            signArray[ind]       = sign0 + sign1;
            signArray[ind + add] = sign0 - sign1;

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}

__kernel void
dataPrepSample_F(
    __global int2 *inFFTBuffer,
    __global float2 *outFFTBuffer,
    const uint log2N,
    const int dlstr_ndec,
    const int sampleNum)
{
    uint i = get_global_id(0);
    uint nl = get_global_id(1);
    uint N = 1 << log2N;

    uint ind = rbs(i, log2N);
    float2 arg = (float2)(0., 0.);
    if (i < dlstr_ndec) {
        arg.s0 = (float)inFFTBuffer[nl*sampleNum + i].s0;
        arg.s1 = (float)inFFTBuffer[nl*sampleNum + i].s1;
    }
    outFFTBuffer[nl*N + ind].s0 = arg.s0;
    outFFTBuffer[nl*N + ind].s1 = arg.s1;
}

__kernel void
dataFFT_F(
    __global float2 *FFTArray,
    __global float *sins,
    const uint log2N,
    const uint groupLog2)
{
    uint thread = get_local_id(0);
    uint nl = get_global_id(1);
    uint N = 1 << log2N;

    int iner = 1 << (log2N - groupLog2 - 1);

    for (int i = 0; i < log2N; i++) {
        int add = 1 << i;
        for (int j = 0; j < iner; j++) {
            int ji = thread + (j << groupLog2);
            int div = ji >> i;
            int rem = ji & (add-1);
            int ind = (div << (i+1)) + rem;

            int arg_i = (1<<(18-i)) * rem;
            int arg_r = (arg_i + (1<<17)) & ((1<<19) - 1);
            float2 W;
            W.s0 = sins[arg_r];
            W.s1 = -sins[arg_i];

            float2 data0 = FFTArray[nl*N + ind];
            float2 data1 = FFTArray[nl*N + ind + add];
            data1 = mulComplex_F(data1, W);

            FFTArray[nl*N + ind]       = data0 + data1;
            FFTArray[nl*N + ind + add] = data0 - data1;

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}

__kernel void
IFFTPrep_F(
    __global float2 *outFFTArray,
    __global float2 *outSignArray,
    __global float2 *midIFFTArray,
    const int log2N,
    const int n1grs,
    const int N2nihs)
{
    uint i = get_global_id(0); //N
    uint v = get_global_id(1); // kgrs
    uint nl = get_global_id(2); // Nl
    uint N = get_global_size(0);
    uint V = get_global_size(1); // kgrs

    int ind = (i + ((v + n1grs) * N2nihs)) & (N-1);
    midIFFTArray[nl*V*N + v*N + i] =
        mulComplexConj_F(outFFTArray[nl*N + ind], outSignArray[i]);
}


__kernel void
IFFT_FFT_F(
    __global float2 *X,
    __global float *sins,
    const uint log2N,
    const uint groupLog2)
{
    int thread = get_local_id(0);  // groupsize
    int v = get_global_id(1);  //kgrs
    int nl = get_global_id(2);

    int V = get_global_size(1);
    int N = 1 << log2N;

    int iner = 1 << (log2N - groupLog2 - 1);

    for (int i = 0; i < log2N; i++) {
        int add = 1 << (log2N - i - 1);
        for (int j = 0; j < iner; j++) {
            int ji = thread + (j << groupLog2);
            int div = ji >> (log2N - i - 1);
            int rem = ji & (add-1);
            int ind = (div << (log2N - i)) + rem;

            int arg_i = (1<<(18-(log2N-i-1))) * rem;
            int arg_r = (arg_i + (1<<17)) & ((1<<19) - 1);
            float2 W;
            W.s0 = sins[arg_r];
            W.s1 = sins[arg_i];

            float2 x0 = X[nl*V*N + v*N + ind];
            float2 x1 = X[nl*V*N + v*N + ind + add];
            float2 xSub = x0 - x1;
            xSub = mulComplex_F(xSub, W);

            X[nl*V*N + v*N + ind] = x0 + x1;
            X[nl*V*N + v*N + ind + add] = xSub;

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}

__kernel void
IFFTPost_F(
    __global float2 *midIFFTArray,
    __global float2 *outIFFTArray,
    const uint log2N,
    const int nfgd,
    const int shgd,
    const int ndec)
{
    uint d = get_global_id(0); // kgd
    uint v = get_global_id(1); // kgrs
    uint nl = get_global_id(2);

    uint D = get_global_size(0); // kgd
    uint V = get_global_size(1); // kgrs

    uint N = 1 << log2N;

    uint idx = (d + nfgd) * shgd / ndec;
    idx &= (N-1);
    idx = rbs(idx, log2N);
    outIFFTArray[nl*D*V + d*V + v] =
        midIFFTArray[nl*V*N + v*N + idx] / (float)N;
}

__kernel void
IFFTPrepsepNl_F(
    __global float2 *outFFTArray,
    __global float2 *outSignArray,
    __global float2 *midIFFTArray,
    const int log2N,
    const int n1grs,
    const int N2nihs,
    const int nl)
{
    uint i = get_global_id(0); //N
    uint v = get_global_id(1); // kgrs
    uint N = get_global_size(0);
    uint V = get_global_size(1); // kgrs

    int ind = (i + ((v + n1grs) * N2nihs)) & (N-1);
    midIFFTArray[v*N + i] =
        mulComplexConj_F(outFFTArray[nl*N + ind], outSignArray[i]);
}

__kernel void
IFFT_FFTsepNl_F(
    __global float2 *X,
    __global float *sins,
    const uint log2N,
    const uint groupLog2)
{
    int thread = get_local_id(0);  // groupsize
    int v = get_global_id(1);  //kgrs

    int V = get_global_size(1);
    int N = 1 << log2N;

    int iner = 1 << (log2N - groupLog2 - 1);

    for (int i = 0; i < log2N; i++) {
        int add = 1 << (log2N - i - 1);
        for (int j = 0; j < iner; j++) {
            int ji = thread + (j << groupLog2);
            int div = ji >> (log2N - i - 1);
            int rem = ji & (add-1);
            int ind = (div << (log2N - i)) + rem;

            int arg_i = (1<<(18-(log2N-i-1))) * rem;
            int arg_r = (arg_i + (1<<17)) & ((1<<19) - 1);
            float2 W;
            W.s0 = sins[arg_r];
            W.s1 = sins[arg_i];

            float2 x0 = X[v*N + ind];
            float2 x1 = X[v*N + ind + add];

            float2 xSub = x0 - x1;
            xSub = mulComplex_F(xSub, W);

            X[v*N + ind] = x0 + x1;
            X[v*N + ind + add] = xSub;

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}

__kernel void
IFFTPostsepNl_F(
    __global float2 *midIFFTArray,
    __global float2 *outIFFTArray,
    const uint log2N,
    const int nfgd,
    const int shgd,
    const int ndec,
    const int nl)
{
    uint d = get_global_id(0); // kgd
    uint v = get_global_id(1); // kgrs
    uint D = get_global_size(0); // kgd
    uint V = get_global_size(1); // kgrs

    uint N = 1 << log2N;

    uint idx = (d + nfgd) * shgd / ndec;
    idx &= (N-1);
    idx = rbs(idx, log2N);
    outIFFTArray[nl*D*V + d*V + v] = midIFFTArray[v*N + idx] / (float)N;
}


__kernel void
FFTPrep_F(
    __global int2 *inBuffer,
    __global float2 *midBuffer,
    const int log2N,
    const int sampleNum,
    const int nihs,
    const int nfgd,
    const int shgd,
    const int ndec,
    const int dlstr)
    // const uint log2N,
    // const uint sampleNum,
    // const uint nihs,
    // const int nfgd,
    // const uint shgd,
    // const uint ndec,
    // const uint dlstr)
{
    int i = get_global_id(0); // N
    int d = get_global_id(1); // kgd
    int nl = get_global_id(2);

    int N = get_global_size(0); //N
    int D = get_global_size(1);

    uint ind = rbs(i, log2N);
    if (nfgd + d < 0) {
        int nihs_t = nihs + (d + nfgd) * shgd / ndec;

        if (i < nihs_t) {
            midBuffer[nl*D*N + d*N + ind].s0 =
                (float)inBuffer[nl*sampleNum + i].s0;
            midBuffer[nl*D*N + d*N + ind].s1 =
                (float)inBuffer[nl*sampleNum + i].s1;
        }
        else {
            midBuffer[nl*D*N + d*N + ind].s0 = 0.;
            midBuffer[nl*D*N + d*N + ind].s1 = 0.;
        }
    }
    else {
        if (i < nihs) {
            int indAdd = (d + nfgd) * shgd / ndec;
            if (i + (d + nfgd) * shgd / ndec >= dlstr / ndec) {
                midBuffer[nl*D*N + d*N + ind].s0 = 0.;
                midBuffer[nl*D*N + d*N + ind].s1 = 0.;
            }
            else {

                midBuffer[nl*D*N + d*N + ind].s0 =
                    (float)inBuffer[nl*sampleNum + i + indAdd].s0;
                midBuffer[nl*D*N + d*N + ind].s1 =
                    (float)inBuffer[nl*sampleNum + i + indAdd].s1;
            }
        }
        else {
            midBuffer[nl*D*N + d*N + ind].s0 = 0.;
            midBuffer[nl*D*N + d*N + ind].s1 = 0.;
        }
    }
}

__kernel void
FFT_FFT_F(
    __global float2 *midArray,
    __global float *sins,
    const uint log2N,
    const uint groupLog2)
{
    int thread = get_local_id(0);  // groupsize
    int d = get_global_id(1);  //kgd
    int nl = get_global_id(2);

    int D = get_global_size(1);
    int N = 1 << log2N;

    int iner = 1 << (log2N - groupLog2 - 1);
    for (int i = 0; i < log2N; i++) {
        int add = 1 << i;
        for (int j = 0; j < iner; j++) {
            int ji = thread + (j << groupLog2);
            int div = ji >> i;
            int rem = ji & (add-1);
            int ind = (div << (i+1)) + rem;

            int arg_i = (1 << (18 - i)) * rem; // arg_i = rem << 18-i
            int arg_r = (arg_i + (1<<17)) & ((1<<19) - 1);
            float2 W;
            W.s0 = sins[arg_r];
            W.s1 = -sins[arg_i];
// if(thread == 0)
// {
//     printf("thread #0, [i,j]=[%d,%d] rem=%d, argr=%d, argi=%d\n", i, j, rem, arg_r, arg_i);
// }
// if(d == 0 && i == 0 && j == 0)
// {
//     printf("iner=%d, groupLog2=%d, log2N=%d\n", iner, groupLog2, log2N);
// }
            float2 mid0 = midArray[nl*D*N + d*N + ind];
            float2 mid1 = midArray[nl*D*N + d*N + ind + add];
            mid1 = mulComplex_F(mid1, W);

            midArray[nl*D*N + d*N + ind]       = mid0 + mid1;
            midArray[nl*D*N + d*N + ind + add] = mid0 - mid1;

            barrier(CLK_GLOBAL_MEM_FENCE);
        }
    }
}

__kernel void
FFTPost_F(
    __global float2 *midFFTArray,
    __global float2 *outFFTArray,
    const uint log2N,
    const int n1grs)
{
    uint f = get_global_id(0); // kgrs
    uint d = get_global_id(1); // kgd
    uint nl = get_global_id(2);
    uint F = get_global_size(0); // kgrs
    uint D = get_global_size(1); // kgd

    uint N = 1 << log2N;

    uint idx = f + n1grs;
    idx &= (N-1);
// if(nl==0&&d==0)
//     printf("f=%d, idx=%d, log2N=%d\n", f, idx, log2N);
    outFFTArray[nl*D*F + d*F + f] = midFFTArray[nl*D*N + d*N + idx];
}
