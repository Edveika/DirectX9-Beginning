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

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1) // D3DFVF_XYZ no RHW means that we are using 3d coords. trying to draw without puting vertices through pipeline will result in no drawing

// Global variables
std::vector<std::string> adapterDetails;
HINSTANCE hInst; // handle to hold the application instance
HWND wndHandle; // holds the windle handle
LPDIRECT3D9 pD3D; // Direct3D object
LPDIRECT3DDEVICE9 pd3dDevice; // d3d9 device
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
LPDIRECT3DINDEXBUFFER9 g_pIB = NULL;
LPDIRECT3DTEXTURE9 texture = NULL;

struct CUSTOMVERTEX
{
	FLOAT x, y, z; // untransformed 3d position for the vertex
	DWORD color; // macro D3DCOLOR_ARGB(a,r,g,b)
};

struct TEXTUREVERTEX
{
	FLOAT x, y, z; 
	FLOAT u, v;
};

// Declarations
bool InitWindow(HINSTANCE hInstance);
bool InitDirect3D();
bool LoadTextures();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Render();
void CleanUp();
void AddItemToList(std::string item);
HRESULT SetupVertexBuffer();
HRESULT SetupIndexBuffer();

// WinMain is a main entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize the window
	if (!InitWindow(hInstance))
		return false;

	// Initialize direct3d
	if (!InitDirect3D())
		return false;

	// Load textures
	if (!LoadTextures())
		return false;

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
			SetupVertexBuffer();
			SetupIndexBuffer();

			// Render function is being called once a frame
			Render();
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
	wcex.lpszMenuName = L"DirectX9LineLists";    // name of menu resource 
	wcex.lpszClassName = L"DirectX9LineLists";  // name of window class 
	wcex.hIconSm = 0; // handle to the small icon
	RegisterClassEx(&wcex);

	// Create window
	wndHandle = CreateWindowW
	(
		L"DirectX9LineLists", // window class to use
		L"DirectX9LineLists", // window title
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

	// turn off the 3D lighting
	pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

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

	// Clears the entire buffer, thats why you pass 0 and NULL as first two parameters
	pd3dDevice->Clear
	(
		0,
		NULL,
		D3DCLEAR_TARGET, // tells dx that you want the render buffer to be cleared
		D3DCOLOR_XRGB(0, 0, 0), // clears the screen to black color
		1.0f, // 1.0f gets passed into depth buffer. depth buffers helps dx determine how far the object uis from the viewer. (hols a vlue between 0.0f - 1.0f)
		0 // Stencil buffer allows for masking certain areas of an image, so they arent displayed. because stencil buffer is not used, we pass 0
	);

	pd3dDevice->BeginScene();

	pd3dDevice->SetTexture(0, texture);

	pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
	
	pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);

	// SET UP THE PIPELINE

	D3DXMATRIX matRotateX;    // a matrix to store the rotation information
	D3DXMATRIX matRotateY;    // a matrix to store the rotation information

	static float index = 0.0f; index += 0.05f;    // an ever-increasing float value

	// build a matrix to rotate the model based on the increasing float value
	D3DXMatrixRotationX(&matRotateX, index);
	D3DXMatrixRotationY(&matRotateY, index);

	D3DXMATRIX matTranslate;    // a matrix to store the translation information

	// build a matrix to move the model 12 units along the x-axis and 4 units along the y-axis
	// store it to matTranslate
	//static float move = 0.0f; move += 0.05f;
	D3DXMatrixTranslation(&matTranslate, 12.0f, 0.0f, 0.0f);

	D3DXMATRIX transformMatreix = matRotateX * matRotateY * matTranslate;

	// tell Direct3D about our matrix
	pd3dDevice->SetTransform(D3DTS_WORLD, &transformMatreix);

	D3DXMATRIX matView;    // the view transform matrix

	D3DXVECTOR3 cameraPos = D3DXVECTOR3(0.0f, 0.0f, 10.0f);
	D3DXVECTOR3 lookAtPos = D3DXVECTOR3(12.0f, 0.0f, 0.0f);
	D3DXVECTOR3 upDir = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&matView,
		&cameraPos,    // the camera position
		&lookAtPos,    // the look-at position
		&upDir);    // the up direction

	pd3dDevice->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		1000.0f);    // the far view-plane

	pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection

	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 4, 2);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 8, 2);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 12, 2);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 16, 2);
	pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 20, 2);

	//pd3dDevice->SetIndices(g_pIB);

	//pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, // PrimitiveType
	//	0,                  // BaseVertexIndex
	//	0,                  // MinIndex
	//	36,                  // NumVertices
	//	0,                  // StartIndex
	//	12);                // PrimitiveCount

	pd3dDevice->EndScene();

	// Present the back buffer contents to the display
	pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void CleanUp()
{
	if (pd3dDevice != NULL)
		pd3dDevice->Release();

	if (pD3D != NULL)
		pD3D->Release();

	if (g_pVB != NULL)
		g_pVB->Release();
}

void AddItemToList(std::string item)
{
	adapterDetails.push_back(item);
}

HRESULT SetupVertexBuffer()
{
	HRESULT hr;

	CUSTOMVERTEX vertices[] =
	{
		// 1
		{-2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,0,255,0)},
		{2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,0,255,0)},
		{-2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,0,255,0)},
		{2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,0,255,0)},

		//// 2
		{-2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{-2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,0,0,255)},

		// 3
		{-2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,255,0,0)},
		{2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,255,0,0)},
		{-2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,255,0,0)},
		{2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,255,0,0)},

		// 4
		{-2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,255, 119, 0)},
		{-2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,255, 119, 0)},
		{2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,255, 119, 0)},
		{2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,255, 119, 0)},

		// 5
		{2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,220, 255, 0)},
		{2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,220, 255, 0)},
		{2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,220, 255, 0)},
		{2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,220, 255, 0)},

		// 6
		{-2.0f, 2.0f, -2.0f, D3DCOLOR_ARGB(0,170, 0, 255)},
		{-2.0f, -2.0f, -2.0f, D3DCOLOR_ARGB(0,170, 0, 255)},
		{-2.0f, 2.0f, 2.0f, D3DCOLOR_ARGB(0,170, 0, 255)},
		{-2.0f, -2.0f, 2.0f, D3DCOLOR_ARGB(0,170, 0, 255)},
	};	

	hr = pd3dDevice->CreateVertexBuffer
	(
		sizeof(vertices) * sizeof(CUSTOMVERTEX), // size in bytes of the buffer
		0,
		D3DFVF_XYZRHW | D3DFVF_DIFFUSE, // because vertex is untransformed
		D3DPOOL_DEFAULT, // allocates the buffer in the most appropriate memory for this type
		&g_pVB, // this variable holds newly created buffer
		NULL
	);

	// If result failed
	if (FAILED(hr))
		// Return NULL
		return NULL;

	void* pVertices;

	// Lock tge verte buffer
	hr = g_pVB->Lock
	(
		0, // lock entire buffer(0 default)
		sizeof(vertices), // buffer size
		(void**)&pVertices, // write pointer to buffer
		0 // no flags
	);

	// If result failed
	if (FAILED(hr))
		// Return NULL
		return E_FAIL;

	memcpy(pVertices, vertices, sizeof(vertices));

	g_pVB->Unlock();

	return S_OK;
}

HRESULT SetupIndexBuffer()
{
	HRESULT hr;

	CUSTOMVERTEX vertices[] =
	{
		{-1.0f,1.0f,-1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{-1.0f,1.0f,-1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{1.0f,1.0f,-1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{1.0f,-1.0f,-1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{-1.0f,-1.0f,1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{1.0f,-1.0f,1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{1.0f,1.0f,1.0f, D3DCOLOR_ARGB(0,0,0,255)},
		{-1.0f,1.0f,1.0f, D3DCOLOR_ARGB(0,0,0,255)},
	};

	WORD indexes[] =
	{
		0,1,2,
		2,3,0,
		4,5,6,
		6,7,4,
		0,3,5,
		5,4,0,
		3,2,6,
		6,5,3,
		2,1,7,
		7,6,2,
		1,0,4,
		4,7,1
	};

	hr = pd3dDevice->CreateIndexBuffer
	(
		sizeof(indexes) * sizeof(WORD),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_DEFAULT,
		&g_pIB,
		NULL
	);

	if (FAILED(hr))
		return E_FAIL;

	void* indexPtr;

	hr = g_pIB->Lock(0, 0, (void**)&indexPtr, D3DLOCK_DISCARD);

	if (FAILED(hr))
		return E_FAIL;

	memcpy(vertices, indexes, sizeof(indexes));

	g_pIB->Unlock();

	return S_OK;
}

bool LoadTextures()
{
	HRESULT hr;

	hr = D3DXCreateTextureFromFile(pd3dDevice, L"brickWall.png", &texture);

	if (texture == NULL)
		return false;

	return true;
}