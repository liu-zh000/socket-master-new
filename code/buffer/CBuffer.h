/********************************************************************
 * Shenzhen Thistory Bio. Tech. Co.
 * Copyright 2018 - 2018
 * 
 * Buffer.h
 *******************************************************************/
#ifndef _BUFFER_H_
#define _BUFFER_H_
/********************************************************************
 * Loop Buffer Struct
 *******************************************************************/
#include <cstdint>
class CBuffer {
public:
    CBuffer(int initBuffSize = 256);
    ~CBuffer();
	bool IsEmpty();
	bool IsFull ();
	uint32_t capacity   ();
	uint32_t length ();
	uint32_t peek   (char* dst, uint32_t len);
	uint32_t pop    (char* dst, uint32_t len);
	uint32_t append (const char* src, uint32_t len);
	uint32_t appendByte(char byte);
private:
	char*  m_buff;	// 
	uint32_t m_size;	// size must > 1
	uint32_t m_idxr;	// read position
	uint32_t m_idxw;	// write position
};


/********************************************************************
 * End of File
 *******************************************************************/
#endif // _BUFFER_H_
