//-----------------------------------------------------------------------------
//
// File:	QCDVisualsDLL.cpp
//
// About:	See QCDVisualsDLL.h
//
// Authors:	Written by Paul Quinn and Richard Carlson.
//
//	QCD multimedia player application Software Development Kit Release 1.0.
//
//	Copyright (C) 1997-2002 Quinnware
//
//	This code is free.  If you redistribute it in any form, leave this notice 
//	here.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//-----------------------------------------------------------------------------

// QCD Visual Plugin - Cover Art Vis
// Written by Anthony Cozzolino
// Copyright 2004

//==Preprocessors==
#include <vector>
using std::vector;
#include "resource.h"
#include "ConfigFunctions.h"
#include "Cover.h"
#include <prsht.h>

//==Prototypes==
HBITMAP LoadCover();
void BlitToVis();
void SetPluginMenuEntry();
BOOL CALLBACK AboutProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
void PropertySheetInit(HWND parentWindow, HINSTANCE pluginInst);
BOOL CALLBACK PriorityProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ImageListProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK TemplateProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK InternetProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK RandomImageProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK NoCoverProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK OtherOptionsProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam);

//==Global Variables==
const char* PLUGIN_NAME = "Cover Art Visual Plugin 2.972";
CoverLoader* coverLoader = NULL;
CoverOptions coverOptions;
static Raster *rasterBuffer;
bool shouldRedraw = false;
bool shouldInit = true;

//==QCD Global Variables==
static PluginInfo*			info;

// TODO : static ResonatorInfo		resBank;

//-----------------------------------------------------------------------------

//
// If there are multiple visualizations, each will have its own entry point
// as VISUALDLL_ENTRY_POINT(n), where n is the index of the visualization
// in your visualization pack. If there is to be only one visualization,
// this line need never change.
// 

PLUGIN_API int VISUALDLL_ENTRY_POINT(0)(PluginInfo *pluginInfo, QCDModInfo *modInfo, int fullinit)
{
	//
	// Visual plugin initialization will be called to obtain basic information
	// (moduleString, version, about and configure) when fullinit == FALSE.
	// When fullinit == TRUE, perform a full initialization required to launch
	// visualization.
	//

	if (modInfo)
		modInfo->moduleString = (char*)PLUGIN_NAME;
	
	if (pluginInfo)
	{
		// Setup the function pointers
		pluginInfo->version		= PLUGIN_API_VERSION_WANTUTF8;
		pluginInfo->about		= About;
		pluginInfo->configure	= Configure;
		pluginInfo->event		= Event;
		pluginInfo->render		= Render;
		pluginInfo->idle		= Idle;
		pluginInfo->wrap		= Wrap;

		// keep a copy of the pluginInfo struct since it will be the
		// means of getting analyzation data and player callbacks.
		// This is the one time it gets passed to the plugin.
		info = pluginInfo;
		rasterBuffer = info->buffer;
		
		if (fullinit) 
		{
			//
			// TODO : Set up analysis controls
			//
			// SAMPLE :
			// pluginInfo->triggerMode   = noTrigger;
			// pluginInfo->triggerForm   = 0;
			// pluginInfo->resonatorMode = customResonators;
			// pluginInfo->resonatorForm = (long)&resBank;
			// pluginInfo->vuMeterMode   = monoVUMeter;
			// pluginInfo->ppMeterMode   = 0;

			pluginInfo->triggerMode = noTrigger;
			pluginInfo->triggerForm = 0;
			pluginInfo->resonatorMode = noResonators;
			pluginInfo->resonatorForm = 0;
			pluginInfo->vuMeterMode = noVUMeter;
			pluginInfo->ppMeterMode = noPPMeter;

			//
			// TODO : Set up resonator bank
			//
			// SAMPLE :
			// resBank.resonatorMode	= monoResonators;
			// resBank.resonatorForm	= resonators;
			// resBank.pitch			= RESONATORPITCH;
			// resBank.width			= RESONATORWIDTH;
			// resBank.fixedsampling	= !0;
			// resBank.decimation		= DECIMATION;
			// resBank.next				= 0;
		}
	}

	if(shouldInit)
	{
		// Create the cover loader object
		coverLoader = new CoverLoader(info);

		// Set cover options
		InitializeOptions(info);

		// If necessary, enable plugin menu entry
		SetPluginMenuEntry();
			
		// Load a cover if necessary
		if(info->serviceFunc(opGetPlayerState, NULL, 0, 0) > 1)
		{
			shouldRedraw = true;
		}

		shouldInit = false;
	}

	return !0;
}

//-----------------------------------------------------------------------------

static void Idle(void)
{
	//
	// TODO : Run background computations.
	//		  Use this low priority callback for secondary computations
	//
}

//-----------------------------------------------------------------------------

static int Render(void)
{
	//
	// TODO : Render the current frame into buffer.
	//		  Return 1 to have buffer rendered to player, 0 otherwise.
	//

	if(shouldRedraw)
	{
		BlitToVis();
		shouldRedraw = false;
		return 1;
	}

	return 0;
}

//-----------------------------------------------------------------------------

static int Event(PluginEventOp type, long x, long y)
{
	//
	// TODO : Accept pointer motion, clicks and other player events.
	//		  Return 1 to have buffer updated to screen, 0 otherwise.
	//
	switch(type)
	{
	case trackChange:
	case lbuttonDown:
	case trackInfoChange:
		{
			// Load New Cover
			shouldRedraw = true;
			break;
		}
	}

	return 0;
}

//-----------------------------------------------------------------------------

static void About(void)
{
	//
	// TODO : Show the "about" dialog.
	//
	int res = DialogBox((HINSTANCE)info->context,
			  MAKEINTRESOURCE(IDD_ABOUT), 
			  (HWND)info->serviceFunc(opGetParentWnd, NULL, 0, 0),
			  (DLGPROC)AboutProc);
}

//-----------------------------------------------------------------------------

static void Configure(long flags)
{
	//
	// TODO : Show the "configure" dialog.
	// flags param is for plugin menu support
	//
	PropertySheetInit((HWND)info->serviceFunc(opGetParentWnd, NULL, 0, 0),
					  (HINSTANCE)info->context);
	SetPluginMenuEntry();
	shouldRedraw = true;
}

//-----------------------------------------------------------------------------

static void Wrap(void)
{
	//
	// TODO : Clean up and deallocate
	//
	if(coverLoader)
	{
		delete coverLoader;
	}
	rasterBuffer = NULL;
	SaveOptions(info);
	shouldInit = true;
}

//-----------------------------------------------------------------------------

//==Load Cover Function==
HBITMAP LoadCover()
{
	// Get the image format extensions to use
	WCHAR** extensions;
	int extensionSize = FORMAT_LENGTH + 1;
	extensions = new WCHAR*[extensionSize];
	for(int ctr = 0; ctr < extensionSize; ctr++)
	{
		extensions[ctr] = new WCHAR[MAX_PATH];
	}
	
	GetExtensionStrings((WCHAR**)extensions);
	
	bool loaded = false;
	int methodCtr = 0;
	HBITMAP cover = NULL;

	while(!loaded)
	{
		switch(coverOptions.loadingPriority[methodCtr++])
		{
		case TEMPLATE:
			{
				WCHAR templateFolder[MAX_PATH] = {0x0000};
				lstrcpy(templateFolder, coverOptions.templateFolder);
				if(!coverOptions.useTemplateFolder)
				{
					char trackFile[MAX_PATH];
					info->serviceFunc(opGetTrackFile, trackFile, sizeof(trackFile), -1);
					ConvertToUnicode(info, trackFile, templateFolder, MAX_PATH);
					for(int ctr = lstrlen(templateFolder) - 1; ctr >= 0; ctr--)
					{
						if(templateFolder[ctr] == 0x005c)
						{
							templateFolder[ctr] = 0x0000;
							break;
						}
					}
				}

				cover = coverLoader->LoadCoverFromTemplate(coverOptions.templateString,
														   templateFolder,
														   extensions, extensionSize);
				loaded = (bool)cover;
				break;
			}

		case IMAGELIST:
			{
				cover = coverLoader->LoadCoverFromImageList(coverOptions.imageListFile);
				loaded = (bool)cover;
				break;
			}

		case INTERNET:
			{
				cover = 
					coverLoader->LoadCoverFromInternetTemplate(coverOptions.netTemplateString,
															   coverOptions.netTemplateAddress,
															   coverOptions.netTemplateSpaceChar,
															   extensions, extensionSize);
				loaded = (bool)cover;
				break;
			}

		case NCA:
			{
				WCHAR nca[MAX_PATH];
				lstrcpy(nca, L"");
				if(coverOptions.useCustomNoCoverImage)
				{
					lstrcpy(nca, coverOptions.customNoCoverImage);
				}
				cover = coverLoader->LoadNCAImage(nca);
				loaded = true;
				break;
			}

		case RANDOM:
			{
				cover = coverLoader->LoadRandomImage(coverOptions.randomImageFolder,
													 extensions, extensionSize);
				loaded = (bool)cover;
				break;
			}

		case ID3:
			{
				cover = coverLoader->LoadCoverFromTag();
				loaded = (bool)cover;
				break;
			}
		}
	}

	// Return the cover to display
	return cover;
}

//==Blit To Visual Window Function==
void BlitToVis()
{
	// Load the cover
	HBITMAP cover = LoadCover();

	// Select the cover into a compatible device context
	HDC coverDC = CreateCompatibleDC(NULL);
	HBITMAP oldCover = (HBITMAP)SelectObject(coverDC, cover);

	// Determine Vis Dimensions
	long width, height, coverDim;
	long dimensions = info->serviceFunc(opGetVisDimensions, 0, -1, 0);
	width = LOWORD(dimensions);
	height = HIWORD(dimensions);
	if(height < width)
	{
		coverDim = height;
	}
	else
	{
		coverDim = width;
	}

	// Create Raster
	static Raster imageRaster;
	info->opFunc(opMakeRaster, &imageRaster, 0, 0, width, height);
	
	// Select the raster into a compatible device context
	HDC rasterDC = CreateCompatibleDC(NULL);
	HBITMAP oldRaster = (HBITMAP)SelectObject(rasterDC, imageRaster.context);

	// Get the dimensions of the cover
	DIBSECTION dib;
	GetObject(cover, sizeof(DIBSECTION), &dib);
	int coverHeight = dib.dsBm.bmHeight;
	int coverWidth = dib.dsBm.bmWidth;
		
	// Collect the colors in the picture
	vector<COLORREF> colors;
	int yIncrement = 50, xIncrement = 25;

	for(int y = 0; y < coverHeight; y += yIncrement)
	{
		for(int x = 0; x < coverWidth; x += xIncrement)
		{
			colors.push_back(GetPixel(coverDC, x, y));
		}
	}
	
	// Collect the frequency of the colors
	vector<int> colorFrequency;
	int frequency = 0;
	int ctr, ctr2;

	for(ctr = 0; ctr < colors.size(); ctr++)
	{
		for(ctr2 = 0; ctr2 < colors.size(); ctr2++)
		{
			if(colors[ctr] == colors[ctr2])
			{
				frequency++;
			}
		}
		colorFrequency.push_back(frequency);
		frequency = 0;
	}
	
	// Find the most frequent color
	int max = 0;
	int maxIndex = 0;
	for(ctr = 0; ctr < colorFrequency.size(); ctr++)
	{
		if(colorFrequency[ctr] > max)
		{
			max = colorFrequency[ctr];
			maxIndex = ctr;
		}
	}

	// Create the background brush
	HBRUSH background = CreateSolidBrush(RGB(GetRValue(colors[maxIndex]),
											 GetGValue(colors[maxIndex]),
											 GetBValue(colors[maxIndex])));
	
		
	// Fill the raster with the background color
	RECT rect;
	rect.bottom = height;
	rect.left = 0;
	rect.right = width;
	rect.top = 0;
	FillRect(rasterDC, &rect, background);
	
 
	// Blit the cover to the raster
	SetStretchBltMode(rasterDC, HALFTONE);
	POINT oldOrigin = {0};
	GetBrushOrgEx(rasterDC, &oldOrigin);
	SetBrushOrgEx(rasterDC, 0, 0, &oldOrigin);
	StretchBlt(rasterDC, (width / 2) - (coverDim / 2), 0, coverDim, coverDim, coverDC, 0, 0,
			   coverWidth, coverHeight, SRCCOPY);
		
	// Set Blit Info
	static BlitInfo B;
	
	// Point the system Raster back to its original location
	info->buffer = rasterBuffer;

	// Destination Raster is the system Raster
	B.dst = *rasterBuffer;
	
	// The image is the source Raster
	B.src = imageRaster;

	// Perform transformations
	B.scalex = B.scaley = 1;
	B.alpha = 255;

	// Composite the source raster into the system Raster:
	info->opFunc(opComposite, (char*)&B, 0, 0, 0, 0);

	// Clean up
	DeleteObject(background);
	DeleteObject(SelectObject(coverDC, oldCover));
	DeleteDC(coverDC);
	info->opFunc(opFreeRaster, &imageRaster, 0, 0, 0, 0);
	DeleteObject(SelectObject(rasterDC, oldRaster));
	DeleteDC(rasterDC);
}

//==Set Plugin Menu Entry Function==
void SetPluginMenuEntry()
{
	HINSTANCE pluginInst = (HINSTANCE)info->context;

	info->serviceFunc(opSetPluginMenuItem, (void*)pluginInst, 0, NULL);
	if(coverOptions.usePluginMenu)
	{
		info->serviceFunc(opSetPluginMenuItem, (void*)pluginInst, 0, (long)"Cover Art Vis Config");
	}
}

//==About Dialog Callback Function==
BOOL CALLBACK AboutProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
					case IDC_ABOUT_OK:
						{
							EndDialog(dlg, TRUE);
							return TRUE;
						}
					}
					return FALSE;
				}
			}
			return FALSE;
		}
	}

	return FALSE;
}

//==Property Sheet Initialization Function==
void PropertySheetInit(HWND parentWindow, HINSTANCE pluginInst)
{
	const NUM_PAGES = 7;
    PROPSHEETPAGE psp[NUM_PAGES];
    PROPSHEETHEADER psh;

	// Set common options for the property sheet pages
	for(int ctr = 0; ctr < NUM_PAGES; ctr++)
	{
		psp[ctr].dwSize = sizeof(PROPSHEETPAGE);
		psp[ctr].dwFlags = PSP_USEICONID | PSP_USETITLE;
		psp[ctr].hInstance = pluginInst;
		psp[ctr].pszIcon = 0;
		psp[ctr].lParam = 0;
		psp[ctr].pfnCallback = NULL;
	}

	// Page 1 - Priority Options
    psp[0].pszTemplate = MAKEINTRESOURCE(IDD_PRIORITY);
    psp[0].pfnDlgProc = PriorityProc;
    psp[0].pszTitle = L"Priority";

	// Page 2 - Image List Options
    psp[1].pszTemplate = MAKEINTRESOURCE(IDD_IMAGELIST);
    psp[1].pfnDlgProc = ImageListProc;
    psp[1].pszTitle = L"Image List";

	// Page 3 - Template Options
    psp[2].pszTemplate = MAKEINTRESOURCE(IDD_TEMPLATE);
	psp[2].pfnDlgProc = TemplateProc;
	psp[2].pszTitle = L"Filename Template";

	// Page 4 - Internet Template Options
	psp[3].pszTemplate = MAKEINTRESOURCE(IDD_INTERNET);
	psp[3].pfnDlgProc = InternetProc;
	psp[3].pszTitle = L"Internet Template";

	// Page 5 - Random Image Options
	psp[4].pszTemplate = MAKEINTRESOURCE(IDD_RANDOMIMAGE);
	psp[4].pfnDlgProc = RandomImageProc;
	psp[4].pszTitle = L"Random Images";

	// Page 6 - No Cover Available Options
	psp[5].pszTemplate = MAKEINTRESOURCE(IDD_NCA);
	psp[5].pfnDlgProc = NoCoverProc;
	psp[5].pszTitle = L"No Cover Image";
	
	// Page 7 - Other Options
	psp[6].pszTemplate = MAKEINTRESOURCE(IDD_OTHEROPTIONS);
	psp[6].pfnDlgProc = OtherOptionsProc;
	psp[6].pszTitle = L"Other Options";
	
	// Set property sheet header options
	psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
    psh.hwndParent = parentWindow;
    psh.hInstance = pluginInst;
    psh.pszIcon = 0;
    psh.pszCaption = L"Cover Art Vis Configuration";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGE)&psp;
    psh.pfnCallback = NULL;

	// Create the property sheet
    PropertySheet(&psh);
}

//==Priority Dialog Callback Function==
BOOL CALLBACK PriorityProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization
	case WM_INITDIALOG:
		{
			PriorityDlgInit(dlg);
			return TRUE;
		}

	// Command Message
	case WM_COMMAND:
		{
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				{
					SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);
					switch(LOWORD(wParam))
					{
					case IDC_IMAGE_MOVEUP:
					case IDC_LOADING_MOVEUP:
						{
							HWND listBox = GetDlgItem(dlg, IDC_PRIORITY_METHOD);
							if(LOWORD(wParam) == IDC_IMAGE_MOVEUP)
							{
								listBox = GetDlgItem(dlg, IDC_PRIORITY_FORMAT);
							}
								
							ListboxMoveUp(listBox);
							return TRUE;
						}

					case IDC_IMAGE_MOVEDOWN:
					case IDC_LOADING_MOVEDOWN:
						{
							HWND listBox = GetDlgItem(dlg, IDC_PRIORITY_METHOD);
							if(LOWORD(wParam) == IDC_IMAGE_MOVEDOWN)
							{
								listBox = GetDlgItem(dlg, IDC_PRIORITY_FORMAT);
							}

							ListboxMoveDown(listBox);
							return TRUE;
						}
					}

					return FALSE;
				}
			}
			return FALSE;
		}
	
	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SavePriorityValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			break;
		}
	}
	
	return FALSE;
}

//==Image List Dialog Callback Function==
BOOL CALLBACK ImageListProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			ImageListDlgInit(dlg);
			return TRUE;
		}

	// Window Command Message
	case WM_COMMAND:
		{
			// Enable the Apply Button
			SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);

			// Clear the success static text area
			SetDlgItemText(dlg, IDC_SUCCESS, L"");

			switch(HIWORD(wParam))
			{
			// Button Clicked Message
			case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
					case IDC_BROWSE:
						{
							WCHAR* fileName = NULL;
							WCHAR filter[] = L"Image Lists\0*.TXT\0All Files\0*.*\0";
							fileName = (WCHAR*)Browse(dlg, filter);
							if(lstrcmp(fileName, L"cancel") != 0)
							{
								SetDlgItemText(dlg, IDC_IMAGELISTFILE, fileName);
							}
							return TRUE;
						}
					case IDC_USECURRENT:
						{
							ImageListUseCurrent(dlg, info);
							return TRUE;
						}
					case IDC_BROWSE2:
						{
							WCHAR* fileName = NULL;
							WCHAR filter[] = 
								L"Image Files\0*.BMP;*.GIF;*.JPG;*.JPEG;*.EXIF;*.PNG;*.TIFF\0";
							fileName = (WCHAR*)Browse(dlg, filter);
							if(lstrcmp(fileName, L"cancel") != 0)
							{
								SetDlgItemText(dlg, IDC_FILENAME, fileName);
							}
							return TRUE;
						}
					case IDC_ADD:
						{
							ImageListWrite(dlg);
							return TRUE;
						}
					case IDC_GET:
						{
							ImageListRead(dlg);
							return TRUE;
						}
					}
				}
			}
			return FALSE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveImageListValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}
					
    }
	
	return FALSE;
}

//==Template Dialog Callback Function==
BOOL CALLBACK TemplateProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			TemplateDlgInit(dlg);
			return TRUE;
		}

	// Window Command Message
	case WM_COMMAND:
		{
			// Enable the Apply Button
			SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);

			switch(HIWORD(wParam))
			{
			// Button Clicked Message
			case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
					case IDC_TEMPLATE_BROWSE:
						{
							WCHAR* folder = NULL;
							folder = (WCHAR*)FolderBrowse(dlg);
							if(lstrcmp(folder, L"cancel") != 0)
							{
								SetDlgItemText(dlg, IDC_TEMPLATE_ROOT, folder);
							}
							return TRUE;
						}
					}
				}
			}
			return FALSE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveTemplateValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}
					
    }
	
	return FALSE;
}

//==Internet Template Callback Function==
BOOL CALLBACK InternetProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			InternetDlgInit(dlg);
			return TRUE;
		}

	// Window Command Message
	case WM_COMMAND:
		{
			// Enable the Apply Button
			SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);

			return FALSE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveInternetValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}
					
    }
	
	return FALSE;
}

//==Random Image Dialog Callback Function==
BOOL CALLBACK RandomImageProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			RandomImageDlgInit(dlg);
			return TRUE;
		}

	// Window Command Message
	case WM_COMMAND:
		{
			// Enable the Apply Button
			SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);
			
			switch(HIWORD(wParam))
			{
			// Button Clicked Message
			case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
					case IDC_RANDOM_BROWSE:
						{
							WCHAR* folder = NULL;
							folder = (WCHAR*)FolderBrowse(dlg);
							if(lstrcmp(folder, L"cancel") != 0)
							{
								SetDlgItemText(dlg, IDC_RANDOMDIR, folder);
							}
							return TRUE;
						}
					}
				}
			}

			return FALSE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveRandomImageValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}
					
    }
	
	return FALSE;
}

//==No Cover Dialog Callback Function==
BOOL CALLBACK NoCoverProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			NoCoverDlgInit(dlg);
			return TRUE;
		}

	// Window Command Message
	case WM_COMMAND:
		{
			// Enable the Apply Button
			SendMessage(GetParent(dlg), PSM_CHANGED, (WPARAM)dlg, 0);
			
			switch(HIWORD(wParam))
			{
			// Button Clicked Message
			case BN_CLICKED:
				{
					switch(LOWORD(wParam))
					{
					case IDC_NCA_BROWSE:
						{
							WCHAR* fileName = NULL;
							WCHAR filter[] = 
								L"Image Files\0*.BMP;*.GIF;*.JPG;*.JPEG;*.EXIF;*.PNG;*.TIFF\0";
							fileName = (WCHAR*)Browse(dlg, filter);
							if(lstrcmp(fileName, L"cancel") != 0)
							{
								SetDlgItemText(dlg, IDC_NCAFILE, fileName);
							}
							return TRUE;
						}
					}
				}
			}

			return FALSE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveNoCoverValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}
					
    }
	
	return FALSE;
}

//==Other Options Callback Function==
BOOL CALLBACK OtherOptionsProc(HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
	// Dialog Initialization Message
	case WM_INITDIALOG:
		{
			OtherOptionsDlgInit(dlg);
			return TRUE;
		}

	// Notify Message
	case WM_NOTIFY:
		{
			switch (((NMHDR*)lParam)->code) 
			{
			case PSN_KILLACTIVE:
				{
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, FALSE);
					return TRUE;
				}
			case PSN_APPLY:
				{
					SaveOtherOptionsValues(dlg);
					SendMessage(GetParent(dlg), PSM_UNCHANGED, (WPARAM)dlg, 0);
					SetWindowLong(((NMHDR*)lParam)->hwndFrom, DWL_MSGRESULT, PSNRET_NOERROR);
					return TRUE;
				}
			}
			
			return FALSE;
		}

	}

	return FALSE;
}