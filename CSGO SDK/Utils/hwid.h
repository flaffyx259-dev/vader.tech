#pragma once
#include <intrin.h>
#include <inttypes.h>
#include <string>
#include <Shlobj.h>
#include <windows.h>

#include "hash_t.h"
#include "XorStr.hpp"
#include "lazy_importer.hpp"

namespace hwid
{
	static __forceinline hash_t cpu_hash() {
		volatile int      registers[4];
		volatile uint8_t* registers_bytes;
		std::string       hash_string;

		__cpuid((int*)registers, 0x80000000);

		if (registers[0] < 0x80000002)
			return hash_t{};

		__cpuid((int*)registers, 0);

		registers_bytes = (uint8_t*)(&registers[1]);

		for (uint8_t b{}; b < (sizeof(int) * 3); ++b) {
			hash_string += (registers_bytes[b] ^ (0x2F * (b % 4)));
		}

		for (int i{}; i < 4; i++) {
			__cpuid((int*)registers, (0x80000002 + i));

			registers_bytes = (uint8_t*)registers;

			for (uint8_t b{}; b < (sizeof(int) * 4); ++b) {
				hash_string += (registers_bytes[b] ^ (0x2F * (b % 4)));
			}
		}

		return hash::fnv1a(hash_string);
	}

	__forceinline hash_t get_hash()
	{
		unsigned long serial;
		GetVolumeInformationA(XorStr("C:\\"), 0, 0, &serial, 0, 0, 0, 0);

		return cpu_hash() ^ serial;
	}
}
