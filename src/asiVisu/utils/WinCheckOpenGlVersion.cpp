//-----------------------------------------------------------------------------
// Created on: 11 December 2024
//-----------------------------------------------------------------------------
// Copyright (c) 2024-present, Quaoar Studio LLC
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//    * Neither the name of the copyright holder(s) nor the
//      names of all contributors may be used to endorse or promote products
//      derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//-----------------------------------------------------------------------------

// Own include
#include "WinCheckOpenGlVersion.h"

#if defined WIN32

typedef HGLRC (WINAPI * PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);
#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0x2092
#define WGL_CONTEXT_FLAGS_ARB 0x2094
#define GL_MAJOR_VERSION 0x821B
#define GL_MINOR_VERSION 0x821C

WinCheckOpenGlVersion::WinCheckOpenGlVersion( HINSTANCE hInst ) :
  hInstance( hInst )
{
  pixelFormatDescriptor = {sizeof( PIXELFORMATDESCRIPTOR ),
    1,
    PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
    PFD_TYPE_RGBA, // The kind of framebuffer. RGBA or palette.
    32,            // Colordepth of the framebuffer.
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    24, // Number of bits for the depthbuffer
    8,  // Number of bits for the stencilbuffer
    0,  // Number of Aux buffers in the framebuffer.
    PFD_MAIN_PLANE,
    0,
    0,
    0,
    0};

  WNDCLASS wc = {0};
  wc.lpfnWndProc = DefWindowProc;
  wc.hInstance = hInstance;
  wc.hbrBackground = reinterpret_cast< HBRUSH >( COLOR_BACKGROUND );
  wc.lpszClassName = "oglversioncheck";
  wc.style = CS_OWNDC;

  // Unregister if this window class remains from the previous invocation
  // of the OpenGL version checker. If not, then `ERROR_CLASS_DOES_NOT_EXIST`
  // (1411) is issued legally.
  if ( !UnregisterClass("oglversioncheck", this->hInstance) )
  {
    int errcode = GetLastError();
    std::cout << "[WinCheckOpenGlVersion] Last error: " << errcode << std::endl;
  }

  if( !RegisterClass( &wc ) )
  {
    int errcode = GetLastError();
    std::cout << "[WinCheckOpenGlVersion] Last error: " << errcode << std::endl;
    return;
  }
  HWND windowHandle = CreateWindow( wc.lpszClassName,
    "openglversioncheck",
    WS_OVERLAPPEDWINDOW,
    0,
    0,
    640,
    480,
    0,
    0,
    this->hInstance,
    0 );

  if( windowHandle != nullptr )
  {
    HDC ourWindowHandleToDeviceContext = GetDC( windowHandle );

    int letWindowsChooseThisPixelFormat;
    letWindowsChooseThisPixelFormat =
      ChoosePixelFormat( ourWindowHandleToDeviceContext, &this->pixelFormatDescriptor );
    SetPixelFormat( ourWindowHandleToDeviceContext,
      letWindowsChooseThisPixelFormat, &this->pixelFormatDescriptor );

    HGLRC ourOpenGLRenderingContext = wglCreateContext( ourWindowHandleToDeviceContext );

    wglMakeCurrent( ourWindowHandleToDeviceContext, ourOpenGLRenderingContext);

    this->version = std::string( reinterpret_cast< const char* >( glGetString( GL_VERSION ) ) );
    this->renderer = std::string( reinterpret_cast< const char* >( glGetString( GL_RENDERER ) ) );
    this->vendor = std::string( reinterpret_cast< const char* >( glGetString( GL_VENDOR ) ) );

    // create a context
    PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB =
      reinterpret_cast<PFNWGLCREATECONTEXTATTRIBSARBPROC>(wglGetProcAddress("wglCreateContextAttribsARB"));
    if (wglCreateContextAttribsARB)
    {
      // we believe that these later versions are all compatible with
      // OpenGL 3.2 so get a more recent context if we can.
      int attemptedVersions[] = {4,5, 4,4, 4,3, 4,2, 4,1, 4,0, 3,3, 3,2};
      int iContextAttribs[] =
        {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 2,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0 // End of attributes list
        };
      HGLRC contextId = nullptr;
      for (int i = 0; i < 8 && !contextId; i++)
      {
        iContextAttribs[1] = attemptedVersions[i*2];
        iContextAttribs[3] = attemptedVersions[i*2+1];
        contextId = wglCreateContextAttribsARB(ourWindowHandleToDeviceContext, 0, iContextAttribs);
      }
      if (contextId)
      {
        wglMakeCurrent( ourWindowHandleToDeviceContext, contextId);

        glGetIntegerv(GL_MAJOR_VERSION, &this->glMajorVersion);
        glGetIntegerv(GL_MINOR_VERSION, &this->glMinorVersion);

        wglDeleteContext(contextId);
        contextId = nullptr;
      }
    }

    wglDeleteContext( ourOpenGLRenderingContext );

    // Close this window so that we can call OpenGL version checker
    // again in the future.
    PostMessage(windowHandle, WM_CLOSE, 0, 0);
  }
}

bool WinCheckOpenGlVersion::hasVersion_3_2( )
{
  // version string should have format "4.5.0 <vendorstuff>"
  // so it is enough to parse the 0th and 2nd char
  int majorCheck = 3;
  int minorCheck = 2;

  if( this->glMajorVersion > majorCheck )
  {
    return true;
  }

  if( this->glMajorVersion == majorCheck && this->glMinorVersion >= minorCheck )
  {
    return true;
  }

  return false;
}

#endif
