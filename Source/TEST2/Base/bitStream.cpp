#include "bitStream.h"
#include "binary.h"

#pragma optimize("",off) 
namespace Base
{
	BitStream::BitStream(void *bufPtr, S32 bufSize, S32 maxWriteSize /* = -1 */)
	{
		setBuffer(bufPtr, bufSize,maxWriteSize);
	}
    
	BitStream::~BitStream()
	{
	}
  
	bool BitStream::bindPacketStream(U8 *buffer,U32 writeSize)
	{
		if(!writeSize)
			return false;
        
		setBuffer(buffer, writeSize);
		setPosition(0);
		return true;
	}
    
    
	void BitStream::setBuffer(void *bufPtr, S32 size, S32 maxSize)
	{
		dataPtr = (U8 *) bufPtr;
		bitNum = 0;
		//bitNum = 8;			//First Byte ReSave, for zip flag
		flagNum = 0;
		tailFlag = false;
		bufSize = size;
		maxReadBitNum = size << 3;
		if(maxSize < 0)
			maxSize = size;
		maxWriteBitNum = maxSize << 3;
		bitsLimite = bufSize<<3; //Ray:set limite /*(bufSize/2)<<3;*/
		error = false;
	}
    
	U32 BitStream::getPosition() const
	{
		return (bitNum + 7) >> 3;
	}
    
	bool BitStream::setPosition(const U32 pos)
	{
		//Ray: BitStream has two POSITION，one flagNum set FLAG BIT read/write positon，another bitNum set other order data read/write positon
		//To this is for write speed and write size.So can not call setPosition to rand a position to write as you like, but pos is zero or flagNum is zero this time can a  exact setposition!
		LIB_ASSERT(!pos || !flagNum,"wrong setPosition call");
		if(pos && flagNum)
			return false;
        
		bitNum = pos << 3;
		flagNum = 0;
		return (true);
	}
    
	U32 BitStream::getStreamSize()
	{
		return bufSize;
	}
    
	U8 *BitStream::getBytePtr()
	{
		return dataPtr + getPosition();
	}
    
	U32 BitStream::getReadByteSize()
	{
		return (maxReadBitNum >> 3) - getPosition();
	}
    
	void BitStream::clear()
	{
		dMemset(dataPtr, 0, bufSize);
	}
    
	void BitStream::writeBits(S32 bitCount, const void *bitPtr)
	{
		if(!bitCount)
			return;
        
		if(tailFlag)
		{
			error = true;
			LIB_ASSERT(false, "Out of range write");
			return;
		}
        
		// after beter code
		//S32 bitAdd = (bitCount - ((bitCount>>3)<<3) );
		//bitAdd = bitAdd ? 8-bitAdd : 0;
		//bitCount+=bitAdd;
		if( bitCount & 0x7 )
			bitCount = ( bitCount & ~0x7 ) + 8;
        
		if(bitCount + bitNum > maxWriteBitNum)
		{
			error = true;
			LIB_ASSERT(false, "Out of range write");
			return;
		}
        
		U8 *stPtr = dataPtr + (bitNum >> 3);
		S32 byteCount = (bitCount + 7) >> 3;
        
		memcpy(stPtr,bitPtr,byteCount);
        
		bitNum += bitCount;
	}

	bool BitStream::writeFlag(bool val)
	{
		if((flagNum - ((flagNum>>3)<<3) ==0) && !tailFlag)
		{
			flagNum = bitNum;
            
			if(bitNum + 8 < maxWriteBitNum)
				bitNum+=8; //Ray; jump 8bit for write flag
			else
				tailFlag = true;
		}
        
		if(flagNum + 1 > maxWriteBitNum)
		{
			error = true;
			LIB_ASSERT(false, "Out of range write");
			return false;
		}
        
		if(val)
			*(dataPtr + (flagNum >> 3)) |= (1 << (flagNum & 0x7));
		else
			*(dataPtr + (flagNum >> 3)) &= ~(1 << (flagNum & 0x7));
		flagNum++;
		return (val);
	}
    
	void BitStream::readBits(S32 bitCount, void *bitPtr)
	{
		if(!bitCount)
			return;
        
		if(tailFlag)
		{
			error = true;
			LIB_ASSERT(false, "Out of range read");
			return;
		}
        
		// can be beter code
		
		//// can be beter code
		////S32 bitAdd = (bitCount - ((bitCount>>3)<<3) );
		//S32 bitAdd = bitCount & 0x7;
        
		//// can be beter code
		////bitAdd = bitAdd ? 8-bitAdd : 0;
        
		//bitAdd = (8-bitAdd) & 0x7;
        
		//bitCount+=bitAdd;
        
		if( bitCount & 0x7 )
			bitCount = ( bitCount & ~0x7 ) + 8;
        
		if(bitCount + bitNum > maxReadBitNum)
		{
			error = true;
			//LIB_ASSERT(false, "Out of range read");
			LIB_ASSERT(false, "Out of range read");
			return;
		}
		U8 *stPtr = dataPtr + (bitNum >> 3);
		S32 byteCount = (bitCount + 7) >> 3;
        
		//U8 *ptr = (U8 *) bitPtr;
        
		memcpy(bitPtr,stPtr,byteCount);
        
		bitNum += bitCount;
	}
    
	void BitStream::writeInt(S32 val, S32 bitCount)
	{
		char buff[4] = {0, 0, 0, 0};
		LITTLE->PutUint32(buff, val);
		writeBits(bitCount, buff);
	}

	S32 BitStream::readInt(S32 bitCount)
	{
		S32 ret = 0;
		char buff[4] = { 0, 0, 0, 0 };
		readBits(bitCount, buff);
		ret = LITTLE->Uint32(buff);
		if (bitCount == 32)
			return ret;
		else
			ret &= (1 << bitCount) - 1;
		return ret;
	}

	void BitStream::writeInt64(S64 val, S32 bitCount)
	{
		char buff[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		LITTLE->PutUint64(buff, val);
		writeBits(bitCount, buff);
	}

	S64  BitStream::readInt64(S32 bitCount) 
	{
		S64 ret = 0;
		char buff[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		readBits(bitCount, buff);
		ret = LITTLE->Uint64(buff);
		if (bitCount == 64)
			return ret;
		else
			ret &= (1 << S64(bitCount)) - 1;
		return ret;
	}

	void BitStream::writeFloat(F32 f, S32 bitCount)
	{
		writeInt(*(S32*)(&f), bitCount);
	}
    
	F32 BitStream::readFloat(S32 bitCount)
	{
		S32 val = readInt(bitCount);
		return *(F32*)(&val) ;
	}  

	void BitStream::writeFloat64(F64 f, S32 bitCount)
	{
		writeInt64(*(S64*)(&f), bitCount);
	}

	F64 BitStream::readFloat64(S32 bitCount)
	{
		S64 val = readInt64(bitCount);
		return *(F64*)(&val);
	}

	//------------------------------------------------------------------------------
	void BitStream::readString(char *buf,int maxSize)
	{
		buf[0] = 0;
        
		if(readFlag())
		{
			int len = readInt(16);
			LIB_ASSERT(len<maxSize,"BitStream::readString Out Range");
			if(len>=maxSize)
			{
				char *pNewBuf = new char[len];
				readBits(len<<3,pNewBuf);
                
				len = maxSize-1;
				memcpy(buf,pNewBuf,len);
				delete [] pNewBuf;
			}
			else
			{
				readBits(len<<3,buf);
			}
			
			buf[len] = 0;
		}
	}
    
	void BitStream::writeString(const char *string, S32 maxSize)
	{
		if(!string)
			string = "";
        
		S32 len = dStrlen(string);
		LIB_ASSERT(len<maxSize,"BitStream::writeString Out Range");
		if(len>=maxSize)
			len = maxSize-1;
        
		if(writeFlag(len>0))
		{
			writeInt(len,16);
			writeBits(len<<3,string);
		}
	}

	std::string BitStream::readString(int maxSize) 
	{
		char buff[1024];
		readString(buff, maxSize);
		return buff;
	}
	void BitStream::writeString(std::string str, S32 maxLen)
	{
		writeString(str.c_str(), maxLen);
	}
};
#pragma optimize("",on) 