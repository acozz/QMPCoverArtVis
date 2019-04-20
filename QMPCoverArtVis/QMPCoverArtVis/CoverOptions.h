//==Preprocessors==
#ifndef _COVER_OPTIONS_H
#define _COVER_OPTIONS_H

#define UNICODE
#include <windows.h>

//==File Format Enumeration==
enum FileFormats{BMP, GIF, JPG, EXIF, PNG, TIFF};

//==Cover Loading Method Enumeration==
enum LoadingMethods{TEMPLATE, IMAGELIST, INTERNET, NCA, RANDOM, ID3};

//==Constants==
const int FORMAT_LENGTH = 6;
const int METHOD_LENGTH = 6;

//==Cover Options Structure==
struct CoverOptions
{
	// Priority Options
	FileFormats formatPriority[FORMAT_LENGTH];
	LoadingMethods loadingPriority[METHOD_LENGTH];

	// Image List Options
	WCHAR imageListFile[MAX_PATH];
	WCHAR imageListSpaceChar[MAX_PATH];

	// Template Options
	WCHAR templateString[MAX_PATH];
	WCHAR templateFolder[MAX_PATH];
	bool useTemplateFolder;

	// Internet Template Options
	WCHAR netTemplateString[MAX_PATH];
	WCHAR netTemplateAddress[MAX_PATH];
	WCHAR netTemplateSpaceChar[MAX_PATH];

	// Random Image Options
	WCHAR randomImageFolder[MAX_PATH];

	// No Cover Available Options
	WCHAR customNoCoverImage[MAX_PATH];
	bool useCustomNoCoverImage;

	// Other Options
	bool usePluginMenu;
};

#endif