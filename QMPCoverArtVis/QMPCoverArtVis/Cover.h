//==Preprocessors==
#ifndef _COVER_H
#define _COVER_H

#define UNICODE
#include "QCDVisualsDLL.h"

//==Cover Loader Class==
class CoverLoader
{
public:
	//==Constructors==
	CoverLoader(PluginInfo* info);

	//==Destructor==
	~CoverLoader();	

	//==Other Methods==
	HBITMAP LoadCoverFromTag();
	HBITMAP LoadCoverFromTemplate(WCHAR* coverTemplate, WCHAR* coverFolder, 
								  WCHAR** extensions, int extensionSize);
	HBITMAP LoadCoverFromImageList(WCHAR* imageList);
	HBITMAP LoadCoverFromInternetTemplate(WCHAR* coverTemplate, WCHAR* server,
										  WCHAR* spaceReplace, WCHAR** extensions, 
										  int extensionSize);
	HBITMAP LoadNCAImage(WCHAR* noCoverImage);
	HBITMAP LoadRandomImage(WCHAR* folder, WCHAR* extensions[], int extensionSize);

private:
	//==Private Functions==
	bool TagGetAttachedCover(HANDLE mp3, long tagLength);
	long TagIsValid(HANDLE mp3);
	bool TagHasAttachedPicture(HANDLE mp3, long tagLength);
	int TagGetFrameSize(HANDLE mp3);
	bool TagSavePictureData(HANDLE mp3, int size, const WCHAR* fileName);
	bool TagFindData(HANDLE mp3, const unsigned char* data, int dataLength, 
					 long tagLength, int bytesToTake = 1);
	long FileGetChar(HANDLE file);
	void FilePutChar(HANDLE file, int character);
	HBITMAP LoadCover(const WCHAR* imageFile);
	HBITMAP LoadCoverFromInternet(WCHAR* url);
	void FindExtension(WCHAR** extensions, int extensionSize, WCHAR* fileName);
	void FindInternetExtension(WCHAR* extensions[], int extensionSize, WCHAR* url);
	void RemoveSpaces(WCHAR* buffer);
	void ReplaceSpaces(WCHAR* buffer, int bufferSize, WCHAR* replacement);
	void ReAddSpaces(WCHAR* buffer);
	void SaveTempCover(unsigned char* data, unsigned long size,
					   const WCHAR* extension, WCHAR* savedFile);
	void ConvertToUnicode(const char* source, WCHAR* dest, int destSize);
	void EliminateInvalidChars(const WCHAR* source, WCHAR* dest, int destSize);
	
	//==Private Members==
	ULONG_PTR gdiPlusToken;
	PluginInfo* qcdInfo;
};

#endif