#include <leveldb/db.h>
#include <leveldb/env.h>
#include <leveldb/filter_policy.h>
#include <leveldb/cache.h>
#include <leveldb/zlib_compressor.h>
#include <leveldb/decompress_allocator.h>

#include <vector>
#include <iostream>

using namespace leveldb;
using namespace std;

#define EXP extern "C" __declspec(dllexport)
#define CC __stdcall

namespace
{
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
}


EXP DB* CC OpenDatabase(const char* name)
{
	leveldb::Options options;

	//create a bloom filter to quickly tell if a key is in the database or not
	options.filter_policy = leveldb::NewBloomFilterPolicy(10);

	//create a 40 mb cache (we use this on ~1gb devices)
	options.block_cache = leveldb::NewLRUCache(40 * 1024 * 1024);

	//create a 4mb write buffer, to improve compression and touch the disk less
	options.write_buffer_size = 4 * 1024 * 1024;

	//disable internal logging. The default logger will still print out things to a file
	options.info_log = new std_out_logger();

	//use the new raw-zip compressor to write (and read)
	options.compressors[0] = new leveldb::ZlibCompressorRaw(-1);

	//also setup the old, slower compressor for backwards compatibility. This will only be used to read old compressed blocks.
	options.compressors[1] = new leveldb::ZlibCompressor();

	DB* db;
	auto status = DB::Open(options, name, &db);
	if (!status.ok())
		return nullptr;

	return db;
}

EXP void CC CloseDatabase(DB* db)
{
	delete db;
}

EXP Iterator* CC NewIterator(DB* db)
{
	//create a reusable memory space for decompression so it allocates less
	ReadOptions read_options;
	read_options.decompress_allocator = new DecompressAllocator();
	return db->NewIterator(read_options);
}

EXP void CC DisposeIterator(Iterator* iterator)
{
	delete iterator;
}

EXP bool CC IteratorValid(Iterator* iterator)
{
	return iterator->Valid();
}

EXP bool CC IteratorSeekToFirst(Iterator* iterator)
{
	iterator->SeekToFirst();
	return iterator->Valid();
}

EXP bool CC IteratorNext(Iterator* iterator)
{
	iterator->Next();
	return iterator->Valid();
}

struct key_value_data
{
	size_t key_size;
	const char* key_data;
	size_t value_size;
	const char* value_data;
};

EXP key_value_data CC IteratorGetData(Iterator* iterator)
{
	return {
		iterator->key().size(),
		iterator->key().data(),
		iterator->value().size(),
		iterator->value().data()
	};
}