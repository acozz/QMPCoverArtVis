//==Preprocessors==
#include "cover.h"
#include <string.h>
#include <vector>
#include <time.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <wininet.h>
#include "resource.h"
using namespace Gdiplus;
using std::vector;

//==Default Constructor==
CoverLoader::CoverLoader(PluginInfo* info)
{
	// Set the qcd plugin info pointer
	qcdInfo = info;

	// Start GDI+
	GdiplusStartupInput gdiPlusStartupInput;
   	GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, NULL);
}

//==Destructor==
CoverLoader::~CoverLoader()
{
	// Shutdown GDI+
	GdiplusShutdown(gdiPlusToken);

	// Free the PluginInfo pointer
	qcdInfo = NULL;
}

//==Load Cover From Tag Method==
HBITMAP CoverLoader::LoadCoverFromTag()
{
	// Get the file to read from
	char mediaFile[MAX_PATH];
	WCHAR wideMediaFile[MAX_PATH];
	qcdInfo->serviceFunc(opGetTrackFile, mediaFile, sizeof(mediaFile), -1);
	ConvertToUnicode(mediaFile, wideMediaFile, MAX_PATH);

	// Open the media file
	HANDLE media = CreateFile(wideMediaFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
							0, NULL);
	if(media == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	// Check if it is an ID3v2 tag and get its length
	bool coverSaved = false;
	long tagLength = TagIsValid(media);

	// Get the attached cover from the tag
	coverSaved = TagGetAttachedCover(media, tagLength);

	// Close the file
	CloseHandle(media);

	// Make sure the cover was saved
	if(!coverSaved)
	{
		return NULL;
	}
	
	// Load the cover
	char tempFile[MAX_PATH];
	WCHAR wideTempFile[MAX_PATH];
	qcdInfo->serviceFunc(opGetPluginFolder, tempFile, MAX_PATH, 0);
	ConvertToUnicode(tempFile, wideTempFile, MAX_PATH);
	lstrcat(wideTempFile, L"\\tempCover.jpg");
	return LoadCover(wideTempFile);
}

//==Tag Get Attached Cover From Tag Method==
bool CoverLoader::TagGetAttachedCover(HANDLE mp3, long tagLength)
{
	// Check if the tag has an attached picture
	if(TagHasAttachedPicture(mp3, tagLength))
	{
		// Get the frame size
		int frameSize;
		if((frameSize = TagGetFrameSize(mp3)) > 0)
		{
			// Skip the flags field (2 bytes)
			DWORD startFrame = SetFilePointer(mp3, 2, NULL, FILE_CURRENT);
			
			// Get the text encoding
			int textEncoding = FileGetChar(mp3);
			int nullStringSize = 1;
			if(textEncoding == 1)
			{
				nullStringSize = 2;
			}
			
			// Skip the mime type field
			unsigned char nullString[2] = {0};
			if(TagFindData(mp3, nullString, 1, tagLength))
			{
				// Get the picture type
				int picType = FileGetChar(mp3);

				// Save it if it is a cover
				if(picType == 3 || picType == 0)
				{
					// Skip the description field
					if(TagFindData(mp3, nullString, nullStringSize, tagLength, nullStringSize))
					{
						// Save the picture data
						int currentPos = SetFilePointer(mp3, 0, NULL, FILE_CURRENT);
						int picSize = frameSize - (currentPos - startFrame);
						char tempFile[MAX_PATH];
						WCHAR wideTempFile[MAX_PATH];
						qcdInfo->serviceFunc(opGetPluginFolder, tempFile, MAX_PATH, 0);
						ConvertToUnicode(tempFile, wideTempFile, MAX_PATH);
						lstrcat(wideTempFile, L"\\tempCover.jpg");
						return TagSavePictureData(mp3, picSize, wideTempFile);
					}
					else
					{
						return false;
					}
				}
				else
				{
					// It's not a cover, see if there is one later in the tag
					return TagGetAttachedCover(mp3, tagLength);
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

//==Tag Is Valid Method==
long CoverLoader::TagIsValid(HANDLE mp3)
{
	long length = 0;

	// Calculate the tag's length
	if(FileGetChar(mp3) == 'I' && FileGetChar(mp3) == 'D' && FileGetChar(mp3) == '3')
	{
		// Skip the version and flags (3 bytes total)
		SetFilePointer(mp3, 3, NULL, FILE_CURRENT);

		// Get the 4 length bytes
		for(int ctr = 0; ctr < 4; ctr++)
		{
			length *= 127 * ctr;
			length += FileGetChar(mp3);
		}
	}

	return length;
}

//==Tag Has Attached Picture Method==
bool CoverLoader::TagHasAttachedPicture(HANDLE mp3, long tagLength)
{
	return TagFindData(mp3, (unsigned char*)"APIC", 4, tagLength);
}

//==Get Tag Frame Size Method==
int CoverLoader::TagGetFrameSize(HANDLE mp3)
{
	// the next 4 bytes should be a hex value equalling the size of the frame
	int size = 0;
	for(int ctr = 0; ctr < 4; ctr++)
	{
		int hexDigit = FileGetChar(mp3);
		if(hexDigit == EOF)
		{
			return 0;
		}
		int hexHigh = hexDigit / 16;
		int hexLow = hexDigit - (hexHigh * 16);
		size = size * 16 + hexHigh;
		size = size * 16 + hexLow;
	}

	return size;
}

//==Save Tag Picture Data Method==
bool CoverLoader::TagSavePictureData(HANDLE mp3, int size, const WCHAR* fileName)
{
	HANDLE newFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
								FILE_ATTRIBUTE_NORMAL, NULL);
	if(newFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	int currentByte;

	for(int ctr = 0; ctr < size; ctr++)
	{
		currentByte = FileGetChar(mp3);
		if(currentByte == EOF)
		{
			CloseHandle(newFile);
			return false;
		}
		FilePutChar(newFile, currentByte);
	}

	CloseHandle(newFile);
	return true;
}

//==Find Tag Data Method==
bool CoverLoader::TagFindData(HANDLE mp3, const unsigned char* data, 
							  int dataLength, long tagLength, int bytesToTake)
{
	bool done = false;
	int ctr = 0;
	long character;
		
	while(ctr < dataLength && (character = FileGetChar(mp3)) != EOF)
	{
		if(character == data[ctr])
		{
			ctr++;
		}
		else
		{
			for(int ctr2 = 1; ctr2 < bytesToTake; ctr2++)
			{
				character = FileGetChar(mp3);
				if(character == EOF)
				{
					return false;
				}
			}
			ctr = 0;
		}

		if((int)SetFilePointer(mp3, 0, NULL, FILE_CURRENT) >= tagLength - 10)
		{
			return false;
		}
	}

	if(ctr == dataLength)
	{
		return true;
	}
	return false;
}

//==Get Character From File Method==
long CoverLoader::FileGetChar(HANDLE file)
{
	unsigned char buffer[1] = {0};
	DWORD bytesRead;

	if(!ReadFile(file, buffer, 1, &bytesRead, NULL))
	{
		return EOF;
	}

	if(bytesRead == 0)
	{
		return EOF;
	}

	return (long)buffer[0];
}

//==Put Character Into File Method==
void CoverLoader::FilePutChar(HANDLE file, int character)
{
	unsigned char buffer[1] = {character};
	DWORD bytesWritten;

	WriteFile(file, buffer, 1, &bytesWritten, NULL);
}

//==Load Cover From Template Method==
HBITMAP CoverLoader::LoadCoverFromTemplate(WCHAR* coverTemplate, WCHAR* coverFolder,
										   WCHAR** extensions, int extensionSize)
{
	// Remove any invalid characters in the file name
	//WCHAR correctFile[MAX_PATH];
	//int size = MAX_PATH;
	//EliminateInvalidChars(imageFile, correctFile, size);

	WCHAR coverFile[MAX_PATH];
	lstrcpy(coverFile, coverFolder);
	if(coverFile[lstrlen(coverFile) - 1] != 0x005C)
	{
		lstrcat(coverFile, L"\\");
	}
	int templateLength = lstrlen(coverTemplate);

	int ctr = 0;
	while(coverTemplate[ctr] != 0x0000 && ctr < templateLength)
	{
		switch(coverTemplate[ctr])
		{
		case 0x0025:	// '%'
			{
				if(ctr < templateLength - 1)
				{
					switch(coverTemplate[++ctr])
					{
					case 0x0041:
					case 0x0061:	// 'A' and 'a' - Artist
						{
							char artist[MAX_PATH] = {0};
							WCHAR artistW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetArtistName, artist, sizeof(artist), -1);
							ConvertToUnicode(artist, artistW, MAX_PATH);
							if(coverTemplate[ctr] == 0x0061)
							{
								RemoveSpaces(artistW);
							}
							WCHAR correctArtist[MAX_PATH];
							int size = MAX_PATH;
							EliminateInvalidChars(artistW, correctArtist, size);
							lstrcat(coverFile, correctArtist);
							break;
						}

					case 0x0044:
					case 0x0064:	// 'D' and 'd' - Disc
						{
							char disc[MAX_PATH] = {0};
							WCHAR discW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetDiscName, disc, sizeof(disc), -1);
							ConvertToUnicode(disc, discW, MAX_PATH);
							if(coverTemplate[ctr] == 0x0064)
							{
								RemoveSpaces(discW);
							}
							WCHAR correctDisc[MAX_PATH];
							int size = MAX_PATH;
							EliminateInvalidChars(discW, correctDisc, size);
							lstrcat(coverFile, correctDisc);
							break;
						}

					case 0x0054:
					case 0x0074:	 // 'T' and 't' - Track
						{
							char track[MAX_PATH] = {0};
							WCHAR trackW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetTrackName, track, sizeof(track), -1);
							ConvertToUnicode(track, trackW, MAX_PATH);
							if(coverTemplate[ctr] == 0x0074)
							{
								RemoveSpaces(trackW);
							}
							WCHAR correctTrack[MAX_PATH];
							int size = MAX_PATH;
							EliminateInvalidChars(trackW, correctTrack, size);
							lstrcat(coverFile, correctTrack);
							break;
						}

					case 0x0045:	// 'E' - Extensions
						{
							FindExtension(extensions, extensionSize, coverFile);
							break;
						}

					default:		// just a '%' character
						{
							if(coverTemplate[ctr] == 0x0025)
							{
								ctr--;
							}
							WCHAR character[2] = {0x0000};
							character[0] = coverTemplate[ctr];
							lstrcat(coverFile, character);
							break;
						}
					}
				}
				else	// last character is a '%'
				{
					WCHAR character[2] = {0x0000};
					character[0] = coverTemplate[ctr];
					lstrcat(coverFile, character);
				}
				
				break;
			}
		default:	// normal character
			{
				WCHAR character[2] = {0x0000};
				character[0] = coverTemplate[ctr];
				lstrcat(coverFile, character);
				break;
			}
		}
		
		ctr++;
	}

	// Load the cover
	return LoadCover(coverFile);
}

//==Load Cover From Image List Method==
HBITMAP CoverLoader::LoadCoverFromImageList(WCHAR* imageList)
{
	// Get the artist and album names
	char artist[MAX_PATH] = {0};
	char disc[MAX_PATH] = {0};
	WCHAR artistW[MAX_PATH] = {0x0000};
	WCHAR discW[MAX_PATH] = {0x0000};
	qcdInfo->serviceFunc(opGetArtistName, artist, sizeof(artist), -1);
	qcdInfo->serviceFunc(opGetDiscName, disc, sizeof(disc), -1);
	ConvertToUnicode(artist, artistW, MAX_PATH);
	ConvertToUnicode(disc, discW, MAX_PATH);

	// Replace spaces in album name with %20
	ReplaceSpaces(discW, MAX_PATH, L"_");

	// First search with a blank artist
	WCHAR fileName[MAX_PATH] = {0x0000};
	WCHAR blank[] = L"";
	GetPrivateProfileString(blank, discW, L"none", fileName, sizeof(fileName), imageList);
	if(lstrcmp(fileName, L"none") == 0)
	{
		// Search with the artist name
		GetPrivateProfileString(artistW, discW, L"none", fileName, sizeof(fileName), imageList);
		if(lstrcmp(fileName, L"none") == 0)
		{
			return NULL;
		}
	}

	// Re add spaces into file name
	if(wcsncmp(fileName + 1, L":\\", 2) == 0)
	{
		ReAddSpaces(fileName);
		return LoadCover(fileName);
	}

	return LoadCoverFromInternet(fileName);
}

//==Load Cover From Internet Template Method==
HBITMAP CoverLoader::LoadCoverFromInternetTemplate(WCHAR* coverTemplate, 
												   WCHAR* server, WCHAR* spaceReplace,
												   WCHAR** extensions, int extensionSize)
{
	WCHAR coverFile[MAX_PATH];
	lstrcpy(coverFile, server);
	if(coverFile[lstrlen(coverFile) - 1] != 0x005C)
	{
		lstrcat(coverFile, L"/");
	}
	int templateLength = lstrlen(coverTemplate);

	int ctr = 0;
	while(coverTemplate[ctr] != 0x0000 && ctr < templateLength)
	{
		switch(coverTemplate[ctr])
		{
		case 0x0025:	// '%'
			{
				if(ctr < templateLength - 1)
				{
					switch(coverTemplate[++ctr])
					{
					case 0x0041:	// 'A' - Artist
						{
							char artist[MAX_PATH] = {0};
							WCHAR artistW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetArtistName, artist, sizeof(artist), -1);
							ConvertToUnicode(artist, artistW, MAX_PATH);
							lstrcat(coverFile, artistW);
							break;
						}

					case 0x0044:	// 'D' - Disc
						{
							char disc[MAX_PATH] = {0};
							WCHAR discW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetDiscName, disc, sizeof(disc), -1);
							ConvertToUnicode(disc, discW, MAX_PATH);
							lstrcat(coverFile, discW);
							break;
						}

					case 0x0054:	// 'T' - Track
						{
							char track[MAX_PATH] = {0};
							WCHAR trackW[MAX_PATH] = {0x0000};
							qcdInfo->serviceFunc(opGetTrackName, track, sizeof(track), -1);
							ConvertToUnicode(track, trackW, MAX_PATH);
							lstrcat(coverFile, trackW);
							break;
						}

					case 0x0046:	// 'F' - File Name Without Extension
						{
							char fileName[MAX_PATH];
							WCHAR wideFileName[MAX_PATH];
							qcdInfo->serviceFunc(opGetTrackFile, fileName, sizeof(fileName), -1);
							ConvertToUnicode(fileName, wideFileName, MAX_PATH);
							int ctr;
							for(ctr = lstrlen(wideFileName) - 1; ctr >= 0; ctr--)
							{
								if(wideFileName[ctr] == 0x005c)
								{
									lstrcpy(wideFileName, wideFileName + (ctr + 1));
									break;
								}
							}
							for(ctr = lstrlen(wideFileName) - 1; ctr >= 0; ctr--)
							{
								if(wideFileName[ctr] == 0x002e)
								{
									wideFileName[ctr] = 0x0000;
									break;
								}
							}
							lstrcat(coverFile, wideFileName);
						}


					/*case 0x0045:	// 'E' - Extensions
						{
							FindInternetExtension(extensions, extensionSize, coverFile);
							break;
						}*/

					default:		// just a '%' character
						{
							if(coverTemplate[ctr] == 0x0025)
							{
								ctr--;
							}
							WCHAR character[2] = {0x0000};
							character[0] = coverTemplate[ctr];
							lstrcat(coverFile, character);
							break;
						}
					}
				}
				else	// last character is a '%'
				{
					WCHAR character[2] = {0x0000};
					character[0] = coverTemplate[ctr];
					lstrcat(coverFile, character);
				}
				
				break;
			}
		default:	// normal character
			{
				WCHAR character[2] = {0x0000};
				character[0] = coverTemplate[ctr];
				lstrcat(coverFile, character);
				break;
			}
		}
		
		ctr++;
	}

	// Replace the spaces in the url
	if(lstrlen(spaceReplace) > 0)
	{
		ReplaceSpaces(coverFile, MAX_PATH, spaceReplace);
	}
	else
	{
		RemoveSpaces(coverFile);
	}

	// Load the cover
	return LoadCoverFromInternet(coverFile);
}

//==Load No Cover Available Image Method==
HBITMAP CoverLoader::LoadNCAImage(WCHAR* noCoverImage)
{
	HBITMAP nca;
	if(lstrlen(noCoverImage) <= 0 || !(nca = LoadCover(noCoverImage)))
	{
		// Load default "No Cover Available" image
		nca = (HBITMAP)LoadImage((HINSTANCE)qcdInfo->context,
								 MAKEINTRESOURCE(IDB_NCA), IMAGE_BITMAP,
								 0, 0, LR_DEFAULTCOLOR);
	}

	return nca;
}

//==Load Random Image Method==
HBITMAP CoverLoader::LoadRandomImage(WCHAR* folder, WCHAR** extensions, int extensionSize)
{
	vector<WIN32_FIND_DATA> files;
	WCHAR fileToFind[MAX_PATH];
	WIN32_FIND_DATA findData;
	HANDLE searchHandle;
	if(folder[lstrlen(folder) - 1] != 0x005C)
	{
		lstrcat(folder, L"\\");
	}

	int ctr = 0;

	// Loop through the different extensions
	while(ctr < extensionSize)
	{
		// Create a search expression
		lstrcpy(fileToFind, folder);
		lstrcat(fileToFind, L"*.");
		lstrcat(fileToFind, extensions[ctr]);

		// Find the first file that matches the expression
		searchHandle = FindFirstFile(fileToFind, &findData);
		if(searchHandle != INVALID_HANDLE_VALUE)
		{
			// Add the file to the vector
			files.push_back(findData);

			// Search for the rest of the files matching the expression
			while(FindNextFile(searchHandle, &findData))
			{
				// Add the file to the vector
				files.push_back(findData);
			}

			// End the search
			FindClose(searchHandle);
			searchHandle = INVALID_HANDLE_VALUE;
		}

		ctr++;
	}

	// Choose a random cover
	if(files.size() > 0)
	{
		srand(unsigned(time(NULL)));
		int index = rand() % files.size();
		WCHAR fileToLoad[MAX_PATH];
		lstrcpy(fileToLoad, folder);
		lstrcat(fileToLoad, files[index].cFileName);

		// Load the cover
		return LoadCover(fileToLoad);
	}

	return NULL;
}

//==Load Cover Method==
HBITMAP CoverLoader::LoadCover(const WCHAR* imageFile)
{
	HBITMAP image;

	// Initialize a GDI+ Bitmap object with the image
	Bitmap imageLoader(imageFile);

	// Create a transparent background color
	Color color(0, 0, 0, 0);

	// Get the image handle
	Status status = imageLoader.GetHBITMAP(color, &image);
	if(status != Ok)
	{
		image = NULL;
	}

	// Return the image handle
	return image;
}

//==Load Cover From Internet Method==
HBITMAP CoverLoader::LoadCoverFromInternet(WCHAR* url)
{
	// Make sure QCD is online
	if(qcdInfo->serviceFunc(opGetOffline, NULL, 0, 0))
	{
		return NULL;
	}

	HINTERNET netHandle;
	DWORD id = 1;
	const WCHAR* CLASSNAME = L"QCD Cover Art Vis";

	// Establish an internet connection
	netHandle = InternetOpen(CLASSNAME, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if(netHandle == NULL)
	{
		return NULL;
	}
	
	// Delete http:// if necessary
	if(_wcsnicmp(url, L"http://", 7) == 0)
	{
		url += 7;
	}

	// Find the first occurence of the '/' character
	WCHAR* marker = wcschr(url, 0x002f);

	// Get the website from the beginning of the url to the marker
	int websiteLength = lstrlen(url) - lstrlen(marker) + 1;
	WCHAR* website = new WCHAR[websiteLength];
	lstrcpyn(website, url, websiteLength);

	// Connect to the website
	netHandle = InternetConnect(netHandle, website, INTERNET_DEFAULT_HTTP_PORT, CLASSNAME,
								NULL, INTERNET_SERVICE_HTTP, NULL, id);
	if(netHandle == NULL)
	{
		return NULL;
	}

	// Open a request for the picture
	WCHAR* acceptedTypes[2] = {0x0000};
	acceptedTypes[0] = L"image/*";
	netHandle = HttpOpenRequest(netHandle, NULL, ++marker, NULL, NULL,
								(LPCWSTR*)acceptedTypes, NULL, id);
	if(netHandle == NULL)
	{
		return NULL;
	}

	// Request the picture to be downloaded
	if(!HttpSendRequest(netHandle, NULL, 0, NULL, 0))
	{
		InternetCloseHandle(netHandle);
		return NULL;
	}

	// Read the picture file
	unsigned char inBuffer[30000];
	unsigned long numRead = 0;
	unsigned long totalRead = 0;
	do
	{
		if(!InternetReadFile(netHandle, (char*)inBuffer + totalRead,
							 sizeof(inBuffer) - totalRead, &numRead))
		{
			InternetCloseHandle(netHandle);
			return NULL;
		}

		totalRead += numRead;
	}
	while(numRead != 0);

	// Get the extension of the picture
	WCHAR* extension = wcsrchr(url, 0x002e);
	if(!extension)
	{
		InternetCloseHandle(netHandle);
		return NULL;
	}

	// Save the picture to a temporary file
	WCHAR filename[MAX_PATH];
	SaveTempCover(inBuffer, totalRead, extension, filename);

	// Clean up
	InternetCloseHandle(netHandle);

	// Load the cover
	return LoadCover(filename);
}

//==Remove Spaces Method==
void CoverLoader::RemoveSpaces(WCHAR* buffer)
{
	int ctr = 0;

	while(buffer[ctr] != 0x0000)
	{
		if(buffer[ctr] == 0x0020)
		{
			int ctr2 = ctr;
			while(buffer[ctr2] != 0x0000)
			{
				buffer[ctr2] = buffer[ctr2 + 1];
				ctr2++;
			}
		}
		ctr++;
	}
}

//==Replace Spaces Method==
void CoverLoader::ReplaceSpaces(WCHAR* buffer, int bufferSize, WCHAR* replacement)
{
	// Create a temporary buffer
	WCHAR* tempBuffer = new WCHAR[bufferSize];
	lstrcpy(tempBuffer, L"");

	// Loop through the buffer
	int ctr = 0;
	while(buffer[ctr] != 0x0000)
	{
		if(buffer[ctr] == 0x0020)
		{
			// Replace the space character
			lstrcat(tempBuffer, replacement);
		}
		else
		{
			// Add the character to the final string
			WCHAR character[2] = {0x0000};
			character[0] = buffer[ctr];
			lstrcat(tempBuffer, character);
		}

		ctr++;
	}

	// Copy the spaceless buffer to the main buffer
	lstrcpy(buffer, tempBuffer);
}

//==Re Add Spaces Method==
void CoverLoader::ReAddSpaces(WCHAR* buffer)
{
	int size = lstrlen(buffer) - 3;
	for(int ctr = 0; ctr < size; ctr++)
	{
		if(buffer[ctr] == 0x0025 && buffer[ctr + 1] == 0x0032 && buffer[ctr + 2] == 0x0030)
		{
			buffer[ctr] = 0x0020;
			buffer[ctr + 1] = 0x0000;
			lstrcat(buffer, buffer + ctr + 3);
		}
	}			
}

//==Find Extension Method==
void CoverLoader::FindExtension(WCHAR* extensions[], int extensionSize, WCHAR* fileName)
{
	WCHAR tempFileName[MAX_PATH];
	
	int ctr = 0;
	bool foundCover = false;

	while(ctr < extensionSize && !foundCover)
	{
		lstrcpy(tempFileName, fileName);
		lstrcat(tempFileName, extensions[ctr]);
		
		HANDLE fileHandle = CreateFile(tempFileName, GENERIC_READ, FILE_SHARE_READ,
									   NULL, OPEN_EXISTING, 0, NULL);
		if(fileHandle != INVALID_HANDLE_VALUE)
		{
			foundCover = true;
			lstrcat(fileName, extensions[ctr]);
		}

		ctr++;
	}
}

//==Find Internet Extension Method==
void CoverLoader::FindInternetExtension(WCHAR* extensions[], int extensionSize, WCHAR* url)
{
	// Make sure QCD is online
	if(qcdInfo->serviceFunc(opGetOffline, NULL, 0, 0))
	{
		return;
	}

	HINTERNET netHandle;
	DWORD id = 1;
	const WCHAR* CLASSNAME = L"QCD Cover Art Vis";

	// Establish an internet connection
	netHandle = InternetOpen(CLASSNAME, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, NULL);
	if(netHandle == NULL)
	{
		return;
	}
	
	// Delete http:// if necessary
	if(_wcsnicmp(url, L"http://", 7) == 0)
	{
		url += 7;
	}

	// Find the first occurence of the '/' character
	WCHAR* marker = wcschr(url, 0x002f);

	// Get the website from the beginning of the url to the marker
	int websiteLength = lstrlen(url) - lstrlen(marker) + 1;
	WCHAR* website = new WCHAR[websiteLength];
	lstrcpyn(website, url, websiteLength);

	// Connect to the website
	netHandle = InternetConnect(netHandle, website, INTERNET_DEFAULT_HTTP_PORT, CLASSNAME,
								NULL, INTERNET_SERVICE_HTTP, NULL, id);
	if(netHandle == NULL)
	{
		return;
	}

	// Try to find the picture's extension
	WCHAR tempURL[MAX_PATH];
	
	int ctr = 0;
	bool foundCover = false;

	while(ctr < extensionSize && !foundCover)
	{
		lstrcpy(tempURL, url);
		lstrcat(tempURL, extensions[ctr]);
		
		// Open a request for the picture
		WCHAR* acceptedTypes[2] = {0x0000};
		acceptedTypes[0] = L"image/*";
		MessageBox(NULL, tempURL, L"looking for", MB_OK);
		netHandle = HttpOpenRequest(netHandle, NULL, ++marker, NULL, NULL,
									(LPCWSTR*)acceptedTypes, NULL, id);
		if(netHandle == NULL)
		{
			return;
		}

		// Request the picture to be downloaded
		if(!HttpSendRequest(netHandle, NULL, 0, NULL, 0))
		{
			InternetCloseHandle(netHandle);
			return;
		}

		// Read the picture file
		unsigned char inBuffer[1];
		unsigned long numRead = 0;
		InternetReadFile(netHandle, (char*)inBuffer, sizeof(inBuffer), &numRead);
		if(numRead > 0)
		{
			MessageBox(NULL, L"found it", L"debug", MB_OK);
			foundCover = true;
			lstrcat(url, extensions[ctr]);			
		}

		ctr++;
	}

	// Close connection to the internet
	InternetCloseHandle(netHandle);
}

//==Save Temp Cover Method==
void CoverLoader::SaveTempCover(unsigned char* data, unsigned long size,
								const WCHAR* extension, WCHAR* savedFile)
{
	// Create a file name for the cover
	WCHAR filename[MAX_PATH];
	char tempFile[MAX_PATH];
	WCHAR wideTempFile[MAX_PATH];
	qcdInfo->serviceFunc(opGetPluginFolder, tempFile, MAX_PATH, 0);
	ConvertToUnicode(tempFile, wideTempFile, MAX_PATH);
	lstrcat(wideTempFile, L"\\tempCover");
	lstrcpy(filename, wideTempFile);
	lstrcat(filename, extension);
	
	// Create the cover file
	HANDLE file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ,
							 NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == NULL)
	{
		lstrcpy(savedFile, L"");
		return;
	}

	// Write the image data to the file
	DWORD bytesWritten = 0;
	DWORD totalWritten = 0;
	do
	{
		if(!WriteFile(file, data + totalWritten, size - totalWritten, &bytesWritten, NULL))
		{
			lstrcpy(savedFile, L"");
			return;
		}

		totalWritten += bytesWritten;
	}
	while(bytesWritten != 0);

	// Clean up
	CloseHandle(file);

	// Copy the file name to the passed argument
	lstrcpy(savedFile, filename);
}

//==Convert To Unicode Method==
void CoverLoader::ConvertToUnicode(const char* source, WCHAR* dest, int destSize)
{
	// Get the Windows Version
	OSVERSIONINFO osVersionInfo = {0};
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);
	
	if(osVersionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
		// We're on NT or above so use QCD's conversion technique
		qcdInfo->serviceFunc(opUTF8toUCS2, (void*)source, (long)dest, destSize);
	}
	else
	{
		// We're on Win9x
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, dest, destSize);
	}
}

//==Eliminate Invalid Characters Method==
void CoverLoader::EliminateInvalidChars(const WCHAR* source, WCHAR* dest, int destSize)
{
	// Keep an index in the destination buffer
	int destIndex = 0;

	// Loop through the source buffer
	int sourceLength = lstrlen(source);
	for(int ctr = 0; ctr < sourceLength; ctr++)
	{
		// invalid characters = \/:*?"<>|
		WCHAR currentChar = source[ctr];
		if(currentChar != 0x005c && // '\' 
		   currentChar != 0x002f && // '/' 
		   currentChar != 0x003a && // ':' 
		   currentChar != 0x002a && // '*' 
		   currentChar != 0x003f && // '?' 
		   currentChar != 0x0022 && // '\"' 
		   currentChar != 0x003c && // '<' 
		   currentChar != 0x003e && // '>' 
		   currentChar != 0x007c && // '|'
		   destIndex < destSize - 1)
		{
			dest[destIndex++] = currentChar;
		}
	}

	dest[destIndex] = 0x0000;
}
