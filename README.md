# QMPCoverArtVis
Visualization plug-in for the defunct Quintessential Media Player. It was written for Windows XP with the last release (version 2.971) on October 21st, 2004.

## Compilation and Linking
Compilers have become better and more stringent, so this project does not compile in its current state when targeting modern Windows. The solution has been set up to target the Windows XP Platform Toolset (Visual Studio 2017 - Windows XP (v141_xp)) using Visual Studio 2019. MFC support is also required (for resources).

In addition, conformance mode (/permissive-) was turned off to allow the Windows XP API to compile. These dependencies are linked: gdiplus.lib, wininet.lib, Comctl32.lib

See https://web.archive.org/web/20210315184252/https://docs.microsoft.com/en-us/cpp/build/configuring-programs-for-windows-xp?view=msvc-160 for information on targeting Windows XP.

Visual Studio's support for Windows XP is deprecated, so it may not be possible to compile in the future without many changes.

## Original Description
https://web.archive.org/web/20160307221351/http://quinnware.com/list_plugins.php?plugin=19

![QMPCoverArtVis example](https://web.archive.org/web/20150428075727im_/http://www.quinnware.com/img/plugin_screens/CoverArtVis.jpg)

This plugin allows one to load cover art image files to the vis window (created with cover art in mind but any picture file will work)!

This plugin will find covers in a few different ways: a filename template, internet url template, Now Playing image list, or ID3v2 tags.

After installing, go to plugin configuration first to set it up the way you'd like.

***IMPORTANT NOTE***
If you are using a Windows version less than Windows XP, you must click on the "website" or "support file" link above and download the GDI+ redistributable from Microsoft. Install it to your Quintessentail Player application folder. This is necessary to load the covers. If you are on XP or Server 2003, you need not install this as you already have it.

This plugin will not work on Windows 95.

New to version 2.971:
- Fixed default filename template to be folder.%E, not a lowercase 'e'.
- Updated support and website links to be newest version of gdi+

New to version 2.97:
- Fix for id3 image loading (images should load from most mp3s correctly now)

New to version 2.95:
- More fixes for Win9x/ME

New to version 2.94:
- Now should work properly on Win9x/ME

New to version 2.93:
- Fix for files with no tag (such as video) ; now will not freeze player.
