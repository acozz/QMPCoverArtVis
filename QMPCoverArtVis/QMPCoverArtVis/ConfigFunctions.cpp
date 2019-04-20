//==Preprocessors==
#include "ConfigFunctions.h"
#include "resource.h"
#include <string>
using std::string;

//==Initialize Options Function==
void InitializeOptions(PluginInfo* info)
{
	// Get the initialization file name
	WCHAR wideFileName[MAX_PATH];
	char fileName[MAX_PATH];
	info->serviceFunc(opGetPluginSettingsFile, fileName, sizeof(fileName), 0);

	// Convert the file name to unicode
	ConvertToUnicode(info, fileName, wideFileName, MAX_PATH);

	// Set default options
	int ctr;
	for(ctr = 0; ctr < FORMAT_LENGTH; ctr++)
	{
		coverOptions.formatPriority[ctr] = (FileFormats)ctr;
	}
	coverOptions.loadingPriority[0] = ID3;
	for(ctr = 1; ctr < METHOD_LENGTH; ctr++)
	{
		coverOptions.loadingPriority[ctr] = (LoadingMethods)(ctr - 1);
	}
	lstrcpy(coverOptions.customNoCoverImage, L"");
	lstrcpy(coverOptions.templateString, L"folder.%E");
	lstrcpy(coverOptions.templateFolder, L"");
	coverOptions.useTemplateFolder = false;
	char imageList[MAX_PATH];
	WCHAR wideImageList[MAX_PATH];
	info->serviceFunc(opGetPluginFolder, imageList, MAX_PATH, 0);
	ConvertToUnicode(info, imageList, wideImageList, MAX_PATH);
	lstrcat(wideImageList, L"\\imagelist.txt");
	lstrcpy(coverOptions.imageListFile, wideImageList);
	lstrcpy(coverOptions.imageListSpaceChar, L"%20");
	lstrcpy(coverOptions.netTemplateString, L"");
	lstrcpy(coverOptions.netTemplateAddress, L"");
	lstrcpy(coverOptions.netTemplateSpaceChar, L"%20");
	lstrcpy(coverOptions.randomImageFolder, L"");
	lstrcpy(coverOptions.customNoCoverImage, L"");
	coverOptions.useCustomNoCoverImage = false;
	coverOptions.usePluginMenu = true;

	// Load options
	GetPrivateProfileStruct(L"QCDCoverArtVis", L"Settings", (void*)&coverOptions,
						    sizeof(CoverOptions), wideFileName);
}

//==Save Options Function==
void SaveOptions(PluginInfo* info)
{
	// Get the initialization file name
	WCHAR wideFileName[MAX_PATH];
	char fileName[MAX_PATH];
	info->serviceFunc(opGetPluginSettingsFile, fileName, sizeof(fileName), 0);

	// Convert the file name to unicode
	ConvertToUnicode(info, fileName, wideFileName, MAX_PATH);

	// Save options
	WritePrivateProfileStruct(L"QCDCoverArtVis", L"Settings", (void*)&coverOptions,
							  sizeof(CoverOptions), wideFileName);
}

//==Get Extensions Strings Function==
void GetExtensionStrings(WCHAR** extensionBuffer)
{
	if(extensionBuffer)
	{
		bool offset = false;

		for(int ctr = 0; ctr < FORMAT_LENGTH + 1; ctr++)
		{
			FileFormats format = BMP;
			if(offset)
			{
				 format = coverOptions.formatPriority[ctr - 1];
			}
			else
			{
				format = coverOptions.formatPriority[ctr];
			}
			switch(format)
			{
			case BMP:
				{
					lstrcpy(extensionBuffer[ctr], L"bmp");
					break;
				}

			case GIF:
				{
					lstrcpy(extensionBuffer[ctr], L"gif");
					break;
				}

			case JPG:
				{
					lstrcpy(extensionBuffer[ctr], L"jpg");
					lstrcpy(extensionBuffer[++ctr], L"jpeg");
					offset = true;
					break;
				}

			case EXIF:
				{
					lstrcpy(extensionBuffer[ctr], L"exif");
					break;
				}

			case PNG:
				{
					lstrcpy(extensionBuffer[ctr], L"png");
					break;
				}

			case TIFF:
				{
					lstrcpy(extensionBuffer[ctr], L"tiff");
					break;
				}
			}
		}
	}
}

//==Listbox Move Up Function==
void ListboxMoveUp(HWND listBox)
{
	int index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	if(index > 0)
	{
		WCHAR itemToMove[MAX_PATH];
		WCHAR itemToReplace[MAX_PATH];

		// Get the item to move's info
		int dataToMove = SendMessage(listBox, LB_GETITEMDATA, index, 0);
		SendMessage(listBox, LB_GETTEXT, index, (LPARAM)itemToMove);

		// Get the item to replace's info
		int dataToReplace = SendMessage(listBox, LB_GETITEMDATA, index - 1, 0);
		SendMessage(listBox, LB_GETTEXT, index - 1, (LPARAM)itemToReplace);

		// Delete the items
		SendMessage(listBox, LB_DELETESTRING, index - 1, 0);
		SendMessage(listBox, LB_DELETESTRING, index - 1, 0);

		// Add the items back in the correct order
		SendMessage(listBox, LB_INSERTSTRING, index - 1, (LPARAM)itemToMove);
		SendMessage(listBox, LB_INSERTSTRING, index, (LPARAM)itemToReplace);
		SendMessage(listBox, LB_SETITEMDATA, index - 1, dataToMove);
		SendMessage(listBox, LB_SETITEMDATA, index, dataToReplace);

		// Reselect the moved item
		SendMessage(listBox, LB_SETCURSEL, index - 1, 0);
	}
}

//==Listbox Move Down Function==
void ListboxMoveDown(HWND listBox)
{
	int index = SendMessage(listBox, LB_GETCURSEL, 0, 0);
	if(index != LB_ERR && index < SendMessage(listBox, LB_GETCOUNT, 0, 0) - 1)
	{
		WCHAR itemToMove[MAX_PATH];
		WCHAR itemToReplace[MAX_PATH];

		// Get the item to move's info
		int dataToMove = SendMessage(listBox, LB_GETITEMDATA, index, 0);
		SendMessage(listBox, LB_GETTEXT, index, (LPARAM)itemToMove);

		// Get the item to replace's info
		int dataToReplace = SendMessage(listBox, LB_GETITEMDATA, index + 1, 0);
		SendMessage(listBox, LB_GETTEXT, index + 1, (LPARAM)itemToReplace);

		// Delete the items
		SendMessage(listBox, LB_DELETESTRING, index, 0);
		SendMessage(listBox, LB_DELETESTRING, index, 0);

		// Add the items back in the correct order
		SendMessage(listBox, LB_INSERTSTRING, index, (LPARAM)itemToReplace);
		SendMessage(listBox, LB_INSERTSTRING, index + 1, (LPARAM)itemToMove);
		SendMessage(listBox, LB_SETITEMDATA, index, dataToReplace);
		SendMessage(listBox, LB_SETITEMDATA, index + 1, dataToMove);

		// Reselect the moved item
		SendMessage(listBox, LB_SETCURSEL, index + 1, 0);
	}
}

//==Initialize Priority Dialog Function==
void PriorityDlgInit(HWND dlg)
{
	// Get the controls to modify
	HWND formatPriorityList = GetDlgItem(dlg, IDC_PRIORITY_FORMAT);
	HWND methodPriorityList = GetDlgItem(dlg, IDC_PRIORITY_METHOD);

	int ctr;

	// Set the format priority list's initial settings
	for(ctr = 0; ctr < FORMAT_LENGTH; ctr++)
	{
		FileFormats itemData = coverOptions.formatPriority[ctr];
		WCHAR itemName[MAX_PATH];

		switch(itemData)
		{
		case BMP:
			{
				lstrcpy(itemName, L"BMP");
				break;
			}

		case GIF:
			{
				lstrcpy(itemName, L"GIF");
				break;
			}

		case JPG:
			{
				lstrcpy(itemName, L"JPG/JPEG");
				break;
			}

		case EXIF:
			{
				lstrcpy(itemName, L"EXIF");
				break;
			}

		case PNG:
			{
				lstrcpy(itemName, L"PNG");
				break;
			}

		case TIFF:
			{
				lstrcpy(itemName, L"TIFF");
				break;
			}
		}

		// Add the list box item
		SendMessage(formatPriorityList, LB_ADDSTRING, 0, (LPARAM)itemName);
		SendMessage(formatPriorityList, LB_SETITEMDATA, (WPARAM)ctr, (LPARAM)itemData);
	}

	// Set the cover loading method list's initial settings
	for(ctr = 0; ctr < METHOD_LENGTH; ctr++)
	{
		LoadingMethods itemData = coverOptions.loadingPriority[ctr];
		WCHAR itemName[MAX_PATH];

		switch(itemData)
		{
		case TEMPLATE:
			{
				lstrcpy(itemName, L"Filename Template");
				break;
			}
		case IMAGELIST:
			{
				lstrcpy(itemName, L"Image List");
				break;
			}
		case INTERNET:
			{
				lstrcpy(itemName, L"Internet Template");
				break;
			}
		case NCA:
			{
				lstrcpy(itemName, L"No Cover Image");
				break;
			}
		case RANDOM:
			{
				lstrcpy(itemName, L"Random Image");
				break;
			}
		case ID3:
			{
				lstrcpy(itemName, L"ID3v2 Tag");
				break;
			}
		}

		// Add the list box item
		SendMessage(methodPriorityList, LB_ADDSTRING, 0, (LPARAM)itemName);
		SendMessage(methodPriorityList, LB_SETITEMDATA, (WPARAM)ctr, (LPARAM)itemData);
	}
}

//==Save Priority Values Function==
void SavePriorityValues(HWND dlg)
{
	HWND methodList = GetDlgItem(dlg, IDC_PRIORITY_METHOD);
	HWND formatList = GetDlgItem(dlg, IDC_PRIORITY_FORMAT);
	int ctr;

	// Save format priority
	for(ctr = 0; ctr < FORMAT_LENGTH; ctr++)
	{
		coverOptions.formatPriority[ctr] = (FileFormats)
										   SendMessage(formatList, LB_GETITEMDATA, ctr, 0);
	}

	// Save method priority
	for(ctr = 0; ctr < METHOD_LENGTH; ctr++)
	{
		coverOptions.loadingPriority[ctr] = (LoadingMethods)
										    SendMessage(methodList, LB_GETITEMDATA, ctr, 0);
	}
}

//==Browse Function==
const WCHAR* Browse(HWND dlg, WCHAR* filter)
{
	// Create a buffer to store the file name
	static WCHAR fileName[MAX_PATH];
	lstrcpy(fileName, L"");

	// Create an OPENFILENAME structure
	OPENFILENAME openParams = {0};
	openParams.lStructSize = sizeof(OPENFILENAME);
	openParams.hwndOwner = dlg;
	openParams.lpstrFilter = filter;
	openParams.lpstrFile = fileName;
	openParams.nMaxFile = MAX_PATH;
	openParams.lpstrInitialDir = NULL;	
	openParams.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST;

	// Get the file to open
	if(!GetOpenFileName(&openParams))
	{
		lstrcpy(fileName, L"cancel");
	}

	// Return the file name
	return fileName;
}

//==Image List Dialog Initialization Function==
void ImageListDlgInit(HWND dlg)
{
	SetDlgItemText(dlg, IDC_IMAGELISTFILE, coverOptions.imageListFile);
}

//==Image List Use Current Song Information Function==
void ImageListUseCurrent(HWND dlg, PluginInfo* info)
{
	WCHAR wideArtist[MAX_PATH];
	WCHAR wideAlbum[MAX_PATH];
	char artist[MAX_PATH];
	char album[MAX_PATH];

	// Get the current file's info
	info->serviceFunc(opGetArtistName, artist, sizeof(artist), -1);
	info->serviceFunc(opGetDiscName, album, sizeof(album), -1);

	// Convert the info to unicode
	ConvertToUnicode(info, artist, wideArtist, MAX_PATH);
	ConvertToUnicode(info, album, wideAlbum, MAX_PATH);

	// Set the dialog control text
	SetDlgItemText(dlg, IDC_ARTIST, wideArtist);
	SetDlgItemText(dlg, IDC_ALBUM, wideAlbum);
}

//==Image List Write Function==
void ImageListWrite(HWND dlg)
{
	WCHAR imageList[MAX_PATH];
	WCHAR artist[MAX_PATH];
	WCHAR album[MAX_PATH];
	WCHAR image[MAX_PATH];

	// Get the information to write
	GetDlgItemText(dlg, IDC_IMAGELISTFILE, imageList, sizeof(imageList));
	GetDlgItemText(dlg, IDC_ARTIST, artist, sizeof(artist));
	GetDlgItemText(dlg, IDC_ALBUM, album, sizeof(album));
	GetDlgItemText(dlg, IDC_FILENAME, image, sizeof(image));

	// Replace spaces from the image file name
	ReplaceSpaces(image, sizeof(image), L"%20");

	// Replace aritist spaces with underscores
	ReplaceSpaces(album, sizeof(artist), L"_");

	// Write to the image list
	if(WritePrivateProfileString(artist, album, image, imageList))
	{
		SetDlgItemText(dlg, IDC_SUCCESS, L"Entry Written Successfully");
	}
	else
	{
		SetDlgItemText(dlg, IDC_SUCCESS, L"Entry Failed To Be Written");
	}
}

//==Image List Read Function==
void ImageListRead(HWND dlg)
{
	WCHAR imageList[MAX_PATH];
	WCHAR artist[MAX_PATH];
	WCHAR album[MAX_PATH];
	WCHAR image[MAX_PATH];
	lstrcpy(image, L"");

	// Get the information to read
	GetDlgItemText(dlg, IDC_IMAGELISTFILE, imageList, sizeof(imageList));
	GetDlgItemText(dlg, IDC_ARTIST, artist, sizeof(artist));
	GetDlgItemText(dlg, IDC_ALBUM, album, sizeof(album));

	// Replace artist spaces with underscores
	ReplaceSpaces(album, sizeof(artist), L"_");
	
	// Read from the image list
	GetPrivateProfileString(artist, album, L"", image, sizeof(image), imageList);

	// Add the spaces back into the file name
	ReAddSpaces(image);

	// Set the file name text
	SetDlgItemText(dlg, IDC_FILENAME, image);
}

//==Save Image List Values Function==
void SaveImageListValues(HWND dlg)
{
	GetDlgItemText(dlg, IDC_IMAGELISTFILE, coverOptions.imageListFile, 
				   sizeof(coverOptions.imageListFile));
}

//==Template Dialog Initialization Function==
void TemplateDlgInit(HWND dlg)
{
	SetDlgItemText(dlg, IDC_TEMPLATE_ROOT, coverOptions.templateFolder);
	SetDlgItemText(dlg, IDC_TEMPLATE, coverOptions.templateString);
	CheckRadioButton(dlg, IDC_TEMPLATE_USEFILEDIR, IDC_TEMPLATE_USEDIR,
					 IDC_TEMPLATE_USEFILEDIR);
	if(coverOptions.useTemplateFolder)
	{
		CheckRadioButton(dlg, IDC_TEMPLATE_USEFILEDIR, IDC_TEMPLATE_USEDIR,
					 IDC_TEMPLATE_USEDIR);
	}
}

//==Folder Browse Function==
const WCHAR* FolderBrowse(HWND dlg)
{
	static WCHAR folder[MAX_PATH];
	lstrcpy(folder, L"");

	// Fill a BROWSEINFO structure
	BROWSEINFO browseInfo = {0};
	browseInfo.hwndOwner = dlg;
	browseInfo.lpszTitle = L"Select a folder...";

	// Allow the user to choose a folder
	LPITEMIDLIST idList;
	if(!(idList = SHBrowseForFolder(&browseInfo)))
	{
		lstrcpy(folder, L"cancel");
	}
	else
	{
		// Get the name of the folder
		SHGetPathFromIDList(idList, folder);
		
		// Free memory used
		IMalloc* imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(idList);
			imalloc->Release();
		}	
	}
	return folder;
}

//==Save Template Values Function==
void SaveTemplateValues(HWND dlg)
{
	GetDlgItemText(dlg, IDC_TEMPLATE_ROOT, coverOptions.templateFolder, 
				   sizeof(coverOptions.templateFolder));
	GetDlgItemText(dlg, IDC_TEMPLATE, coverOptions.templateString,
				   sizeof(coverOptions.templateString));
	coverOptions.useTemplateFolder = IsDlgButtonChecked(dlg, IDC_TEMPLATE_USEDIR);
}

//==Internet Template Dialog Initialization Function==
void InternetDlgInit(HWND dlg)
{
	SetDlgItemText(dlg, IDC_INTERNET_ROOT, coverOptions.netTemplateAddress);
	SetDlgItemText(dlg, IDC_INTERNET_TEMPLATE, coverOptions.netTemplateString);
	SetDlgItemText(dlg, IDC_INTERNET_SPACECHAR, coverOptions.netTemplateSpaceChar);
}

//==Save Internet Values Function==
void SaveInternetValues(HWND dlg)
{
	GetDlgItemText(dlg, IDC_INTERNET_ROOT, coverOptions.netTemplateAddress,
				   sizeof(coverOptions.netTemplateAddress));
	GetDlgItemText(dlg, IDC_INTERNET_TEMPLATE, coverOptions.netTemplateString,
				   sizeof(coverOptions.netTemplateString));
	GetDlgItemText(dlg, IDC_INTERNET_SPACECHAR, coverOptions.netTemplateSpaceChar,
				   sizeof(coverOptions.netTemplateSpaceChar));
}

//==Random Image Dialog Initialization Function==
void RandomImageDlgInit(HWND dlg)
{
	SetDlgItemText(dlg, IDC_RANDOMDIR, coverOptions.randomImageFolder);
}

//==Save Random Image Values Function==
void SaveRandomImageValues(HWND dlg)
{
	GetDlgItemText(dlg, IDC_RANDOMDIR, coverOptions.randomImageFolder, 
				   sizeof(coverOptions.randomImageFolder));
}

//==No Cover Dialog Initialization Function==
void NoCoverDlgInit(HWND dlg)
{
	SetDlgItemText(dlg, IDC_NCAFILE, coverOptions.customNoCoverImage);
	if(coverOptions.useCustomNoCoverImage)
	{
		CheckDlgButton(dlg, IDC_USEMYNCA, BST_CHECKED);
	}
}

//==Save No Cover Values Function==
void SaveNoCoverValues(HWND dlg)
{
	GetDlgItemText(dlg, IDC_NCAFILE, coverOptions.customNoCoverImage,
				   sizeof(coverOptions.customNoCoverImage));
	coverOptions.useCustomNoCoverImage = IsDlgButtonChecked(dlg, IDC_USEMYNCA);
}

//==Other Options Dialog Initialization Function==
void OtherOptionsDlgInit(HWND dlg)
{
	if(coverOptions.usePluginMenu)
	{
		CheckDlgButton(dlg, IDC_PLUGINMENUENTRY, BST_CHECKED);
	}
}

//==Save Other Options Values Function==
void SaveOtherOptionsValues(HWND dlg)
{
	coverOptions.usePluginMenu = IsDlgButtonChecked(dlg, IDC_PLUGINMENUENTRY);
}

//==Remove Spaces Function==
void RemoveSpaces(WCHAR* buffer)
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

//==Replace Spaces Function==
void ReplaceSpaces(WCHAR* buffer, int bufferSize, WCHAR* replacement)
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

//==Re Add Spaces Function==
void ReAddSpaces(WCHAR* buffer)
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

//==Convert To Unicode Function==
void ConvertToUnicode(PluginInfo* info, const char* source, WCHAR* dest, int destSize)
{
	// Get the Windows Version
	OSVERSIONINFO osVersionInfo = {0};
	osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osVersionInfo);
	
	if(osVersionInfo.dwPlatformId >= VER_PLATFORM_WIN32_NT)
	{
		// We're on NT or above so use QCD's conversion technique
		info->serviceFunc(opUTF8toUCS2, (void*)source, (long)dest, destSize);
	}
	else
	{
		// We're on Win9x
		MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, source, -1, dest, destSize);
	}
}
