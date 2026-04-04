
#include "stdafx.h"

#include <commdlg.h>
#include <objbase.h>
#include <cderr.h>

// enable using of Vista funcs
#define _WIN32_WINNT 0x0600
#include <shobjidl.h>
#include "FileDialog.h"

typedef HRESULT (*LPFNSHCREATEITEMFROMPARSINGNAMEPROC)(PCWSTR path, void *, REFIID riid, void **ppv);
static LPFNSHCREATEITEMFROMPARSINGNAMEPROC pSHCreateItemFromParsingName;

static std::wstring ansi2wide(xr_string str)
{
	std::wstring ret;
	size_t sz = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), NULL, 0);
	ret.resize(sz);
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), &*ret.begin(), ret.size());
	return ret;
}

static xr_string wide2ansi(std::wstring str)
{
	xr_string ret;
	size_t sz = WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
	ret.resize(sz);
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), str.length(), &*ret.begin(), ret.length(), NULL, NULL);
	return ret;
}

static xr_string tofullpath(xr_string path)
{
	if(path.length() >= 2 && (path[1] == ':' || (path[0] == '\\' && path[1] == '\\')))
		return path;

	char cd[512];
	GetCurrentDirectory(sizeof(cd), cd);

	xr_string current_path = cd;
	char last = current_path[current_path.length()-1];
	if(last != '\\' && last != '/')
		current_path += '\\';

    return current_path + path;
}

CFileDialog::CFileDialog(int dialog_type)
{
	Multiselect 	= false;
    ShowReadOnly 	= false;
    FileMustExist 	= false;
    PathMustExist 	= false;
    OverwritePrompt = false;
    NoChangeDir 	= false;
    NoPlacesBar 	= false;

    if(dialog_type == fdOpen)
    	Caption = "Open";
    if(dialog_type == fdSave)
    	Caption = "Save";

    DefaultExt = "";
    DefaultDir = "";
    CurrentFile = "";
    CurrentDir	= "";

	Type 			= dialog_type;
	DefaultFilter	= 0;
    CurrentFilter	= -1;
}

void CFileDialog::SetFilters(const char *filters)
{
	Filters.clear();
    
	const char *p = filters;
    while(p[0])
    {
    	const char *desc = p;
        p = strchr(desc, 0) + 1;
        const char *mask = p;
        p = strchr(mask, 0) + 1;

        AddFilter(mask, desc);
    }
}

bool CFileDialog::ShowModal(HWND owner)
{
	Results.clear();

    if(DefaultExt.length())
    {
    	if(DefaultExt.length() >= 2 && DefaultExt[0] == '*' && DefaultExt[1] == '.')
        	DefaultExt.erase(0, 2);
        else if(DefaultExt.length() >= 1 && DefaultExt[0] == '.')
        	DefaultExt.erase(0, 1);
    }

	int ret;
	if((GetVersion() & 0xFF) >= 6) // if NT6 or greater
	{
    	ret = ShowVista(owner);
		if(ret < 0)
        {
        	Log("Vista dialog failed :-(");
            ret = ShowOld(owner);
        }
    }
    else
    	ret = ShowOld(owner);

    return ret > 0;
}

int CFileDialog::ShowOld(HWND owner)
{
	char result[512*256];
    strcpy(result, CurrentFile.c_str());
	OPENFILENAME ofn = {0};

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = owner;
	ofn.lpstrFile = result;
	ofn.nMaxFile = sizeof(result);
	ofn.lpstrInitialDir = DefaultDir.size() ? DefaultDir.c_str() : NULL;
	ofn.lpstrTitle = Caption.size() ? Caption.c_str() : NULL;

	DWORD flags = 0;

	if(Multiselect)
		flags |= OFN_EXPLORER|OFN_ALLOWMULTISELECT;
	if(!ShowReadOnly)
		flags |= OFN_HIDEREADONLY;
    if(PathMustExist)
        flags |= OFN_PATHMUSTEXIST;
    if(FileMustExist)
        flags |= OFN_FILEMUSTEXIST;
    if(OverwritePrompt)
        flags |= OFN_OVERWRITEPROMPT;
    if(NoChangeDir)
        flags |= OFN_NOCHANGEDIR;

    ofn.Flags = flags;
    ofn.lpstrDefExt = DefaultExt.size() ? DefaultExt.c_str() : NULL;

    if(NoPlacesBar)
    	ofn.FlagsEx = OFN_EX_NOPLACESBAR;

    char flt[1024] = "";
    if(Filters.size())
    {
    	char *p = flt;
    	for(size_t i = 0; i < Filters.size(); i++)
    	{
        	SFilter &f = Filters[i];
			size_t len;

            len = f.desc.length();
            memcpy(p, f.desc.c_str(), len+1);
            p += len + 1;
			len = f.mask.length();
            memcpy(p, f.mask.c_str(), len+1);
            p += len + 1;
		}
		*p = '\0';
		ofn.lpstrFilter = flt;
	}

    if(CurrentFilter != -1)
    	ofn.nFilterIndex = CurrentFilter + 1;
    else
    	ofn.nFilterIndex = DefaultFilter + 1;

    if(Type == fdOpen)
    {
		if(GetOpenFileName(&ofn) == FALSE)
		{
			if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
				SetLastError(DWORD(ERROR_INSUFFICIENT_BUFFER));
			return 0;
		}
	}
	else
	{
		if(GetSaveFileName(&ofn) == FALSE)
		{
			if(CommDlgExtendedError() == FNERR_BUFFERTOOSMALL)
				SetLastError(DWORD(ERROR_INSUFFICIENT_BUFFER));
            return 0;
        }
    }
    // if multiselect and selected more than one file
    if(Type == fdOpen && Multiselect && result[strlen(result)+1] != '\0')
    {
    	char *p = result;

        char dir[512];
        strcpy(dir, p);

        p = strchr(p, 0) + 1;
        while(p[0])
        {
        	Results.push_back(xr_string(dir) + "\\" + p);
            p = strchr(p, 0) + 1;
        }
    }
    else
    	Results.push_back(result);

    CurrentFilter = ofn.nFilterIndex - 1;
    CurrentFile = Results[0];
    char drive[_MAX_DRIVE], dir[_MAX_DIR];
    _splitpath(Results[0].c_str(), drive, dir, NULL, NULL);
    CurrentDir = xr_string(drive) + dir;

    return 1;
}

int CFileDialog::ShowVista(HWND owner)
{
	if(DefaultDir.length())
		DefaultDir = tofullpath(DefaultDir);
	if(CurrentDir.length())
		CurrentDir = tofullpath(CurrentDir);
    
    if(pSHCreateItemFromParsingName == NULL)
    {
		HMODULE hShell32 = GetModuleHandle("SHELL32.DLL");
        R_ASSERT(hShell32);
        pSHCreateItemFromParsingName = (LPFNSHCREATEITEMFROMPARSINGNAMEPROC)GetProcAddress(hShell32, "SHCreateItemFromParsingName");
        R_ASSERT(pSHCreateItemFromParsingName);
    }

    IFileDialog *dlg;
    IFileOpenDialog *odlg = NULL;
    IFileSaveDialog *sdlg = NULL;

    int ret = -1;

    do
    {
		HRESULT hr;

		if(Type == fdOpen)
        {
			hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileOpenDialog, (void**)&odlg);
            if(FAILED(hr)) break;

            dlg = odlg; // it's okay??
        }
        else
        {
            hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_IFileSaveDialog, (void**)&sdlg);
            if(FAILED(hr)) break;

			dlg = sdlg;
        }

        DWORD flags = 0;

        if(Multiselect)
            flags |= FOS_ALLOWMULTISELECT;
        if(PathMustExist)
            flags |= FOS_PATHMUSTEXIST;
        if(FileMustExist)
            flags |= FOS_FILEMUSTEXIST;
        if(NoChangeDir)
            flags |= FOS_NOCHANGEDIR;
        if(OverwritePrompt)
            flags |= FOS_OVERWRITEPROMPT;

        hr = dlg->SetOptions(flags);
		if(FAILED(hr)) break;

		hr = dlg->SetTitle(ansi2wide(Caption).c_str());
		if(FAILED(hr)) break;

        IShellItem *si_defdir;
		hr = pSHCreateItemFromParsingName(ansi2wide(DefaultDir).c_str(),
			NULL, IID_IShellItem, (void**)&si_defdir);
        if(!FAILED(hr))
		{
			hr = dlg->SetDefaultFolder(si_defdir);
        	si_defdir->Release();
        	if(FAILED(hr)) break;
		}

        if(CurrentDir.size())
		{
            IShellItem *si_cd;
			hr = pSHCreateItemFromParsingName(ansi2wide(CurrentDir).c_str(),
				NULL, IID_IShellItem, (void**)&si_cd);
            if(!FAILED(hr))
            {
            	hr = dlg->SetFolder(si_cd);
                si_cd->Release();
                if(FAILED(hr)) break;
            }
		}

		hr = dlg->SetDefaultExtension(ansi2wide(DefaultExt).c_str());
        if(FAILED(hr)) break;

        if(Filters.size())
		{
			xr_vector<COMDLG_FILTERSPEC> specs;
			xr_vector<std::wstring> names, masks;

			size_t filter_cnt = Filters.size();
			specs.resize(filter_cnt);
			names.resize(filter_cnt);
			masks.resize(filter_cnt);

            for(size_t i = 0; i < filter_cnt; i++)
			{
				names[i] = ansi2wide(Filters[i].desc);
				masks[i] = ansi2wide(Filters[i].mask);

				specs[i].pszName = (LPCWSTR)names[i].c_str();
				specs[i].pszSpec = (LPCWSTR)masks[i].c_str();
			}

			hr = dlg->SetFileTypes(filter_cnt, &specs.front());
            if(FAILED(hr)) break;
        }

        if(CurrentFilter != -1)
        	/*hr =*/ dlg->SetFileTypeIndex(CurrentFilter + 1);
        else
			/*hr =*/ dlg->SetFileTypeIndex(DefaultFilter + 1);

		/*hr =*/ dlg->SetFileName(ansi2wide(CurrentFile).c_str());

        if(dlg->Show(owner) != S_OK)
		{
        	ret = 0;
            break;
        }

		if(Type == fdOpen && Multiselect)
        {
            IShellItemArray *arr;
            hr = odlg->GetResults(&arr);
            if(FAILED(hr)) break;

            ULONG count;
            hr = arr->GetCount(&count);
            if(FAILED(hr)) { arr->Release(); break; }

            for(ULONG i = 0; i < count; i++)
            {
                IShellItem *si;
                hr = arr->GetItemAt(i, &si);
                if(FAILED(hr)) { arr->Release(); goto fail; }

                WCHAR *pfn;
                hr = si->GetDisplayName(SIGDN_FILESYSPATH, &pfn);
                if(FAILED(hr)) { arr->Release(); si->Release(); goto fail; }

				Results.push_back(wide2ansi((wchar_t*)pfn));
				CoTaskMemFree(pfn);

                si->Release();
            }

            arr->Release();
        }
        else
        {
            IShellItem *si_result;
            hr = dlg->GetResult(&si_result);
            if(FAILED(hr)) break;

            WCHAR *pfn;
            hr = si_result->GetDisplayName(SIGDN_FILESYSPATH, &pfn);
            si_result->Release();
            if(FAILED(hr)) break;

			Results.push_back(wide2ansi((wchar_t*)pfn));
			CoTaskMemFree(pfn);
        }

        hr = dlg->GetFileTypeIndex((UINT*)&CurrentFilter);
        if(FAILED(hr)) CurrentFilter = -1;

        WCHAR *wcfn;
        hr = dlg->GetFileName(&wcfn);
        if(!FAILED(hr))
		{
			CurrentFile = wide2ansi((wchar_t*)wcfn);
			CoTaskMemFree(wcfn);
        }

        IShellItem *si_cd;
        hr = dlg->GetFolder(&si_cd);
        if(!FAILED(hr))
        {
			WCHAR *wcd;
            hr = si_cd->GetDisplayName(SIGDN_FILESYSPATH, &wcd);
            if(!FAILED(hr))
			{
				CurrentDir = wide2ansi((wchar_t*)wcd);
				CoTaskMemFree(wcd);
            }
            si_cd->Release();
		}

        ret = 1;
    }
    while(0);
    fail:

    if(odlg) odlg->Release();
    if(sdlg) sdlg->Release();

    return ret;
}
