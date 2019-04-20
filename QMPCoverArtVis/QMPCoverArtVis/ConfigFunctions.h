//==Preprocessors==
#ifndef _CONFIG_FUNCTIONS_H
#define _CONFIG_FUNCTIONS_H

#define UNICODE
#include "CoverOptions.h"
#include "QCDModVisuals.h"
#include <shlobj.h>

//==Global Variables==
extern CoverOptions coverOptions;

//==Preprocessors==
void InitializeOptions(PluginInfo* info);
void SaveOptions(PluginInfo* info);
void GetExtensionStrings(WCHAR** extensionBuffer);
void ListboxMoveUp(HWND listBox);
void ListboxMoveDown(HWND listBox);
void PriorityDlgInit(HWND dlg);
void SavePriorityValues(HWND dlg);
const WCHAR* Browse(HWND dlg, WCHAR* filter);
void ImageListDlgInit(HWND dlg);
void ImageListUseCurrent(HWND dlg, PluginInfo* info);
void ImageListWrite(HWND dlg);
void ImageListRead(HWND dlg);
void SaveImageListValues(HWND dlg);
void TemplateDlgInit(HWND dlg);
const WCHAR* FolderBrowse(HWND dlg);
void SaveTemplateValues(HWND dlg);
void InternetDlgInit(HWND dlg);
void SaveInternetValues(HWND dlg);
void RandomImageDlgInit(HWND dlg);
void SaveRandomImageValues(HWND dlg);
void NoCoverDlgInit(HWND dlg);
void SaveNoCoverValues(HWND dlg);
void OtherOptionsDlgInit(HWND dlg);
void SaveOtherOptionsValues(HWND dlg);
void RemoveSpaces(WCHAR* buffer);
void ReplaceSpaces(WCHAR* buffer, int bufferSize, WCHAR* replacement);
void ReAddSpaces(WCHAR* buffer);
void ConvertToUnicode(PluginInfo* info, const char* source, WCHAR* dest, int destSize);

#endif