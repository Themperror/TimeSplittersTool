#pragma once

#include "print.h"
#include "break.h"
namespace Utility
{
	class MemoryReader
	{
	public:
		enum class SeekMode { CURRENT, END, BEGIN};
		MemoryReader(char* data, size_t size) : data(data), size(size), offset(0) {};

		//value types (e.g: int)
		template<typename T, std::enable_if_t<!std::is_void_v<T>, int> = 0 >
		T Read()
		{
			constexpr size_t dataSize = sizeof(T);
			if (offset + dataSize > size)
			{
				Utility::Print("MemoryReader::Read, out of bounds read: %zu / %zu", offset, size);
				Utility::Break();
			}
			T val;
			memcpy(&val, data + offset, dataSize);
			offset += dataSize;
			return val;
		}

		template<typename T, std::enable_if_t<std::is_pointer_v<T>, int> = 0 >
		void Read(T ptr, size_t numElements)
		{
			constexpr size_t dataSize = sizeof(std::remove_pointer_t<T>);
			if (offset + dataSize*numElements > size)
			{
				Utility::Print("MemoryReader::Read, out of bounds read: %zu / %zu", offset, size);
				Utility::Break();
			}
			memcpy(ptr, data + offset, dataSize*numElements);
			offset += dataSize * numElements;
		}

		//array types (e.g: char[4])
		template<typename T, size_t numElements, std::enable_if_t<std::is_array_v<T>, int> = 0 >
		void Read(T(&destination)[numElements])
		{
			constexpr size_t dataSize = sizeof(T) * numElements;
			if (offset + dataSize > size)
			{
				Utility::Print("MemoryReader::Read, out of bounds read: %zu / %zu", offset, size);
				Utility::Break();
			}
			memcpy(destination, data + offset, dataSize);
			offset += dataSize;
		}

		//pointer type (e.g: char*)
		template<typename T, size_t numElements, typename U, std::enable_if_t<std::is_pointer_v<U> && !std::is_array_v<U>, int> = 0>
		void Read(U destination)
		{
			constexpr size_t dataSize = sizeof(T) * numElements;
			if (offset + dataSize > size)
			{
				Utility::Print("MemoryReader::Read, out of bounds read: %zu / %zu", offset, size);
				Utility::Break();
			}
			memcpy(destination, data + offset, dataSize);
			offset += dataSize;
		}


		bool IsEOF() { return offset == size; }
		void Skip(size_t bytes)
		{
			offset += bytes;
		}
		void Seek(size_t offset, SeekMode mode = SeekMode::BEGIN)
		{
			switch (mode)
			{
				case SeekMode::CURRENT:
					Skip(offset);
					break;
				case SeekMode::BEGIN:
					this->offset = 0;
					Skip(offset);
					break;
				case SeekMode::END:
				{
					size_t newOffset = size - offset;
					if (newOffset > size)
					{
						Utility::Print("MemoryReader::Seek, out of bounds: %zu / %zu", offset, size);
						Utility::Break();
					}
					this->offset = newOffset;
					break;
				}
			}
		}
		size_t Tell()
		{
			return offset;
		}
		size_t Size()
		{
			return size;
		}

	private:
		char* data;
		size_t size;
		size_t offset;
	};

}