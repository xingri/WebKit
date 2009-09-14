/*
 * Copyright (C) 2007 Kevin Ollivier  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "config.h"
#include "CString.h"
#include "Document.h"
#include "Element.h"
#include "Editor.h"
#include "EventHandler.h"
#include "FocusController.h"
#include "Frame.h"
#include "FrameLoader.h"
#include "FrameView.h"
#include "GraphicsContext.h"
#include "Logging.h"
#include "markup.h"
#include "Page.h"
#include "ContextMenu.h"
#include "ContextMenuItem.h"
#include "ContextMenuController.h"
#include "PlatformKeyboardEvent.h"
#include "PlatformMouseEvent.h"
#include "PlatformString.h"
#include "PlatformWheelEvent.h"
#include "RenderObject.h"
#include "RenderView.h"
#include "Scrollbar.h"
#include "SelectionController.h"
#include "Settings.h"
#include "SubstituteData.h"
#include "Threading.h"

#include "ChromeClientWx.h"
#include "ContextMenuClientWx.h"
#include "DragClientWx.h"
#include "EditorClientWx.h"
#include "FrameLoaderClientWx.h"
#include "InspectorClientWx.h"

#include "ScriptController.h"
#include "JSDOMBinding.h"
#include <runtime/JSValue.h>
#include <runtime/UString.h>

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
    #include "wx/wx.h"
#endif

#include "WebFrame.h"
#include "WebView.h"
#include "WebViewPrivate.h"

#include <wx/defs.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>

#if defined(_MSC_VER)
int rint(double val)
{
    return (int)(val < 0 ? val - 0.5 : val + 0.5);
}
#endif

// ----------------------------------------------------------------------------
// wxWebView Events
// ----------------------------------------------------------------------------

IMPLEMENT_DYNAMIC_CLASS(wxWebViewLoadEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_LOAD)

wxWebViewLoadEvent::wxWebViewLoadEvent(wxWindow* win)
{
    SetEventType( wxEVT_WEBVIEW_LOAD);
    SetEventObject( win );
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewBeforeLoadEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_BEFORE_LOAD)

wxWebViewBeforeLoadEvent::wxWebViewBeforeLoadEvent(wxWindow* win)
{
    m_cancelled = false;
    SetEventType(wxEVT_WEBVIEW_BEFORE_LOAD);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewNewWindowEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_NEW_WINDOW)

wxWebViewNewWindowEvent::wxWebViewNewWindowEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_NEW_WINDOW);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewRightClickEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_RIGHT_CLICK)

wxWebViewRightClickEvent::wxWebViewRightClickEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_RIGHT_CLICK);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewConsoleMessageEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_CONSOLE_MESSAGE)

wxWebViewConsoleMessageEvent::wxWebViewConsoleMessageEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_CONSOLE_MESSAGE);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewAlertEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_JS_ALERT)

wxWebViewAlertEvent::wxWebViewAlertEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_JS_ALERT);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewConfirmEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_JS_CONFIRM)

wxWebViewConfirmEvent::wxWebViewConfirmEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_JS_CONFIRM);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewPromptEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_JS_PROMPT)

wxWebViewPromptEvent::wxWebViewPromptEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_JS_PROMPT);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewReceivedTitleEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_RECEIVED_TITLE)

wxWebViewReceivedTitleEvent::wxWebViewReceivedTitleEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_RECEIVED_TITLE);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}

IMPLEMENT_DYNAMIC_CLASS(wxWebViewWindowObjectClearedEvent, wxCommandEvent)

DEFINE_EVENT_TYPE(wxEVT_WEBVIEW_WINDOW_OBJECT_CLEARED)

wxWebViewWindowObjectClearedEvent::wxWebViewWindowObjectClearedEvent(wxWindow* win)
{
    SetEventType(wxEVT_WEBVIEW_WINDOW_OBJECT_CLEARED);
    SetEventObject(win);
    if (win)
        SetId(win->GetId());
}


//---------------------------------------------------------
// DOM Element info data type
//---------------------------------------------------------

wxWebViewDOMElementInfo::wxWebViewDOMElementInfo() :
    m_domElement(NULL),
    m_isSelected(false),
    m_text(wxEmptyString),
    m_imageSrc(wxEmptyString),
    m_link(wxEmptyString)
{
}

BEGIN_EVENT_TABLE(wxWebView, wxWindow)
    EVT_PAINT(wxWebView::OnPaint)
    EVT_SIZE(wxWebView::OnSize)
    EVT_MOUSE_EVENTS(wxWebView::OnMouseEvents)
    EVT_CONTEXT_MENU(wxWebView::OnContextMenuEvents)
    EVT_MENU(wxID_ANY, wxWebView::OnMenuSelectEvents)
    EVT_KEY_DOWN(wxWebView::OnKeyEvents)
    EVT_KEY_UP(wxWebView::OnKeyEvents)
    EVT_CHAR(wxWebView::OnKeyEvents)
    EVT_SET_FOCUS(wxWebView::OnSetFocus)
    EVT_KILL_FOCUS(wxWebView::OnKillFocus)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(wxWebView, wxWindow)

const wxChar* wxWebViewNameStr = wxT("webView");

wxWebView::wxWebView() :
    m_textMagnifier(1.0),
    m_isEditable(false),
    m_isInitialized(false),
    m_beingDestroyed(false),
    m_title(wxEmptyString)
{
}

wxWebView::wxWebView(wxWindow* parent, int id, const wxPoint& position, 
                     const wxSize& size, long style, const wxString& name) :
    m_textMagnifier(1.0),
    m_isEditable(false),
    m_isInitialized(false),
    m_beingDestroyed(false),
    m_title(wxEmptyString)
{
    Create(parent, id, position, size, style, name);
}

bool wxWebView::Create(wxWindow* parent, int id, const wxPoint& position, 
                       const wxSize& size, long style, const wxString& name)
{
    if ( (style & wxBORDER_MASK) == 0)
        style |= wxBORDER_NONE;
    
    if (!wxWindow::Create(parent, id, position, size, style, name))
        return false;

    WTF::initializeThreading();

// This is necessary because we are using SharedTimerWin.cpp on Windows,
// due to a problem with exceptions getting eaten when using the callback
// approach to timers (which wx itself uses).
#if __WXMSW__
    WebCore::Page::setInstanceHandle(wxGetInstance());
#endif

    // this helps reduce flicker on platforms like MSW
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);

    m_impl = new WebViewPrivate();

    WebCore::InitializeLoggingChannelsIfNecessary();    
    WebCore::HTMLFrameOwnerElement* parentFrame = 0;

    WebCore::EditorClientWx* editorClient = new WebCore::EditorClientWx();
    m_impl->page = new WebCore::Page(new WebCore::ChromeClientWx(this), new WebCore::ContextMenuClientWx(), editorClient, new WebCore::DragClientWx(), new WebCore::InspectorClientWx());
    editorClient->setPage(m_impl->page);
    
    m_mainFrame = new wxWebFrame(this);

    // Default settings - we should have wxWebViewSettings class for this
    // eventually
    WebCore::Settings* settings = m_impl->page->settings();
    settings->setLoadsImagesAutomatically(true);
    settings->setDefaultFixedFontSize(13);
    settings->setDefaultFontSize(16);
    settings->setSerifFontFamily("Times New Roman");
    settings->setFixedFontFamily("Courier New");
    settings->setSansSerifFontFamily("Arial");
    settings->setStandardFontFamily("Times New Roman");
    settings->setJavaScriptEnabled(true);

    m_isInitialized = true;

    return true;
}

wxWebView::~wxWebView()
{
    m_beingDestroyed = true;
    
    if (m_mainFrame && m_mainFrame->GetFrame())
        m_mainFrame->GetFrame()->loader()->detachFromParent();
    
    delete m_impl->page;
    m_impl->page = 0;   
}

void wxWebView::Stop()
{
    if (m_mainFrame)
        m_mainFrame->Stop();
}

void wxWebView::Reload()
{
    if (m_mainFrame)
        m_mainFrame->Reload();
}

wxString wxWebView::GetPageSource()
{
    if (m_mainFrame)
        return m_mainFrame->GetPageSource();

    return wxEmptyString;
}

void wxWebView::SetPageSource(const wxString& source, const wxString& baseUrl)
{
    if (m_mainFrame)
        m_mainFrame->SetPageSource(source, baseUrl);
}

wxString wxWebView::GetInnerText()
{
    if (m_mainFrame)
        return m_mainFrame->GetInnerText();
        
    return wxEmptyString;
}

wxString wxWebView::GetAsMarkup()
{
    if (m_mainFrame)
        return m_mainFrame->GetAsMarkup();
        
    return wxEmptyString;
}

wxString wxWebView::GetExternalRepresentation()
{
    if (m_mainFrame)
        return m_mainFrame->GetExternalRepresentation();
        
    return wxEmptyString;
}

void wxWebView::SetTransparent(bool transparent)
{
    WebCore::Frame* frame = 0;
    if (m_mainFrame)
        frame = m_mainFrame->GetFrame();
    
    if (!frame || !frame->view())
        return;

    frame->view()->setTransparent(transparent);
}

bool wxWebView::IsTransparent() const
{
    WebCore::Frame* frame = 0;
    if (m_mainFrame)
        frame = m_mainFrame->GetFrame();

   if (!frame || !frame->view())
        return false;

    return frame->view()->isTransparent();
}

wxString wxWebView::RunScript(const wxString& javascript)
{
    if (m_mainFrame)
        return m_mainFrame->RunScript(javascript);
    
    return wxEmptyString;
}

void wxWebView::LoadURL(const wxString& url)
{
    if (m_mainFrame)
        m_mainFrame->LoadURL(url);
}

bool wxWebView::GoBack()
{
    if (m_mainFrame)
        return m_mainFrame->GoBack();

    return false;
}

bool wxWebView::GoForward()
{
    if (m_mainFrame)
        return m_mainFrame->GoForward();

    return false;
}

bool wxWebView::CanGoBack()
{
    if (m_mainFrame)
        return m_mainFrame->CanGoBack();

    return false;
}

bool wxWebView::CanGoForward()
{
    if (m_mainFrame)
        return m_mainFrame->CanGoForward();

    return false;
}

bool wxWebView::CanIncreaseTextSize() const
{
    if (m_mainFrame)
        return m_mainFrame->CanIncreaseTextSize();

    return false;
}

void wxWebView::IncreaseTextSize()
{
    if (m_mainFrame)
        m_mainFrame->IncreaseTextSize();
}

bool wxWebView::CanDecreaseTextSize() const
{
    if (m_mainFrame)
        m_mainFrame->CanDecreaseTextSize();

    return false;
}

void wxWebView::DecreaseTextSize()
{        
    if (m_mainFrame)
        m_mainFrame->DecreaseTextSize();
}

void wxWebView::ResetTextSize()
{
    if (m_mainFrame)
        m_mainFrame->ResetTextSize();    
}

void wxWebView::MakeEditable(bool enable)
{
    m_isEditable = enable;
}


/* 
 * Event forwarding functions to send events down to WebCore.
 */

void wxWebView::OnPaint(wxPaintEvent& event)
{
    if (m_beingDestroyed || !m_mainFrame)
        return;

    // WebView active state is based on TLW active state.
    wxTopLevelWindow* tlw = dynamic_cast<wxTopLevelWindow*>(wxGetTopLevelParent(this));
    if (tlw && tlw->IsActive())
        m_impl->page->focusController()->setActive(true);
    else {
        m_impl->page->focusController()->setActive(false);
    }
    WebCore::Frame* frame = m_mainFrame->GetFrame();
    if (!frame || !frame->view())
        return;
    
    wxAutoBufferedPaintDC dc(this);

    if (IsShown() && frame->document()) {
#if USE(WXGC)
        wxGCDC gcdc(dc);
#endif

        if (dc.IsOk()) {
            wxRect paintRect = GetUpdateRegion().GetBox();

#if USE(WXGC)
            WebCore::GraphicsContext gc(&gcdc);
#else
            WebCore::GraphicsContext gc(&dc);
#endif
            if (frame->contentRenderer()) {
                frame->view()->layoutIfNeededRecursive();
                frame->view()->paint(&gc, paintRect);
            }
        }
    }
}

bool wxWebView::FindString(const wxString& string, bool forward, bool caseSensitive, bool wrapSelection, bool startInSelection)
{
    if (m_mainFrame)
        return m_mainFrame->FindString(string, forward, caseSensitive, wrapSelection, startInSelection);

    return false;
}

void wxWebView::OnSize(wxSizeEvent& event)
{ 
    if (m_isInitialized && m_mainFrame) {
        WebCore::Frame* frame = m_mainFrame->GetFrame();
        frame->view()->setFrameRect(wxRect(wxPoint(0,0), event.GetSize()));
        frame->view()->forceLayout();
        frame->view()->adjustViewSize();
    }
      
    event.Skip();
}

static int getDoubleClickTime()
{
#if __WXMSW__
    return ::GetDoubleClickTime();
#else
    return 500;
#endif
}

void wxWebView::OnMouseEvents(wxMouseEvent& event)
{
    event.Skip();
    
    if (!m_impl->page)
        return; 
        
    WebCore::Frame* frame = m_mainFrame->GetFrame();  
    if (!frame || !frame->view())
        return;
    
    wxPoint globalPoint = ClientToScreen(event.GetPosition());

    wxEventType type = event.GetEventType();
    
    if (type == wxEVT_MOUSEWHEEL) {
        WebCore::PlatformWheelEvent wkEvent(event, globalPoint);
        frame->eventHandler()->handleWheelEvent(wkEvent);
        return;
    }
    
    int clickCount = event.ButtonDClick() ? 2 : 1;

    if (clickCount == 1 && m_impl->tripleClickTimer.IsRunning()) {
        wxPoint diff(event.GetPosition() - m_impl->tripleClickPos);
        if (abs(diff.x) <= wxSystemSettings::GetMetric(wxSYS_DCLICK_X) &&
            abs(diff.y) <= wxSystemSettings::GetMetric(wxSYS_DCLICK_Y)) {
            clickCount = 3;
        }
    } else if (clickCount == 2) {
        m_impl->tripleClickTimer.Start(getDoubleClickTime(), false);
        m_impl->tripleClickPos = event.GetPosition();
    }
    
    WebCore::PlatformMouseEvent wkEvent(event, globalPoint, clickCount);

    if (type == wxEVT_LEFT_DOWN || type == wxEVT_MIDDLE_DOWN || type == wxEVT_RIGHT_DOWN || 
                type == wxEVT_LEFT_DCLICK || type == wxEVT_MIDDLE_DCLICK || type == wxEVT_RIGHT_DCLICK) {
        frame->eventHandler()->handleMousePressEvent(wkEvent);
        if (!HasCapture())
            CaptureMouse();
    } else if (type == wxEVT_LEFT_UP || type == wxEVT_MIDDLE_UP || type == wxEVT_RIGHT_UP) {
        frame->eventHandler()->handleMouseReleaseEvent(wkEvent);
        while (HasCapture())
            ReleaseMouse();
    } else if (type == wxEVT_MOTION || type == wxEVT_ENTER_WINDOW || type == wxEVT_LEAVE_WINDOW)
        frame->eventHandler()->mouseMoved(wkEvent);
}

void wxWebView::OnContextMenuEvents(wxContextMenuEvent& event)
{
    m_impl->page->contextMenuController()->clearContextMenu();
    wxPoint localEventPoint = ScreenToClient(event.GetPosition());

    if (!m_impl->page)
        return;
        
    WebCore::Frame* focusedFrame = m_impl->page->focusController()->focusedOrMainFrame();
    if (!focusedFrame->view())
        return;

    //Create WebCore mouse event from the wxContextMenuEvent
    wxMouseEvent mouseEvent(wxEVT_RIGHT_DOWN);
    mouseEvent.m_x = localEventPoint.x;
    mouseEvent.m_y = localEventPoint.y;
    WebCore::PlatformMouseEvent wkEvent(mouseEvent, event.GetPosition(), 1);

    bool handledEvent = focusedFrame->eventHandler()->sendContextMenuEvent(wkEvent);
    if (!handledEvent)
        return;

    WebCore::ContextMenu* coreMenu = m_impl->page->contextMenuController()->contextMenu();
    if (!coreMenu)
        return;

    WebCore::PlatformMenuDescription menuWx = coreMenu->platformDescription();
    if (!menuWx)
        return;

    PopupMenu(menuWx, localEventPoint);
}

void wxWebView::OnMenuSelectEvents(wxCommandEvent& event)
{
    WebCore::ContextMenuItem* item = WebCore::ContextMenu::itemWithId (event.GetId());
    if (!item)
        return;

    m_impl->page->contextMenuController()->contextMenuItemSelected(item);
    delete item;
}

bool wxWebView::CanCopy()
{
    if (m_mainFrame)
        return m_mainFrame->CanCopy();

    return false;
}

void wxWebView::Copy()
{
    if (m_mainFrame)
        m_mainFrame->Copy();
}

bool wxWebView::CanCut()
{
    if (m_mainFrame)
        return m_mainFrame->CanCut();

    return false;
}

void wxWebView::Cut()
{
    if (m_mainFrame)
        m_mainFrame->Cut();
}

bool wxWebView::CanPaste()
{
    if (m_mainFrame)
        return m_mainFrame->CanPaste();

    return false;
}

void wxWebView::Paste()
{
    if (m_mainFrame)
        m_mainFrame->Paste();
}

void wxWebView::OnKeyEvents(wxKeyEvent& event)
{
    WebCore::Frame* frame = 0;
    if (m_impl->page)
        frame = m_impl->page->focusController()->focusedOrMainFrame();

    if (!(frame && frame->view()))
        return;

    if (event.GetKeyCode() == WXK_CAPITAL)
        frame->eventHandler()->capsLockStateMayHaveChanged();

    WebCore::PlatformKeyboardEvent wkEvent(event);

    if (frame->eventHandler()->keyEvent(wkEvent))
        return;

    //Some things WebKit won't do for us... Copy/Cut/Paste and KB scrolling
    if (event.GetEventType() == wxEVT_KEY_DOWN) {
        switch (event.GetKeyCode()) {
        case 67: //"C"
            if (CanCopy() && event.GetModifiers() == wxMOD_CMD) {
                Copy();
                return;
            }
            break;
        case 86: //"V"
            if (CanPaste() && event.GetModifiers() == wxMOD_CMD) {
                Paste();
                return;
            }
            break;
        case 88: //"X"
            if (CanCut() && event.GetModifiers() == wxMOD_CMD) {
                Cut();
                return;
            }
            break;
        case WXK_INSERT:
            if (CanCopy() && event.GetModifiers() == wxMOD_CMD) {
                Copy();
                return;
            }
            if (CanPaste() && event.GetModifiers() == wxMOD_SHIFT) {
                Paste();
                return;
            }
            return; //Insert shall not become a char
        case WXK_DELETE:
            if (CanCut() && event.GetModifiers() == wxMOD_SHIFT) {
                Cut();
                return;
            }
            break;
        case WXK_LEFT:
        case WXK_NUMPAD_LEFT:
            frame->view()->scrollBy(WebCore::IntSize(-WebCore::cScrollbarPixelsPerLineStep, 0));
            return;
        case WXK_UP:
        case WXK_NUMPAD_UP:
            frame->view()->scrollBy(WebCore::IntSize(0, -WebCore::cScrollbarPixelsPerLineStep));
            return;
        case WXK_RIGHT:
        case WXK_NUMPAD_RIGHT:
            frame->view()->scrollBy(WebCore::IntSize(WebCore::cScrollbarPixelsPerLineStep, 0));
            return;
        case WXK_DOWN:
        case WXK_NUMPAD_DOWN:
            frame->view()->scrollBy(WebCore::IntSize(0, WebCore::cScrollbarPixelsPerLineStep));
            return;
        case WXK_END:
        case WXK_NUMPAD_END:
            frame->view()->setScrollPosition(WebCore::IntPoint(frame->view()->scrollX(), frame->view()->maximumScrollPosition().y()));
            return;
        case WXK_HOME:
        case WXK_NUMPAD_HOME:
            frame->view()->setScrollPosition(WebCore::IntPoint(frame->view()->scrollX(), 0));
            return;
        case WXK_PAGEUP:
        case WXK_NUMPAD_PAGEUP:
            frame->view()->scrollBy(WebCore::IntSize(0, -frame->view()->visibleHeight() + WebCore::cAmountToKeepWhenPaging));
            return;
        case WXK_PAGEDOWN:
        case WXK_NUMPAD_PAGEDOWN:
            frame->view()->scrollBy(WebCore::IntSize(0, frame->view()->visibleHeight() - WebCore::cAmountToKeepWhenPaging));
            return;
        //These we don't want turning into char events, stuff 'em
        case WXK_ESCAPE:
        case WXK_LBUTTON:
        case WXK_RBUTTON:
        case WXK_CANCEL:
        case WXK_MENU:
        case WXK_MBUTTON:
        case WXK_CLEAR:
        case WXK_PAUSE:
        case WXK_SELECT:
        case WXK_PRINT:
        case WXK_EXECUTE:
        case WXK_SNAPSHOT:
        case WXK_HELP:
        case WXK_F1:
        case WXK_F2:
        case WXK_F3:
        case WXK_F4:
        case WXK_F5:
        case WXK_F6:
        case WXK_F7:
        case WXK_F8:
        case WXK_F9:
        case WXK_F10:
        case WXK_F11:
        case WXK_F12:
        case WXK_F13:
        case WXK_F14:
        case WXK_F15:
        case WXK_F16:
        case WXK_F17:
        case WXK_F18:
        case WXK_F19:
        case WXK_F20:
        case WXK_F21:
        case WXK_F22:
        case WXK_F23:
        case WXK_F24:
        case WXK_NUMPAD_F1:
        case WXK_NUMPAD_F2:
        case WXK_NUMPAD_F3:
        case WXK_NUMPAD_F4:
        //When numlock is off Numpad 5 becomes BEGIN, or HOME on Char
        case WXK_NUMPAD_BEGIN:
        case WXK_NUMPAD_INSERT:
            return;
        }
    }

    event.Skip();
}

void wxWebView::OnSetFocus(wxFocusEvent& event)
{
    WebCore::Frame* frame = 0;
    if (m_mainFrame)
        frame = m_mainFrame->GetFrame();
        
    if (frame) {
        frame->selection()->setFocused(true);
    }

    event.Skip();
}

void wxWebView::OnKillFocus(wxFocusEvent& event)
{
    WebCore::Frame* frame = 0;
    if (m_mainFrame)
        frame = m_mainFrame->GetFrame();
        
    if (frame) {
        frame->selection()->setFocused(false);
    }
    event.Skip();
}

wxWebViewDOMElementInfo wxWebView::HitTest(const wxPoint& pos) const
{
    if (m_mainFrame)
        return m_mainFrame->HitTest(pos);

    return wxWebViewDOMElementInfo();
}

