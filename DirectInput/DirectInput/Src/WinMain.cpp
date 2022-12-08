#pragma comment (lib,"dxguid.lib") 
#pragma comment (lib,"dinput8.lib")
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
// For DirectInput
#include <dinput.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

// Global variables
std::vector<std::string> adapterDetails;
HINSTANCE hInst; // handle to hold the application instance
HWND wndHandle; // holds the windle handle
LPDIRECT3D9 pD3D; // Direct3D object
LPDIRECT3DDEVICE9 pd3dDevice; // d3d9 device

// Direct input
LPDIRECTINPUT8 directObj; // direct input object
LPDIRECTINPUTDEVICE8 directInputDevice; // direct input device
LPDIRECTINPUTDEVICE8 joyStick; 

// Declarations
bool InitWindow(HINSTANCE hInstance);
bool InitDirect3D();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void AddItemToList(std::string item);

// Input function declarations
bool CreateDirectInputObject(HINSTANCE hInstance);
bool CreateDirectInputDevice();
bool SetDataFormat();
bool AcquireAccess();
bool EnumerateDevices();
BOOL CALLBACK DeviceEnumCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
BOOL CALLBACK EnumObjCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
void CleanUpDirectInput();
// force feedback
bool ForceFeedbackEnumDevices();

LPDIRECTINPUTDEVICE8 FFdev = NULL;

//
// for multiple devices
//
#define NUM_DEVICES 3

LPDIRECTINPUTDEVICE8 devices[NUM_DEVICES]; // array of input devices
LPDIRECTINPUT g_lpDI = NULL;
int curCount = 0; // count of current devices


// WinMain is a main entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize the window
	if (!InitWindow(hInstance))
		return false;

	// Initialize direct3d
	if (!InitDirect3D())
		return false;

	// Create direct input object
	if (!CreateDirectInputObject(hInstance))
		return false;

	// Create direct input device
	if (!CreateDirectInputDevice())
		return false;

	// Set directin input format
	if (!SetDataFormat())
		return false;

	// Acquire access
	if (!AcquireAccess())
		return false;

	if (!EnumerateDevices())
		return false;
	// Macro to check the state of key 
	//#define KEYDOWN(name, key) (name[key] & 0x80)

	// key buffer
	//char buffer[256];

	// Returns true or false for each button on mouse
	#define BUTTONDOWN(name, key) (name.rgbButtons[key] & 0x80)

	// Holds current state of the mouse
	DIMOUSESTATE mouseState;

	//
	// Joysticks
	//
	HRESULT hr;

	hr = DirectInput8Create
	(
		hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&g_lpDI,
		NULL
	);

	hr = g_lpDI->EnumDevices
	(
		DI8DEVCLASS_GAMECTRL,
		EnumDevicesCallback,
		NULL,
		DIEDFL_ATTACHEDONLY
	);

	if (FAILED(hr))
		return false;

	while (true)
	{
		//
		// Get keyboard input
		//
		/*directInputDevice->GetDeviceState(sizeof(buffer), (LPVOID)&buffer);

		if (KEYDOWN(buffer, DIK_UP))
			MessageBox(wndHandle, L"up", NULL, NULL);
		else if (KEYDOWN(buffer, DIK_DOWN))
			MessageBox(wndHandle, L"down", NULL, NULL);
		else if (KEYDOWN(buffer, DIK_LEFT))
			MessageBox(wndHandle, L"left", NULL, NULL);
		else if (KEYDOWN(buffer, DIK_RIGHT))
			MessageBox(wndHandle, L"right", NULL, NULL);*/

		//
		// Get mouse input
		//
		hr = directInputDevice->GetDeviceState(sizeof(mouseState), (LPVOID)&mouseState);
		
		//
		// Logic to reacquire the device after it was lost
		//
		if (FAILED(hr))
		{
			hr = directInputDevice->Acquire();

			while(hr == DIERR_INPUTLOST)
				hr = directInputDevice->Acquire();

			continue;
		}

		// If first mouse button is pressed
		if(BUTTONDOWN(mouseState, 0))
			MessageBox(wndHandle, L"mb1", NULL, NULL);
		else if (BUTTONDOWN(mouseState, 1))
			MessageBox(wndHandle, L"mb2", NULL, NULL);

		//
		// Get joystick input
		//
		/*DIJOYSTATE joyState;

		joyStick->Poll();

		directInputDevice->GetDeviceState(sizeof(DIJOYSTATE), (LPVOID)&joyState);

		pos += js.lX;*/


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

bool CreateDirectInputObject(HINSTANCE hInstance)
{
	HRESULT hr;

	// Create the direct input object
	hr = DirectInput8Create
	(
		hInstance, // hInstance
		DIRECTINPUT_VERSION, // version of direct input(default value)
		IID_IDirectInput8, // identifier of the required device(default value)
		(void**)&directObj, // direct input is going to be created in this variable
		NULL // null(default value)
	);

	// If creating the obj failed
	if (FAILED(hr))
		// Return fail
		return false;

	// Return ok
	return true;
}

bool CreateDirectInputDevice()
{
	HRESULT hr;

	// Create direct input device
	//hr = directObj->CreateDevice
	//(
	//	GUID_SysKeyboard, // can be GUID_SysKeyboard/mouse or any value returned from EnumDevices
	//	&directInputDevice, // create device in this variable
	//	NULL // NULL(default value)
	//);

	hr = directObj->CreateDevice
	(
		GUID_SysMouse, // can be GUID_SysKeyboard/mouse or any value returned from EnumDevices
		&directInputDevice, // create device in this variable
		NULL // NULL(default value)
	);

	// If creating the device failed
	if (FAILED(hr))
		// Return fail
		return false;

	// Return ok
	return true;
}

bool SetDataFormat()
{
	HRESULT hr;

	// Sets data format to keyboard
	//hr = directInputDevice->SetDataFormat(&c_dfDIKeyboard);

	hr = directInputDevice->SetDataFormat(&c_dfDIMouse);

	if (FAILED(hr))
		return false;

	return true;
}

bool SetCooperativeLevel()
{
	HRESULT hr;

	// Set the cooperative level
	hr = directInputDevice->SetCooperativeLevel(wndHandle, DISCL_FOREGROUND | DISCL_BACKGROUND);

	// Check if it failed
	if (FAILED(hr))
		// If it did return false
		return false;
	
	// Return true
	return true;
}

bool AcquireAccess()
{
	HRESULT hr;
	HRESULT Aqcuire(VOID);

	hr = directInputDevice->Acquire();

	if (FAILED(hr))
		return false;

	return true;
}

bool EnumerateDevices()
{
	HRESULT hr;

	hr = directObj->EnumDevices
	(
		DI8DEVCLASS_GAMECTRL,
		DeviceEnumCallback,
		NULL,
		DIEDFL_ATTACHEDONLY
	);

	if (FAILED(hr))
		return false;

	return true;
}

BOOL CALLBACK DeviceEnumCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	hr = directObj->CreateDevice
	(
		pdidInstance->guidInstance,
		&joyStick,
		NULL
	);

	if (FAILED(hr))
		return DIENUM_CONTINUE;

	return DIENUM_STOP;
}

BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	hr = directObj->CreateDevice
	(
		pdidInstance->guidInstance,
		&devices[curCount],
		NULL
	);

	if (FAILED(hr))
		return DIENUM_CONTINUE;
	else
	{
		++curCount;
		if (curCount >= NUM_DEVICES)
			return DIENUM_STOP;
	}

	return DIENUM_STOP;
}

BOOL CALLBACK EnumObjCallback(const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext)
{
	if (pdidoi->dwType & DIDFT_AXIS)
	{
		// Create DIPROPRANGE structure(holds the range)
		DIPROPRANGE diprg;

		diprg.diph.dwSize = sizeof(DIPROPRANGE);
		diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		diprg.diph.dwHow = DIPH_BYID;
		diprg.diph.dwObj = pdidoi->dwType;

		// Min and Max portions of the range
		diprg.lMin = -100;
		diprg.lMax = 100;

		HRESULT hr;

		// Set range of axis
		hr = joyStick->SetProperty(DIPROP_RANGE, &diprg.diph);

		// if result failed
		if (FAILED(hr))
			// tell enum objects to stop
			return DIENUM_STOP;
	}

	// Tells enum objects to continue to next obj in this device
	return DIENUM_CONTINUE;
}

void CleanUpDirectInput()
{
	if (directObj)
	{
		if (directInputDevice)
		{
			directInputDevice->Unacquire();
			directInputDevice->Release();
			directInputDevice = NULL;
		}

		directObj->Release();
		directObj = NULL;
	}
}

BOOL CALLBACK FFDeviceCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	hr = directObj->CreateDevice
	(
		pdidInstance->guidInstance,
		&FFdev,
		NULL
	);

	if (FAILED(hr))
		return DIENUM_CONTINUE;

	return DIENUM_STOP;
}

bool ForceFeedbackEnumDevices()
{
	HRESULT hr;

	hr = directObj->EnumDevices
	(
		DI8DEVCLASS_GAMECTRL,
		FFDeviceCallback,
		NULL,
		DIEDFL_ATTACHEDONLY | DIEDFL_FORCEFEEDBACK
	);
}