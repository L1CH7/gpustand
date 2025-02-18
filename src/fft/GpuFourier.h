#ifndef __GPU_FFT_H
#define __GPU_FFT_H

#include <iostream>
#include <ProgramHandler.h>
#include <types.h>
#include <error.h>

// struct TimeResult
// {
//     unsigned total_time;

//     /// Время записи входного массива (filter_input) на ячейку (GPU).
//     unsigned write_input_time;

//     /// Время расчета синусов.
//     unsigned sine_computation_time;

//     /// Время БПФ М-последовательности (ONLY FM)
//     unsigned mseq_fft_time;

//     /// Время БПФ входных данных (это прямой БПФ, за ним будет еще и обратный)
//     unsigned fft_time;

//     /// Время обратного БПФ данных, сгенерированный предыдущим шагом. (ONLY FM)
//     unsigned ifft_time; 

//     /// Время чтения результата с ячейки.
//     unsigned final_read_time;
// };

// TODO:
// Добавить timestamp-ы для записи времени начала и конца, вместо общего времени к-л вычисления



class FftInterface
{
public:
    FftInterface( 
        ProgramHandler * handler, 
        const FftParams & params,
        cl_int2 * dataArray
    )   
    :   handler( std::move(handler) ),
        params( params ),
        dataArray( dataArray ),
        outArray( new cl_float2[params.nl * params.kgd * params.kgrs] )
    {
        invariant();
    }

    FftInterface(){}
    ~FftInterface()
    {
        if( outArray )
            delete outArray;
    }

    virtual TimeResult compute() = 0;

    cl_float2 * getFftResult() const
    {
        return outArray;
    }

protected:
    void invariant()
    {
        if( handler == nullptr )
        {
            // std::cout << __PRETTY_FUNCTION__ << ":\n\t";
            std::cout << "handler->" << handler << std::endl;
            PRINT_ERROR("No OpenCL handler found");
            assert(0);
        }
        if( params.nl < 1 || params.kgd < 1 || params.kgrs < 1 )
        {
            // std::cout << __PRETTY_FUNCTION__ << ":\n\t";
            // std::cout << "Incorrect params" << std::endl;
            PRINT_ERROR("Incorrect params");
            assert(0);
        }
        // if( dataArray.size() < params.samples_num || ( !params.is_am && params.mseq.size() < params.true_nihs ) )
        if( ( !params.is_am && params.mseq.size() < params.true_nihs ) )
        {
            // std::cout << __PRETTY_FUNCTION__ << ":\n\t";
            // std::cout << "Not enough data" << std::endl;
            PRINT_ERROR("Not enough data");
            assert(0);
        }
    }

    ProgramHandler * handler;
    FftParams params;
    cl_int2 * dataArray;
    cl_float2 * outArray;
};

class AmFft: public FftInterface
{
public:
    AmFft( 
        ProgramHandler * handler, 
        const FftParams & params,
        cl_int2 * dataArray
    )   
    :   FftInterface( handler, params, dataArray )
    {}
    
    TimeResult compute();
};

class FmFft: public FftInterface
{
public:
    FmFft( 
        ProgramHandler * handler, 
        const FftParams & params,
        cl_int2 * dataArray
    )   
    :   FftInterface( handler, params, dataArray )
    {}

    TimeResult compute();
};

class FmFftSepNl: public FftInterface
{
public:
    FmFftSepNl( 
        ProgramHandler * handler, 
        const FftParams & params,
        cl_int2 * dataArray
    )   
    :   FftInterface( handler, params, dataArray )
    {}

    TimeResult compute();
};

class FftCreator
{
public:
    FftCreator() = delete;

    explicit FftCreator( 
        ProgramHandler * handler, 
        const FftParams & params,
        cl_int2 * dataArray
    )   
    {
        if( params.is_am )
            fft = new AmFft( handler, params, dataArray );
        else
            fft = new FmFft( handler, params, dataArray );
    }

    ~FftCreator()
    {
        delete fft;
    }

    TimeResult compute()
    {
        return fft->compute();
    }

    cl_float2 * getFftResult() const
    {
        return fft->getFftResult();
    }

private:
    FftInterface * fft;
};

#endif // __GPU_FFT_H
