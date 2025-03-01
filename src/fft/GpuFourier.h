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
    FftInterface( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );

    FftInterface( std::shared_ptr< ProgramHandler > handler );

    ~FftInterface();

    void setParams( const FftParams & newParams );

    void setDataArray( cl_int2 * newDataArray );

    void update( const FftParams & newParams, cl_int2 * newDataArray );

    virtual TimeResult compute() = 0;

protected:
    void invariant();

    std::shared_ptr< ProgramHandler > handler;
    FftParams params;
    cl_int2 * dataArray;
    cl_float2 * outArray;

    friend class FftCreator;
};

class AmFft: public FftInterface
{
public:
    AmFft( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );
    
    ~AmFft() = default;

    TimeResult compute();
};

class FmFft: public FftInterface
{
public:
    FmFft( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );

    ~FmFft() = default;

    TimeResult compute();
};

class FmFftSepNl: public FftInterface
{
public:
    FmFftSepNl( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );

    ~FmFftSepNl() = default;

    TimeResult compute();
};

class FftCreator
{
public:
    FftCreator();

    explicit FftCreator( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );

    ~FftCreator() = default;

    void makeFftInterface( std::shared_ptr< ProgramHandler > handler, const FftParams & params, cl_int2 * dataArray );

    bool hasFftInterface();

    // void setParams( const FftParams & newParams );

    // void setDataArray( cl_int2 * newDataArray );

    void update( const FftParams & newParams, cl_int2 * newDataArray );

    TimeResult compute();

    cl_float2 * getFftResult() const;

private:
    std::unique_ptr< FftInterface > fft;
};

#endif // GPU_FOURIER_H__
