#pragma once
#include "render.h"

class i_app_system;

typedef void *( *create_interface_fn )( const char *name, int *return_code );

class i_app_system {
public:
	virtual bool connect( create_interface_fn factory ) = 0;
	virtual void disconnect( ) = 0;
	virtual void *query_interface( const char *interface_name ) = 0;
	virtual int init( ) = 0;
	virtual void shutdown( ) = 0;
	virtual const void *get_dependencies( ) = 0;
	virtual int  get_tier( ) = 0;
	virtual void reconnect( create_interface_fn factory, const char *interface_name ) = 0;
	virtual void unknown( ) = 0;
};

typedef unsigned long HFont;
typedef unsigned int VPANEL;

class c_surface : public i_app_system {
public:

	virtual void          RunFrame( ) = 0;
	virtual VPANEL		  GetEmbeddedPanel( ) = 0;
	virtual void          SetEmbeddedPanel( VPANEL pPanel ) = 0;
	virtual void          PushMakeCurrent( VPANEL panel, bool useInsets ) = 0;
	virtual void          PopMakeCurrent( VPANEL panel ) = 0;
	virtual void          DrawSetColor( int r, int g, int b, int a ) = 0;
	virtual void          DrawSetColor( color col ) = 0;
	virtual void          DrawFilledRect( int x0, int y0, int x1, int y1 ) = 0;
	virtual void          DrawFilledRectArray( void* pRects, int numRects ) = 0;
	virtual void          DrawOutlinedRect( int x0, int y0, int x1, int y1 ) = 0;
	virtual void          DrawLine( int x0, int y0, int x1, int y1 ) = 0;
	virtual void          DrawPolyLine( int* px, int* py, int numPoints ) = 0;
	virtual void          DrawSetApparentDepth( float f ) = 0;
	virtual void          DrawClearApparentDepth( void ) = 0;
	virtual void          DrawSetTextFont( HFont font ) = 0;
	virtual void          DrawSetTextColor( int r, int g, int b, int a ) = 0;
	virtual void          DrawSetTextColor( color col ) = 0;
	virtual void          DrawSetTextPos( int x, int y ) = 0;
	virtual void          DrawGetTextPos( int& x, int& y ) = 0;
	virtual void          DrawPrintText( const wchar_t* text, int textLen, int drawType = 0 ) = 0;
	virtual void          DrawUnicodeChar( wchar_t wch, int drawType = 0 ) = 0;
	virtual void          DrawFlushText( ) = 0;
	virtual void* CreateHTMLWindow( void* events, VPANEL context ) = 0;
	virtual void          PaintHTMLWindow( void* htmlwin ) = 0;
	virtual void          DeleteHTMLWindow( void* htmlwin ) = 0;
	virtual int           DrawGetTextureId( char const* filename ) = 0;
	virtual bool          DrawGetTextureFile( int id, char* filename, int maxlen ) = 0;
	virtual void          DrawSetTextureFile( int id, const char* filename, int hardwareFilter, bool forceReload ) = 0;
	virtual void          DrawSetTextureRGBA( int id, const unsigned char* rgba, int wide, int tall ) = 0;
	virtual void          DrawSetTexture( int id ) = 0;
	virtual void          DeleteTextureByID( int id ) = 0;
	virtual void          DrawGetTextureSize( int id, int& wide, int& tall ) = 0;
	virtual void          DrawTexturedRect( int x0, int y0, int x1, int y1 ) = 0;
	virtual bool          IsTextureIDValid( int id ) = 0;
	virtual int           CreateNewTextureID( bool procedural = false ) = 0;
	virtual void          GetScreenSize( int& wide, int& tall ) = 0;
	virtual void          SetAsTopMost( VPANEL panel, bool state ) = 0;
	virtual void          BringToFront( VPANEL panel ) = 0;
	virtual void          SetForegroundWindow( VPANEL panel ) = 0;
	virtual void          SetPanelVisible( VPANEL panel, bool state ) = 0;
	virtual void          SetMinimized( VPANEL panel, bool state ) = 0;
	virtual bool          IsMinimized( VPANEL panel ) = 0;
	virtual void          FlashWindow( VPANEL panel, bool state ) = 0;
	virtual void          SetTitle( VPANEL panel, const wchar_t* title ) = 0;
	virtual void          SetAsToolBar( VPANEL panel, bool state ) = 0;
	virtual void          CreatePopup( VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true ) = 0;
	virtual void          SwapBuffers( VPANEL panel ) = 0;
	virtual void          Invalidate( VPANEL panel ) = 0;
	virtual void          SetCursor( unsigned long cursor ) = 0;
	virtual bool          IsCursorVisible( ) = 0;
	virtual void          ApplyChanges( ) = 0;
	virtual bool          IsWithin( int x, int y ) = 0;
	virtual bool          HasFocus( ) = 0;
	virtual bool          SupportsFeature( int /*SurfaceFeature_t*/ feature ) = 0;
	virtual void          RestrictPaintToSinglePanel( VPANEL panel, bool bForceAllowNonModalSurface = false ) = 0;
	virtual void          SetModalPanel( VPANEL ) = 0;
	virtual VPANEL		  GetModalPanel( ) = 0;
	virtual void          UnlockCursor( ) = 0;
	virtual void          LockCursor( ) = 0;
};