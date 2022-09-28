//**************************************************************************************
//  Copyright (C) 2002 - 2011, Huamin Wang
//  All rights reserved.
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//     1. Redistributions of source code must retain the above copyright
//        notice, this list of conditions and the following disclaimer.
//     2. Redistributions in binary form must reproduce the above copyright
//        notice, this list of conditions and the following disclaimer in the
//        documentation and/or other materials provided with the distribution.
//     3. The names of its contributors may not be used to endorse or promote
//        products derived from this software without specific prior written
//        permission.
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//**************************************************************************************
// BMP_IO.h
//**************************************************************************************
#ifndef __FILE_IO_BMP_IO_H__
#define __FILE_IO_BMP_IO_H__
#include <stdio.h>
#include <windows.h>


// We only consider 24 bits RGB color BMP files.
// The real storage sequence in BITMAP is BGR, not RGB.


bool BMP_Read(const char* filename, BYTE** pixels, int& width, int& height)
{

	HBITMAP bmp_handle = (HBITMAP)LoadImageA(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE);
	if (bmp_handle == INVALID_HANDLE_VALUE) return false;
	BITMAP bitmap;
	GetObject(bmp_handle, sizeof(BITMAP), (LPSTR)&bitmap);

	if (bitmap.bmPlanes * bitmap.bmBitsPixel != 24)
	{
		printf("Error: The bmp image depth is not 24.\n"); DeleteObject(bmp_handle); return false;
	}

	width = bitmap.bmWidth;
	height = bitmap.bmHeight;
	if (*pixels) delete* pixels;
	*pixels = new BYTE[bitmap.bmWidth * bitmap.bmHeight * 3];

	unsigned char* ptr = (unsigned char*)(bitmap.bmBits);
	for (int j = 0; j < height; j++)
	{
		unsigned char* line_ptr = ptr;
		for (int i = 0; i < width; i++)
		{
			(*pixels)[3 * (i * height + j)] = line_ptr[2];
			(*pixels)[3 * (i * height + j) + 1] = line_ptr[1];
			(*pixels)[3 * (i * height + j) + 2] = line_ptr[0];
			line_ptr += 3;
		}
		ptr += bitmap.bmWidthBytes;
	}
	DeleteObject(bmp_handle);
	return true;
}




bool BMP_Write(const char* filename, BYTE** pixels, int width, int height)
{
	//Preparing the BITMAP data structure from IMAGE object.
	HDC dc = GetDC(NULL);
	BITMAPINFO info;
	ZeroMemory(&info.bmiHeader, sizeof(BITMAPINFOHEADER));
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 24;
	info.bmiHeader.biSizeImage = 0;
	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	VOID* pvBits;
	HBITMAP bmp_handle = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pvBits, NULL, 0);
	BITMAP bitmap;
	GetObject(bmp_handle, sizeof(BITMAP), (LPSTR)&bitmap);


	unsigned char* ptr = (unsigned char*)(bitmap.bmBits);
	for (int j = 0; j < height; j++)
	{
		unsigned char* line_ptr = ptr;
		for (int i = 0; i < width; i++)
		{
			line_ptr[2] = (*pixels)[(j * width + i) * 3 + 0];
			line_ptr[1] = (*pixels)[(j * width + i) * 3 + 1];
			line_ptr[0] = (*pixels)[(j * width + i) * 3 + 2];
			line_ptr += 3;
		}
		ptr += bitmap.bmWidthBytes;
	}

	//Decide the data size.
	WORD wBitCount = 24;
	DWORD dwPaletteSize = 0, dwBmBitsSize, dwDIBSize, dwWritten;
	LPBITMAPINFOHEADER lpbi;
	dwBmBitsSize = ((bitmap.bmWidth * wBitCount + 31) / 32) * 4 * bitmap.bmHeight;

	//Preparing the palette and the pixel data.
	HANDLE hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = info.bmiHeader;
	HANDLE hPal, hOldPal = NULL;
	hPal = GetStockObject(DEFAULT_PALETTE);
	if (hPal) { hOldPal = SelectPalette(dc, (HPALETTE)hPal, FALSE); RealizePalette(dc); }
	GetDIBits(dc, bmp_handle, 0, (UINT)bitmap.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER) + dwPaletteSize, (BITMAPINFO*)lpbi, DIB_RGB_COLORS);
	if (hOldPal) { SelectPalette(dc, (HPALETTE)hOldPal, TRUE); RealizePalette(dc);	ReleaseDC(NULL, dc); }

	//Start writing the file.
	HANDLE fh = CreateFileA(filename, GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (fh == INVALID_HANDLE_VALUE) return false;
	BITMAPFILEHEADER bmfHdr;
	bmfHdr.bfType = 0x4D42;//"BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;
	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
	CloseHandle(fh);

	//Deallocation.
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	DeleteObject(bmp_handle);
	return true;
}



#endif //__FILE_IO_BMP_IO_H__#pragma once
