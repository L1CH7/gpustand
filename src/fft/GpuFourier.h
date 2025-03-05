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
    FftInterface( std::shared_ptr< ProgramHandler > handler, FftData & data );

    FftInterface( std::shared_ptr< ProgramHandler > handler );

    ~FftInterface() = default;

    // void setParams( const FftParams & new_params );

    // void setDataArray( cl_int2 * new_data_array );

    void update( FftData & data );

    virtual TimeResult compute() = 0;

protected:
    void invariant();

    bool ready{ true };
    std::shared_ptr< ProgramHandler > handler_;
    FftParams params_;
    std::vector< int > mseq_;
    std::vector< std::complex< int > > data_array_;
    std::vector< std::complex< float > > out_array_;

    friend class FftCreator;
};

class DummyFft;

class AmFft: public FftInterface
{
public:
    AmFft( std::shared_ptr< ProgramHandler > handler, FftData & data );
    
    ~AmFft() = default;

    TimeResult compute();
};

class FmFft: public FftInterface
{
public:
    FmFft( std::shared_ptr< ProgramHandler > handler, FftData & data );

    ~FmFft() = default;

    TimeResult compute();
};

class FmFftSepNl: public FftInterface
{
public:
    FmFftSepNl( std::shared_ptr< ProgramHandler > handler, FftData & data );

    ~FmFftSepNl() = default;

    TimeResult compute();
};

class FftCreator
{
public:
    FftCreator() = default;

    explicit FftCreator( std::shared_ptr< ProgramHandler > handler );

    explicit FftCreator( std::shared_ptr< ProgramHandler > handler, FftData & data );

    ~FftCreator() = default;

    void makeFftInterface( std::shared_ptr< ProgramHandler > handler, FftData & data );

    bool hasFftInterface();

    // void setParams( const FftParams & new_params );

    // void setDataArray( cl_int2 * new_data_array );

    void update( FftData & data );

    TimeResult compute();

    std::vector< std::complex< float > > getFftResult();

private:
    std::unique_ptr< FftInterface > fft_;
};

#endif // GPU_FOURIER_H__
