#pragma once

#pragma warning( disable : 4307 ) // '*': integral constant overflow
#pragma warning( disable : 4244 ) // possible loss of data

#include <cstdint>
#include <string>

// weird hacky fix for hashes not working when passing to functions as an arg / etc. fuck you msvc
// always use this for your compile-time hashes.
#define HASH( str ) \
       []() { \
           constexpr hash_t ret = hash::fnv1a_ct( str ); \
           return ret; \
       }() \

using hash_t = uint64_t;

namespace hash
{
	enum MagicNumbers : uint64_t {
		FNV1A_PRIME = 1099511628211u,
		FNV1A_BIAS = 14695981039346656037u
	};

	// run-time FNV-1a hash.
	__forceinline hash_t fnv1a(const void* data, size_t len) {
		hash_t	 out;
		uint8_t* enc;

		if (!data || !len)
			return hash_t{ };

		// setup vars.
		out = FNV1A_BIAS;
		enc = (uint8_t*)data;

		// make hash.
		for (size_t i{ }; i < len; ++i) {
			out ^= enc[i];
			out *= FNV1A_PRIME;
		}

		return out;
	}

	// run-time FNV-1a string hash.
	__forceinline hash_t fnv1a(const std::string& str) {
		return fnv1a(str.c_str(), str.length());
	}

	// compile-time FNV-1a string hash.
	constexpr hash_t fnv1a_ct(const char* str, hash_t start = FNV1A_BIAS) {
		return (*str != '\0') ? fnv1a_ct(str + 1, (*str ^ start) * FNV1A_PRIME) : start;
	}
};