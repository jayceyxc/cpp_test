/*
* PBE
* (C) 1999-2007 Jack Lloyd
*
* Distributed under the terms of the Botan license
*/

#ifndef BOTAN_PBE_BASE_H__
#define BOTAN_PBE_BASE_H__

#include <asn1_oid.h>
#include <data_src.h>
#include <filter.h>
#include <rng.h>

namespace Botan {

/**
* Password Based Encryption (PBE) Filter.
*/
class BOTAN_DLL PBE : public Filter
   {
   public:
      /**
      * Set this filter's key.
      * @param pw the password to be used for the encryption
      */
      virtual void set_key(const std::string&) = 0;

      /**
      * Create a new random salt value and set the default iterations value.
      */
      virtual void new_params(RandomNumberGenerator& rng) = 0;

      /**
      * DER encode the params (the number of iterations and the salt value)
      * @return the encoded params
      */
      virtual MemoryVector<byte> encode_params() const = 0;

      /**
      * Decode params and use them inside this Filter.
      * @param src a data source to read the encoded params from
      */
      virtual void decode_params(DataSource&) = 0;

      /**
      * Get this PBE's OID.
      * @return the OID
      */
      virtual OID get_oid() const = 0;
   };

}

#endif
