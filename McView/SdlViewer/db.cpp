#include "db.h"

#include <leveldb/db.h>
#include <leveldb/env.h>
#include <leveldb/filter_policy.h>
#include <leveldb/cache.h>
#include <leveldb/zlib_compressor.h>
#include <leveldb/decompress_allocator.h>

#include <vector>
#include <memory>
#include <iostream>
#include <map>
#include <algorithm>

using namespace std;
using namespace leveldb;

class std_out_logger : public Logger
{
public:
	void Logv(const char* format, const va_list args) override
	{
		auto buffer = vector<char>(256);
		const size_t actual_size = vsnprintf(&buffer[0], buffer.size(), format, args);
		if (actual_size + 1 > buffer.size())
		{
			buffer.resize(actual_size + 1);
			vsnprintf(&buffer[0], buffer.size(), format, args);
		}
		cout << &buffer[0];
	}
};

int read_int_le(const char* begin, const char* end, const char** current)
{
	if (end - begin < sizeof(int))
		throw runtime_error("Less than 4 bytes for integer");
	int result = *reinterpret_cast<const int*>(begin);
	*current = begin + sizeof(int);
	return result;
}

struct block_coords
{
	int x, z, y;
};

block_coords get_block_coords(const Slice& block_key)
{
	const char* data_ptr = block_key.data();
	const char* end = data_ptr + block_key.size();
	int x = read_int_le(data_ptr, end, &data_ptr);
	int z = read_int_le(data_ptr, end, &data_ptr);
	if (data_ptr + 1 >= end)
		throw runtime_error("Key does not contain y coordinate");
	int y = data_ptr[1];
	return { x * 16, z * 16, y * 16 };
}

vector<point> load_points()
{
	Options options;
	options.filter_policy = leveldb::NewBloomFilterPolicy(10);
	options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);
	options.write_buffer_size = 4 * 1024 * 1024;
	options.info_log = new std_out_logger();
	options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);
	options.compressors[1] = new leveldb::ZlibCompressor();

	unique_ptr<DB> db;
	{
		DB* dbptr;
		auto status = DB::Open(options, "last/db", &dbptr);
		if (!status.ok())
			throw runtime_error("Error opening db");
		db.reset(dbptr);
	}

	ReadOptions read_options;
	read_options.decompress_allocator = new DecompressAllocator();
	unique_ptr<Iterator> iterator(db->NewIterator(read_options));

	vector<point> result;
	int block = 0;
	int frequency[256] = {0};

	for(iterator->SeekToFirst(); iterator->Valid() /*&& block < 5*/; iterator->Next())
	{
		Slice key = iterator->key();
		if (key.size() == 10)
		{
			Slice value = iterator->value();
			if (value.size() == 0x1801)
			{
				auto value_array = value.data();
				auto start_coords = get_block_coords(key);
				for (int y = 0; y < 16; ++y)
				{
					for (int x = 0; x < 16; ++x)
					{
						for (int z = 0; z < 16; ++z)
						{
							const auto block_value = value_array[x + z * 16 + y * 256 + 1];
							++frequency[static_cast<unsigned char>(block_value)];

							bool keep = block_value == 0
								|| block_value == 8
								|| block_value == 10
								|| block_value == 58
								|| block_value == 49;
								//|| block_value == 56;

							if (keep)
							{
								bool skip = false;

								if (x >= 1 && x < 15 && y >= 1 && y < 15 && z >= 1 && z < 15)
								{
									auto v1 = value_array[x + 1 + z * 16 + y * 256 + 1];
									auto v2 = value_array[x - 1 + z * 16 + y * 256 + 1];
									auto v3 = value_array[x + (z + 1) * 16 + y * 256 + 1];
									auto v4 = value_array[x + (z - 1) * 16 + y * 256 + 1];
									auto v5 = value_array[x + z * 16 + (y + 1) * 256 + 1];
									auto v6 = value_array[x + z * 16 + (y - 1) * 256 + 1];
									auto e1 = block_value == v1;
									auto e2 = block_value == v2;
									auto e3 = block_value == v3;
									auto e4 = block_value == v4;
									auto e5 = block_value == v5;
									auto e6 = block_value == v6;
									skip = e1 + e2 + e3 + e4 + e5 + e6 == 6;
								}
								else
								{
									skip = block_value == 0;
								}

								if (!skip)
								{
									float xc = (start_coords.x + y) * 0.3f;
									float yc = (start_coords.z + z) * 0.3f;
									float zc = (start_coords.y + x) * 0.3f;
									float r = block_value == 0 ? 1.f : (block_value & 0x7) / 8.f;
									float g = block_value == 0 ? 1.f : (block_value & 0x1C >> 3) / 8.f;
									float b = block_value == 0 ? 1.f : (block_value & 0xC0 >> 3) / 4.f;
									float size = block_value == 0 ? 1.f : 5.f;
									result.push_back({ { r, g, b }, { -yc, zc, xc }, size, block_value });
								}
							}
						}
					}
				}

				++block;
			}
		}
	}

	vector<pair<int, int>> stat;
	for (int i = 0; i < 256; ++i)
	{
		if (frequency[i] != 0)
			stat.push_back(make_pair(i, frequency[i]));
	}

	sort(stat.begin(), stat.end(), [](auto a, auto b)
	{
		return a.second < b.second;
	});
	for (const auto pair : stat)
	{
		cout << "val: " << pair.first << ", count: " << dec << pair.second << endl;
	}

	return result;
}