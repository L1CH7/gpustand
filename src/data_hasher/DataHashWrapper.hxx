#pragma once

#include <IHash.hxx>
#include <iostream>

#include <DefaultHashType.hxx> // DefaultHashType

/**
 * An IHash wrapper class
 * used to store any hasher type (MUST be realisation of IHash interface) and check its equality
 * Example usage:
 * 
 * // Note that Tp is set in DataHashWrapper.hxx
 * Tp hash;
 * DataHashWrapper wr1( std::make_unique< Tp >( hash ); // copy c-tor
 * 
 * DataHashWrapper wr1( std::make_unique< Tp >( arguments, of, Tp, c-tor,... ) ); // unique variadic arguments c-tor
 * 
 * auto wr2 = makeDataHashWrapper( arguments, of, Tp, c-tor,... ); // variadic arguments maker (it is recommend to use this one c-tor)
 * 
 * // JSON integration:
 * json j( wr1 ); // calls to_json
 * DataHashWrapper wr1_from_j( j ); // calls from_json
 * 
 * // check equality of wrappers
 * bool eq = wr1 == wr2;
 */

class DataHashWrapper 
{
    using BaseTp = IHash;
    /**
     * DerivedTp MUST be set in this header
     */
    using DerivedTp = DefaultHashType;

    std::unique_ptr< BaseTp > impl_;

public:
    DataHashWrapper() = default;

    /**
     * Constructor without template argument. Requires set DerivedTp inside external object which will move inside impl_ ptr
     * Example call:
     * DataHashWrapper wr( std::make_unique< Tp >( arguments, of, Tp, c-tor,... ) );
     */
    DataHashWrapper( std::unique_ptr< BaseTp > && p ) 
    :   impl_( std::move( p ) ) 
    {
        // static_assert( std::is_base_of< BaseTp, impl_::value >::value,
            // "DerivedTp должен наследоваться от BaseType" );
        std::cout << "from unique c-tor!\n";
    }

    /**
     * Copy c-tor creates empty implementation
     */
    DataHashWrapper( const DataHashWrapper & other )
    :   DataHashWrapper()
    {}

    /**
     * Copy ass-ingment creates empty implementation
     */
    DataHashWrapper & operator=( const DataHashWrapper & other )
    {
        this->impl_ = nullptr;
        return *this;
    }

    DataHashWrapper( DataHashWrapper && other )
    :   DataHashWrapper( std::move( other.impl_ ) )
    {}
    
    DataHashWrapper & operator=( DataHashWrapper && other )
    {
        this->impl_ = std::move( other.impl_ );
        return *this;
    }

    bool operator==( const DataHashWrapper & other )
    {
        if( !( impl_ && other.impl_ ) )
            return false;
            
        return *impl_ == *other.impl_;
    }
    
    bool operator!=( const DataHashWrapper & other )
    {
        return !( *this == other);
    }

    /**
     * Friend maker function. 
     * Takes variadic template of Args
     * Calls DerivedTp c-tor wrapped in std::make_unique
     */
    template< typename ...Args >
    friend 
    DataHashWrapper
    makeDataHashWrapper( Args... args );

    /**
     * Using IHash::[to|from]_json(j) for serializing data
     * method compatible with nlohmann::json
     */
    friend void to_json( json & j, const DataHashWrapper & wrapper) 
    {
        if( wrapper.impl_ ) 
            wrapper.impl_->to_json( j );
        else 
            j = nullptr;
    }    
    
    friend void from_json( const json & j, DataHashWrapper & wrapper ) 
    {
        wrapper.impl_ = std::make_unique< DataHashWrapper::DerivedTp >(); // for type-compability
        wrapper.impl_->from_json( j );
    }
};

template< typename ...Args > 
DataHashWrapper
makeDataHashWrapper( Args... args ) 
{
    static_assert( std::is_base_of< DataHashWrapper::BaseTp, DataHashWrapper::DerivedTp >::value,
        "DerivedTp должен наследоваться от BaseType" );
    
    std::cout << "variate maker!\n";
    return DataHashWrapper{ std::make_unique< DataHashWrapper::DerivedTp >( std::forward< Args >( args )... ) };
}
