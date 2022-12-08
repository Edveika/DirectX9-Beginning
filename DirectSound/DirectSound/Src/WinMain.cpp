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
// Include direct sound
#pragma comment(lib, "Dsound.lib")
#include <DSound.h>
//#include <mmeapi.h>

#include "Sound/SoundClass.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Global variables
std::vector<std::string> adapterDetails;
HINSTANCE hInst; // handle to hold the application instance
HWND wndHandle; // holds the windle handle
LPDIRECT3D9 pD3D; // Direct3D object
LPDIRECT3DDEVICE9 pd3dDevice; // d3d9 device

// Direct sound
LPDIRECTSOUND8 directSound;
LPDIRECTSOUNDBUFFER DSbuffer = NULL;

// Declarations
bool InitWindow(HINSTANCE hInstance);
bool InitDirect3D();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void AddItemToList(std::string item);

// Direct sound function declarations
bool CreateDefaultSoundDevice();
bool EnumerateDevices();
BOOL CALLBACK DSCallback(GUID* pGUID, LPSTR strDesc, LPSTR strDevName, VOID* pContext);
bool SetDirectInputCooperativeLevel();
bool CreateSecondSoundBuffer();

// WinMain is a main entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize the window
	if (!InitWindow(hInstance))
		return false;

	// Initialize direct3d
	if (!InitDirect3D())
		return false;
	SoundClass sound;

	sound.Initialize(wndHandle);

	while (true)
	{

	}

	// Return 0
	return 0;
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
	wcex.lpszMenuName = L"DirectX9PredefinedMeshes";    // name of menu resource 
	wcex.lpszClassName = L"DirectX9PredefinedMeshes";  // name of window class 
	wcex.hIconSm = 0; // handle to the small icon
	RegisterClassEx(&wcex);

	// Create window
	wndHandle = CreateWindowW
	(
		L"DirectX9PredefinedMeshes", // window class to use
		L"DirectX9PredefinedMeshes", // window title
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

	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);    // turn off the 3D lighting

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

void AddItemToList(std::string item)
{
	adapterDetails.push_back(item);
} 

bool CreateDefaultSoundDevice()
{
	HRESULT hr;

	hr = DirectSoundCreate8
	(
		NULL,
		&directSound,
		NULL
	);

	if (FAILED(hr))
		return false;

	return true;
}

bool EnumerateDevices()
{
	/*HRESULT hr;

	hr = DirectSoundEnumerate
	(
		(LPDSENUMCALLBACK)CallBack,
		0
	);

	if (FAILED(hr))
		return false;*/

	return true;
}

BOOL CALLBACK DSCallback(GUID* pGUID, LPSTR strDesc, LPSTR strDevName, VOID* pContext)
{
	std::string tmpStr;

	tmpStr = "Device name = ";
	tmpStr += strDesc;
	tmpStr += "\nDriver name = ";
	tmpStr += strDevName;

	MessageBoxA(NULL, tmpStr.c_str(), "message", MB_OK);

	return false;
}

bool SetDirectInputCooperativeLevel()
{
	HRESULT hr;

	hr = directSound->SetCooperativeLevel(wndHandle, DSSCL_PRIORITY);

	if (FAILED(hr))
		return false;

	return true;
}

bool CreateSecondSoundBuffer()
{
	HRESULT hr;
	WAVEFORMATEX wfx;

	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));

	wfx.wFormatTag = (WORD)WAVE_FORMAT_PCM;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = 22050;
	wfx.wBitsPerSample = 16;
	wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
	wfx.nAvgBytesPerSec = (DWORD)(wfx.nSamplesPerSec * wfx.nBlockAlign);

	DSBUFFERDESC dsbd;

	ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));

	dsbd.dwSize = sizeof(DSBUFFERDESC);
	dsbd.dwFlags = 0;
	dsbd.dwBufferBytes = 64000;
	dsbd.guid3DAlgorithm = GUID_NULL;
	dsbd.lpwfxFormat = &wfx;

	hr = directSound->CreateSoundBuffer
	(
		&dsbd, 
		&DSbuffer, 
		NULL
	);

	if (FAILED(hr))
		return false;

	return true;
}