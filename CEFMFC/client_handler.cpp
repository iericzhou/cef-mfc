// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "stdafx.h"
#include "CEFMFC.h"
#include "CEFMFCView.h"
#include "CEFMFCDoc.h"

#include "client_handler.h"
#include <stdio.h>
#include <algorithm>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "include/cef_browser.h"
#include "include/cef_frame.h"
#include "include/cef_path_util.h"
#include "include/cef_process_util.h"
#include "include/cef_runnable.h"
#include "include/cef_trace.h"
#include "include/cef_url.h"
#include "client_switches.h"
#include "client_util.h"


ClientHandler::ClientHandler()
{
}

ClientHandler::~ClientHandler()
{
}

bool ClientHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
	// get browser ID
	INT nBrowserId = browser->GetIdentifier();
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	return false;
}

void ClientHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
	REQUIRE_UI_THREAD();

	// get browser ID
	INT nBrowserId = browser->GetIdentifier();
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// assign new browser
	CefBrowser* pBrowser = browser;
	::SendMessage( hWindow, WM_APP_CEF_NEW_BROWSER, (WPARAM)nBrowserId, (LPARAM)pBrowser );
}

void ClientHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
	REQUIRE_UI_THREAD();

	// get browser ID
	INT nBrowserId = browser->GetIdentifier();
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// close browser
	::SendMessage( hWindow, WM_APP_CEF_CLOSE_BROWSER, (WPARAM)nBrowserId, (LPARAM)NULL );
}

bool ClientHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& target_url, const CefString& target_frame_name, const CefPopupFeatures& popupFeatures, CefWindowInfo& windowInfo, CefRefPtr<CefClient>& client, CefBrowserSettings& settings, bool* no_javascript_access)
{
	if(browser->GetHost()->IsWindowRenderingDisabled())
	{
		// Cancel popups in off-screen rendering mode.
		return true;
	}
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// send message
	LPCTSTR lpszURL(target_url.c_str());
	if( ::SendMessage( hWindow, WM_APP_CEF_WINDOW_CHECK, (WPARAM)&popupFeatures, (LPARAM)lpszURL) == S_FALSE )
		return true;

	// send message
	if( ::SendMessage( hWindow, WM_APP_CEF_NEW_WINDOW, (WPARAM)&popupFeatures, (LPARAM)&windowInfo) == S_FALSE )
		return true;

	// return false to create browser
	return false;
}

void ClientHandler::OnBeforeContextMenu(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, CefRefPtr<CefMenuModel> model)
{
	if((params->GetTypeFlags() & (CM_TYPEFLAG_PAGE | CM_TYPEFLAG_FRAME)) != 0)
	{
	}
}

bool ClientHandler::OnContextMenuCommand(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefContextMenuParams> params, int command_id, EventFlags event_flags)
{
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	return false;
}

void ClientHandler::OnLoadingStateChange(CefRefPtr<CefBrowser> browser, bool isLoading, bool canGoBack, bool canGoForward)
{
	REQUIRE_UI_THREAD();

	INT nState = 0;
	// set state
	if( isLoading )
		nState |= CEF_BIT_IS_LOADING;
	if( canGoBack )
		nState |= CEF_BIT_CAN_GO_BACK;
	if( canGoForward )
		nState |= CEF_BIT_CAN_GO_FORWARD;

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// send message
	::SendMessage( hWindow, WM_APP_CEF_STATE_CHANGE, (WPARAM)nState, NULL );
}

void ClientHandler::OnAddressChange(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, const CefString& url)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	LPCTSTR pszURL(url.c_str());
	::SendMessage( hWindow, WM_APP_CEF_ADDRESS_CHANGE, (WPARAM)pszURL, NULL );
}

void ClientHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString& title)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	LPCTSTR pszTitle(title.c_str());
	::SendMessage( hWindow, WM_APP_CEF_TITLE_CHANGE, (WPARAM)pszTitle, NULL );
}

void ClientHandler::OnStatusMessage(CefRefPtr<CefBrowser> browser, const CefString& value)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	LPCTSTR pszStatus(value.c_str());
	::SendMessage( hWindow, WM_APP_CEF_STATUS_MESSAGE, (WPARAM)pszStatus, NULL );
}

bool ClientHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, const CefString& message, const CefString& source, int line)
{
	REQUIRE_UI_THREAD();
	return TRUE;
}

void ClientHandler::OnBeforeDownload(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, const CefString& suggested_name, CefRefPtr<CefBeforeDownloadCallback> callback)
{
	REQUIRE_UI_THREAD();

	// Continue the download and show the "Save As" dialog.
	callback->Continue(GetDownloadPath(suggested_name), true);
}

void ClientHandler::OnDownloadUpdated(CefRefPtr<CefBrowser> browser, CefRefPtr<CefDownloadItem> download_item, CefRefPtr<CefDownloadItemCallback> callback)
{
	REQUIRE_UI_THREAD();

	CEFDownloadItemValues values;

	values.bIsValid = download_item->IsValid();
	values.bIsInProgress = download_item->IsInProgress();
	values.bIsComplete = download_item->IsComplete();
	values.bIsCanceled = download_item->IsCanceled();
	values.nProgress = download_item->GetPercentComplete();
	values.nSpeed = download_item->GetCurrentSpeed();
	values.nReceived = download_item->GetReceivedBytes();
	values.nTotal = download_item->GetTotalBytes();

	CString szDispo = download_item->GetContentDisposition().c_str();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// send message
	::SendMessage( hWindow, WM_APP_CEF_DOWNLOAD_UPDATE, (WPARAM)&values, NULL );
}

void ClientHandler::OnLoadStart(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// send message
	::SendMessage( hWindow, WM_APP_CEF_LOAD_START, NULL, NULL);
}

void ClientHandler::OnLoadEnd(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, int httpStatusCode)
{
	REQUIRE_UI_THREAD();

	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// send message
	::SendMessage( hWindow, WM_APP_CEF_LOAD_END, httpStatusCode, NULL);
}

void ClientHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode, const CefString& errorText, const CefString& failedUrl)
{
	REQUIRE_UI_THREAD();

	// net_error_list.h NAME_NOT_RESOLVED -105
	if( errorCode == -105)
	{
		// The frame window will be the parent of the browser window
		HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

		LPCTSTR pszSearch(failedUrl.c_str());
		::SendMessage( hWindow, WM_APP_CEF_SEARCH_URL, (WPARAM)pszSearch, NULL );
	}
}

bool ClientHandler::GetAuthCredentials(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, bool isProxy, const CefString& host, int port, const CefString& realm, const CefString& scheme, CefRefPtr<CefAuthCallback> callback)
{
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	CEFAuthenticationValues values;
	values.lpszHost = host.c_str();
	values.lpszRealm = realm.c_str();
	_tcscpy_s(values.szUserName, _T(""));
	_tcscpy_s(values.szUserPass, _T(""));

	// send info
	if(::SendMessage( hWindow, WM_APP_CEF_AUTHENTICATE, (WPARAM)&values, (LPARAM)NULL ) == S_OK)
	{
		callback->Continue( values.szUserName, values.szUserPass );
		return TRUE;
	}
	// canceled
	return FALSE;
}

CefRefPtr<CefResourceHandler> ClientHandler::GetResourceHandler(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
	return NULL;
}

bool ClientHandler::OnQuotaRequest(CefRefPtr<CefBrowser> browser, const CefString& origin_url, int64 new_size, CefRefPtr<CefQuotaCallback> callback)
{
	static const int64 max_size = 1024 * 1024 * 20;  // 20mb.

	// Grant the quota request if the size is reasonable.
	callback->Continue(new_size <= max_size);
	return true;
}

bool ClientHandler::OnBeforePluginLoad(CefRefPtr<CefBrowser> browser, const CefString& url, const CefString& policy_url, CefRefPtr<CefWebPluginInfo> info)
{
	// do default
	return FALSE;
}

bool ClientHandler::OnCertificateError(cef_errorcode_t cert_error, const CefString& request_url, CefRefPtr<CefAllowCertificateErrorCallback> callback)
{
	CString szMessage;

	// no file, or empty, show the default
	if( szMessage.IsEmpty() )
	{
		szMessage.Format(_T("The site's security certificate is not trusted!\n\n You attempted to reach \"%s\""), request_url.c_str() );
	
		szMessage += _T("but the server presented a certificate issued by an entity that is not trusted by your computer's operating system.");
		szMessage += _T("This may mean that the server has generated its own security credentials, ");
		szMessage += _T("which Chrome cannot rely on for identity information, or an attacker may be ");
		szMessage += _T("trying to intercept your communications.\n\n");
		szMessage += _T("You should not proceed, especially if you have never seen this warning before for this site.");
	}

	if( MessageBox(NULL, szMessage, _T("The site's security certificate is not trusted:"), MB_YESNO ) == IDNO )
		return FALSE;

	// continue
	callback->Continue( true );

	return TRUE;
}

bool ClientHandler::OnBeforeResourceLoad(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request)
{
	// do defulat
	return FALSE;
}

void ClientHandler::OnProtocolExecution(CefRefPtr<CefBrowser> browser, const CefString& url, bool& allow_os_execution)
{
}

bool ClientHandler::OnBeforeBrowse(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, CefRefPtr<CefRequest> request, bool is_redirect)
{
	// The frame window will be the parent of the browser window
	HWND hWindow = GetParent( browser->GetHost()->GetWindowHandle() );

	// get URL requested
	CefString newURL = request->GetURL();

	LPCTSTR pszURL(newURL.c_str());
	if( ::SendMessage( hWindow, WM_APP_CEF_BEFORE_BROWSE, (WPARAM)pszURL, (LPARAM)is_redirect ) == S_FALSE )
	{
		// cancel navigation
		return TRUE;
	}
	// allow navigation
	return FALSE;
}

std::string ClientHandler::GetDownloadPath(const std::string& file_name)
{
	TCHAR szFolderPath[MAX_PATH];
	std::string path;

	// Save the file in the user's "My Documents" folder.
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE,
		NULL, 0, szFolderPath))) {
			path = CefString(szFolderPath);
			path += "\\" + file_name;
	}

	return path;
}