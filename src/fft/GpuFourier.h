#ifndef GPU_FOURIER_H__
#define GPU_FOURIER_H__

// #include <iostream>
#include <ProgramHandler.h>
#include <types.h>
#include <error.h>

// Deferred definition
class FftCreator;

class FftInterface
{
public:
    FftInterface( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );

    FftInterface(){}

    ~FftInterface();

    void update( const FftParams & newParams, cl_int2 * newDataArray );

    virtual TimeResult compute() = 0;

protected:
    void invariant();

    ProgramHandler * handler;
    FftParams params;
    cl_int2 * dataArray;
    cl_float2 * outArray;

    friend class FftCreator;
};

class AmFft: public FftInterface
{
public:
    AmFft( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );
    
    ~AmFft() = default;

    TimeResult compute();
};

class FmFft: public FftInterface
{
public:
    FmFft( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );

    ~FmFft() = default;

    TimeResult compute();
};

class FmFftSepNl: public FftInterface
{
public:
    FmFftSepNl( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );

    ~FmFftSepNl() = default;

    TimeResult compute();
};

class FftCreator
{
public:
    FftCreator() = delete;

    explicit FftCreator( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );

    ~FftCreator();

    void update( const FftParams & newParams, cl_int2 * newDataArray );

    TimeResult compute();

    cl_float2 * getFftResult() const;

private:
    void makeFftInterface( ProgramHandler * handler, const FftParams & params, cl_int2 * dataArray );
    
    FftInterface * fft;
};

#endif // GPU_FOURIER_H__
