/********************************************************************
 * Shenzhen Thistory Bio. Tech. Co.
 * Copyright 2018 - 2018
 * 
 * Buffer.c
 *******************************************************************/
#include <string.h>
#include <unistd.h> 
#include "CBuffer.h"
#include <iostream>
#define TMEP_BUFF_LEN (4096)	
/********************************************************************
 * m_size must > 1
 *******************************************************************/
CBuffer::CBuffer(int initBuffSize)
{
	m_buff = new char[initBuffSize];
	m_idxr  = m_idxw = 0;
	m_size = (m_buff != nullptr) ? initBuffSize : 0;
}

/********************************************************************
 * 
 *******************************************************************/
CBuffer::~CBuffer()
{
	if(m_buff){
		delete m_buff;
	}
}

/********************************************************************
 * if m_buff is null, will return  true
 *******************************************************************/
bool CBuffer::IsEmpty(void)
{
	if (m_buff){
		if (m_idxr != m_idxw)
			return  false;
	}
	return  true;
}
/********************************************************************
 * if m_buff is null, will return  true
 *******************************************************************/
bool CBuffer::IsFull (void)
{
	if (m_buff) {
		uint32_t idxw = m_idxw;
		uint32_t nextw = (idxw+1) % (m_size);
		if (nextw != m_idxr)
			return  false;
	}
	return  true;
}
/********************************************************************
 * if m_buff is null, will return 0
 *******************************************************************/
uint32_t CBuffer::capacity   (void)
{
	return m_size;
}
/********************************************************************
 * if m_buff is null, will return 0
 *******************************************************************/
uint32_t CBuffer::length (void)
{
	if (m_buff) {
		if (m_idxw >= m_idxr)
			return (m_idxw - m_idxr);
		else
			return (m_idxw + m_size - m_idxr);
	}
	return 0;
}
/********************************************************************
 * copy but do not move m_idxr
 *******************************************************************/
uint32_t CBuffer::peek(char* dst, uint32_t len)
{
	if (m_buff) {
		// how many bytes to copy?
		uint32_t total = length();
		len = total >= len ? len : total;
		// if just have 1 segment to copy
		if ( m_idxr <= m_idxw ) {
			memcpy (dst, &(m_buff[m_idxr]), len );
		}
		// or have 2 segment to copy
		else{
			uint32_t seg1 = m_size - m_idxr;
			seg1 = seg1 >= len ? len : seg1;
			uint32_t seg2 = len - seg1;
			memcpy (dst, &(m_buff[m_idxr]), seg1);
			if (seg2)
				memcpy (dst+seg1, &(m_buff[0]), seg2);
		}
		return len;
	}
	return 0;
}
/********************************************************************
 * copy and move m_idxr
 *******************************************************************/
uint32_t CBuffer::pop(char* dst, uint32_t len)
{
	len = peek (dst, len );
	if (len && m_buff) {
		m_idxr = m_idxr + len;
		m_idxr = m_idxr % m_size;
	}
	return len;
}
/********************************************************************
 * append byte array
 *******************************************************************/
uint32_t CBuffer::append (const char* src, uint32_t len)
{
	static int cnt = 0;
	cnt++;
	static int flag = 0;
	if(cnt > 254){
		flag++;
	}
	uint32_t freebyte = m_size - length() - 1;
	if(len >= freebyte){
		return 0;
	}
	if (m_buff) {
		// how many free byte?
		
		// reset copy length
		len = len > freebyte ? freebyte : len;
		// if just have 1 segment free room for copy
		if ( m_idxw < m_idxr ) {
			memcpy ( &m_buff[m_idxw], src, len );
			m_idxw += len;
		}
		// or have 2 segment for copy
		else{
			uint32_t seg1 = m_size - m_idxw;
			seg1 = seg1 >= len ? len : seg1;
			uint32_t seg2 = len - seg1;
			memcpy ( &m_buff[m_idxw] , src, seg1 );
			
			if (seg2){
				memcpy ( &m_buff[0], src+seg1, seg2 );
				m_idxw = seg2;
			}else {
				m_idxw += len;
			}
		}
		return len;

	}
	return 0;
}
/********************************************************************
 * append one byte
 *******************************************************************/
uint32_t CBuffer::appendByte(char byte)
{
	if (m_buff && !IsFull() ){
		m_buff [ m_idxw++ ] = byte;
		m_idxw %= m_size;
		return 1;
	}
	return 0;
}


/********************************************************************
 * END OF FILE
 *******************************************************************/
 
