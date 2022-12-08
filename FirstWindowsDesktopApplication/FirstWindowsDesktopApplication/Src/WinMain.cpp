// Windows header is needed for all windows applications
#include <Windows.h>
// Every dx9 app needs this header
#include <d3d9.h>
// For adapter details
#include <vector>
#include <string>
// Fstream for writing stuff to a file
#include <fstream>
// d3dx is a collection of commanly used functions that microsoft has provided w directx sdk
#include <d3dx9.h>

#define SCREEN_WIDTH 506
#define SCREEN_HEIGHT 400
#define SPRITE_WIDTH 170
#define SPRITE_HEIGHT 180

// Global variables
std::vector<std::string> adapterDetails;
HINSTANCE hInst; // handle to hold the application instance
HWND wndHandle; // holds the windle handle
LPDIRECT3D9 pD3D; // Direct3D object
LPDIRECT3DDEVICE9 pd3dDevice; // d3d9 device

// sprite shit
CONST INT MAX_SPRITE_COUNT = 1;
IDirect3DSurface9* spriteSurface;
LARGE_INTEGER timeStart;
LARGE_INTEGER timeEnd;
LARGE_INTEGER timeFrequency;
float anim_rate;
struct SpriteStruct
{
	RECT src;

	float x;
	float y;

	float moveX; // how many pixels to move in the x dir per frame
	float moveY; // how many pixels to move in the y dir per frame

	int numFrames;
	int curFrame;
} spriteStruct[MAX_SPRITE_COUNT];

IDirect3DSurface9* spriteSurface1;
struct SpriteStruct1
{
	RECT src;

	float x;
	float y;

	float moveX; // how many pixels to move in the x dir per frame
	float moveY; // how many pixels to move in the y dir per frame

	int numFrames;
	int curFrame;
} spriteStruct1;

//SpriteStruct1 spriteStruct1;

// Declarations
bool InitWindow(HINSTANCE hInstance);
bool InitDirect3D();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Render();
void CleanUp();
IDirect3DSurface9* GetSurfaceFromBitmap(std::string filename, int imageWidth, int imageHeight);
bool InitTextures();

void AddItemToList(std::string item);
bool InitSprites();
void SpriteMovement();

// WinMain is a main entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	srand(time(NULL));

	// Initialize the window
	if (!InitWindow(hInstance))
		return false;

	// Initialize direct3d
	if (!InitDirect3D())
		return false;

	if (!InitSprites())
		return false;

	if (!InitTextures())
		return false;

	QueryPerformanceFrequency(&timeFrequency);

	// Main message loop
	// Main message loop is the most important part of this code, because it recieves messages from the rest of the system, allowing the program to run in Windows environment
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		//
		// This get message loop is not as good for games, because if there is any messages, GetMessage waits for em to return. PeekMessage() returns in immediatly
		// 
		// GetMessage() checks applications message queue and determines wheter user input or system messages are waiting
		//while (GetMessage(&msg, wndHandle, 0, 0))
		//{
		//	// If messages are available, these functions get called
		//	TranslateMessage(&msg);
		//	DispatchMessage(&msg);
		//}

		// So we do this: if there is a message, tranlsate & dispatch it instantly
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise
		else
		{
			QueryPerformanceCounter(&timeStart);

			SpriteMovement();

			// Render function is being called once a frame
			Render();

			QueryPerformanceCounter(&timeEnd);
			anim_rate = ((float)timeEnd.QuadPart - (float)timeStart.QuadPart) / timeFrequency.QuadPart;
			Sleep(10);
		}
	}

	// Cleanup
	CleanUp();

	// Return wParam
	return (int)msg.wParam;
}

// Implementations
bool InitWindow(HINSTANCE hInstance)
{
	// WNDCLASSEX struct describes how thw windows will look
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); // size of the struct
	wcex.style = CS_HREDRAW | CS_VREDRAW; // class style
	wcex.lpfnWndProc = (WNDPROC)WndProc; //window procedure callback
	wcex.cbClsExtra = 0; // extra bytes to allocate for this class
	wcex.cbWndExtra = 0; // extra bytes to allocate for this 
	wcex.hInstance = hInstance; // handle to the application instance
	wcex.hIcon = 0; //icon for the application
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); // default cursor
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // background color
	wcex.lpszMenuName = L"FirstWindowsDesktopApplication";    // name of menu resource 
	wcex.lpszClassName = L"FirstWindowsDesktopApplication";  // name of window class 
	wcex.hIconSm = 0; // handle to the small icon
	RegisterClassEx(&wcex);

	// Create window
	wndHandle = CreateWindowW
	(
		L"FirstWindowsDesktopApplication", // window class to use
		L"FirstWindowsDesktopApplication", // window title
		WS_EX_TOPMOST // window style. window is going to be on top of every other window
		//| WS_POPUP  // no borders/buttons & other crap
		| WS_VISIBLE, // tells the window to display itself
		CW_USEDEFAULT, // starting x coordinate
		CW_USEDEFAULT, // starting y coordinate
		SCREEN_WIDTH, // window width(pixels)
		SCREEN_HEIGHT, // window height(pixels)
		NULL, // parrent window; NULL for desktop
		NULL, // menu for the application
		hInstance, // handle to application instance
		NULL // no values passed to the window
	);

	// Make sure that the window handle created is valid
	if (!wndHandle)
		return false;

	// Render the window on the screen
	ShowWindow(wndHandle, SW_SHOW);
	UpdateWindow(wndHandle);

	// Return true, everything went ok
	return true;
}

bool InitDirect3D()
{
	pD3D = NULL;
	pd3dDevice = NULL;

	// Create the DirectX object
	if ((pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Gets adapter details
	D3DADAPTER_IDENTIFIER9 ident;
	pD3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &ident);

	// Adds information to the list
	AddItemToList("Adapter details");
	AddItemToList(ident.Description);
	AddItemToList(ident.DeviceName);
	AddItemToList(ident.Driver);

	// Collects the mode count this adapter has
	UINT numModes = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT, D3DFMT_X8R8G8B8);

	std::ofstream file;

	file.open("SupportedAdapterData.dat");

	// Use the collected mode count to gather info about each one
	for (int i = 0; i < numModes; ++i)
	{
		D3DDISPLAYMODE mode; // D3DDISPLAYMODE struct definition
		char modeStr[255]; // Temp char array

		// Get the displaymode struct for this adapter mode
		pD3D->EnumAdapterModes
		(
			D3DADAPTER_DEFAULT,
			D3DFMT_X8R8G8B8,
			i,
			&mode
		);

		// Blank line to seperate stuff
		AddItemToList("");
		file << "" << std::endl;

		// Output the width
		sprintf_s(modeStr, "Width=%d", mode.Width);
		file << mode.Width << std::endl;
		// Add width to the list
		AddItemToList(modeStr);

		// Output the height
		sprintf_s(modeStr, "Height=%d", mode.Height);
		file << mode.Height << std::endl;
		// Add height to the list
		AddItemToList(modeStr);

		// Output the RefreshRate
		sprintf_s(modeStr, "Refresh rate=%d", mode.RefreshRate);
		file << mode.RefreshRate << std::endl;
		// Add RefreshRate to the list
		AddItemToList(modeStr);
	}

	file.close();

	// Fill the Present parameters struct
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferWidth = SCREEN_WIDTH;
	d3dpp.BackBufferHeight = SCREEN_HEIGHT;
	d3dpp.hDeviceWindow = wndHandle;

	// Create a default DirectX device
	if (FAILED(pD3D->CreateDevice
	(
		D3DADAPTER_DEFAULT, // use the primary video adapter
		D3DDEVTYPE_REF, // create a device that uses a default Direct3D implementation
		wndHandle,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING, // ensures that your app runs on most hardware
		&d3dpp,
		&pd3dDevice // stores the created device in pd3dDevice
	)))
		return false;

	return true;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// Check any available messaged from the queue
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	// Always return the message to the default window
	// Procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void Render()
{
	// This will hold the back buffer
	IDirect3DSurface9* backbuffer = NULL;

	// Make sure the Direct3D device is not NULL
	if (pd3dDevice == NULL)
		return; // If its NULL, clear the back buffer to a blue color. We dont want to draw if Direct3D dev is released.

	// Clears the entire buffer, thats why you pass 0 and NULL as first two parameters
	pd3dDevice->Clear
	(
		0,
		NULL,
		D3DCLEAR_TARGET, // tells dx that you want the render buffer to be cleared
		D3DCOLOR_XRGB(255, 255, 255), // clears the screen to white color
		1.0f, // 1.0f gets passed into depth buffer. depth buffers helps dx determine how far the object uis from the viewer. (hols a vlue between 0.0f - 1.0f)
		0 // Stencil buffer allows for masking certain areas of an image, so they arent displayed. because stencil buffer is not used, we pass 0
	);

	// Get the back buffer prt
	pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backbuffer);

	for (int i = 0; i < MAX_SPRITE_COUNT; ++i)
	{
		if (spriteStruct[i].curFrame < spriteStruct[i].numFrames)
			++spriteStruct[i].curFrame;
		else
			spriteStruct[i].curFrame = 0;

		spriteStruct[i].src.left = spriteStruct[i].curFrame * SPRITE_WIDTH;
		spriteStruct[i].src.right = spriteStruct[i].src.left + SPRITE_WIDTH;

		RECT dst;
		RECT src = spriteStruct[i].src;

		dst.left = spriteStruct[i].x;
		dst.top = spriteStruct[i].y;
		dst.bottom = dst.top + SPRITE_HEIGHT;
		dst.right = dst.left + SPRITE_WIDTH;

		pd3dDevice->StretchRect
		(
			spriteSurface,
			&src,
			backbuffer,
			&dst,
			D3DTEXF_NONE
		);
	}

	// Present the back buffer contents to the display
	pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void CleanUp()
{
	if (pd3dDevice != NULL)
		pd3dDevice->Release();

	if (pD3D != NULL)
		pD3D->Release();
}

void AddItemToList(std::string item)
{
	adapterDetails.push_back(item);
}

IDirect3DSurface9* GetSurfaceFromBitmap(std::string filename, int imageWidth, int imageHeight)
{
	HRESULT hResult;
	IDirect3DSurface9* surface = NULL;
	D3DXIMAGE_INFO imageInfo;

	// Get image info from a file
	hResult = D3DXGetImageInfoFromFileA(filename.c_str(), &imageInfo);

	if (FAILED(hResult))
		return NULL;

	//
	// Creating surface
	//
	hResult = pd3dDevice->CreateOffscreenPlainSurface(imageWidth, imageHeight, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

	if (FAILED(hResult))
		return NULL;

	//
	// Loading bitmap into a surface
	//
	hResult = D3DXLoadSurfaceFromFileA(surface, NULL, NULL, filename.c_str(), NULL, D3DX_DEFAULT, 0, NULL);

	if (FAILED(hResult))
		return NULL;

	return surface;
}

bool InitSprites()
{
	for (int i = 0; i < MAX_SPRITE_COUNT; ++i)
	{
		spriteStruct[i].src.top = 0;
		spriteStruct[i].src.left = i * SPRITE_WIDTH;
		spriteStruct[i].src.right = spriteStruct[i].src.left + SPRITE_WIDTH;
		spriteStruct[i].src.bottom = SPRITE_HEIGHT;
		spriteStruct[i].x = rand() % SCREEN_WIDTH - SPRITE_WIDTH;
		spriteStruct[i].y = rand() & SCREEN_HEIGHT - SPRITE_HEIGHT;
		spriteStruct[i].moveX = 60.f;
		spriteStruct[i].moveY = 60.f;
		spriteStruct[i].curFrame = 0;
		spriteStruct[i].numFrames = 4;
	}
	return true;
}

void SpriteMovement()
{
	for (int i = 0; i < MAX_SPRITE_COUNT; ++i)
	{
		spriteStruct[i].x += spriteStruct[i].moveX * anim_rate;
		if (spriteStruct[i].x + SPRITE_WIDTH > SCREEN_WIDTH)
			spriteStruct[i].moveX *= -1;

		spriteStruct[i].y += spriteStruct[i].moveY * anim_rate;
		if (spriteStruct[i].y + SPRITE_HEIGHT > SCREEN_HEIGHT)
			spriteStruct[i].moveY *= -1;

		if (spriteStruct[i].x < 0)
			spriteStruct[i].moveX *= -1;

		if (spriteStruct[i].y < 0)
			spriteStruct[i].moveY *= -1;
	}
}

bool InitTextures()
{
	spriteSurface = GetSurfaceFromBitmap("sprite.bmp", 860, 366);
	if (spriteSurface == NULL)
		return false;

	return true;
}