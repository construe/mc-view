// LevelDb.cpp : Defines the exported functions for the DLL application.
//

#include <port/port_win.h>

namespace leveldb {
	namespace port {
		uint32_t AcceleratedCRC32C(uint32_t crc, const char* buf, size_t size)
		{
			return 0;
		}
	}
}