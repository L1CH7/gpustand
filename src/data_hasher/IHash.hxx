#pragma once

#include <JsonHelper.hxx>

/**
 * 
 * Interface for data hashing 
 * you must override protected equalTo method to make work equality operators
 * which type of hash your realization stores is your decision
 * Example realization:
 * 
 * class MyHash : public IHash
 * {
 *      HashTp hash_;
 *      // other members required for hashing and equality checking
 * public:
 *      MyHash(Args...)
 *      {
 *          // logic to compute hash from given data in args 
 *      }
 * 
 *      void to_json( json & j ) const
 *      {
 *          // logic for json serializing
 *      }
 * protected:
 *      bool equalTo( const IHash & other ) const override
 *      {
 *          const MyHash & that = dynamic_cast< const MyHash & >( other ); // needed to get other casted to our hasher type
 *          auto other_hash = that.hash_;
 *          // logic for comparing two hashes...
 *      }
 * }
 * 
 * From now, you can do next:
 * MyHash m1(args...);
 * MyHash m2(args...);
 * bool eq = m1 == m2;
 */
class IHash
{
protected:
    virtual bool equalTo( const IHash & other ) const = 0;
    
public:
    virtual ~IHash() = default;

    bool operator==( const IHash & other ) const 
    {
        return this->equalTo( other );
    }

    bool operator!=( const IHash & other ) const 
    {
        return !( this->equalTo( other ) );
    }

    virtual void to_json( json & j ) const = 0;

    virtual void from_json( const json & j ) = 0;
};
