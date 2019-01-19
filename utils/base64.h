/*

Author: Nandakumar
e-mail: nandakumar.nie@gmail.com
Description: Base64 encode/decode header.
Version: 1.1

*/

#ifndef __BASE64__H__
#define __BASE64__H__

#include<map>
#include<stdio.h>
#include<iostream>

using namespace std;

class Base64
{
	private:
		static char BASE64_TABLE[65];
		static map<char,int> BASE64_RTABLE;
		Base64(){}
		int getNumberOfPaddingChars(const int nsiZe) const;		
		static bool create_map();
		static bool dummy;

	public:
		/* Gets instance for this class*/
		static Base64* getInstance();

		/*
			Encodes data of size to outputBuffer.
			outputBuffer must be large enough to hold encoded data.
			See getEncodeLength API
		*/
		void Encode(const char* data, const int size, char* outputBuffer);

		/* 
			Decodes data of size to outputBuffer.
			outputBuffer must be large enough to hold decoded data.
			See getDecodeLength API
		*/
		void Decode(const char* data, const int size, char* outputBuffer);

		/* Same as above, except provides info about actual data size of decoded data */
		void Decode(const char* data, const int size, char* outputBuffer, int& bytes_converted);

		/* Returns encoded length for data of 'nSizeData' length */
		static int getEncodeLength(const int nSizeData);

		/* 
			Gets the decode size of encoded data with 'nSizebase64' length.
	
			NOTES:
				This interface gives +1 result when data ends with "==".
				Combine the interface with Decode(const char*, const int, char*, int&)
				for accurate result.
		*/
		static int getDecodeLength(const int nSizebase64);
};
#endif
