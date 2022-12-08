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
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE) // D3DFVF_XYZ no RHW means that we are using 3d coords. trying to draw without puting vertices through pipeline will result in no drawing

// Global variables
std::vector<std::string> adapterDetails;
HINSTANCE hInst; // handle to hold the application instance
HWND wndHandle; // holds the windle handle
LPDIRECT3D9 pD3D; // Direct3D object
LPDIRECT3DDEVICE9 pd3dDevice; // d3d9 device
LPD3DXMESH mesh;
DWORD m_dwNumMaterials;
LPD3DXMESH loadedMesh;
LPD3DXBUFFER pD3DXMtrlBuffer;
D3DMATERIAL9* p_MeshMaterials;

struct CUSTOMVERTEX
{
	FLOAT x, y, z; // untransformed 3d position for the vertex
	DWORD color; // macro D3DCOLOR_ARGB(a,r,g,b)
};
// Declarations
bool InitWindow(HINSTANCE hInstance);
bool InitDirect3D();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void Render();
void CleanUp();
void AddItemToList(std::string item);
HRESULT CreateMesh();
HRESULT CreateMeshFVF();
HRESULT SetupMesh();
void GetMeshDetails();
void OptimizeMesh();
HRESULT SaveMeshToXFile();
HRESULT LoadMeshFromXFile();

// WinMain is a main entry point for windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	// Initialize the window
	if (!InitWindow(hInstance))
		return false;

	// Initialize direct3d
	if (!InitDirect3D())
		return false;

	// Create mesh
	if (CreateMeshFVF() == E_FAIL)
		return false;

	// Setup mesh
	if (SetupMesh() == E_FAIL)
		return false;
	
	// Optimize the mesh
	//OptimizeMesh();

	// Shows vertex and face count of the 3d model
	//GetMeshDetails();

	/*if (SaveMeshToXFile() == E_FAIL)
		return false;*/

	if (LoadMeshFromXFile() == E_FAIL)
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
	wcex.lpszMenuName = L"DirectX9Meshes";    // name of menu resource 
	wcex.lpszClassName = L"DirectX9Meshes";  // name of window class 
	wcex.hIconSm = 0; // handle to the small icon
	RegisterClassEx(&wcex);

	// Create window
	wndHandle = CreateWindowW
	(
		L"DirectX9Meshes", // window class to use
		L"DirectX9Meshes", // window title
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

	D3DXMATRIX finalMat = matRotateX * matRotateY * matTranslate;

	// tell Direct3D about our matrix
	pd3dDevice->SetTransform(D3DTS_WORLD, &finalMat);

	D3DXMATRIX matView;    // the view transform matrix


	D3DXVECTOR3 camPos(0.0f, 0.0f, 10.0f);
	D3DXVECTOR3 lookPos(12.0f, 0.0f, 0.0f);
	D3DXVECTOR3 upDir(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&matView,
		&camPos,    // the camera position
		&lookPos,    // the look-at position
		&upDir);    // the up direction

	pd3dDevice->SetTransform(D3DTS_VIEW, &matView);    // set the view transform to matView

	D3DXMATRIX matProjection;     // the projection transform matrix

	D3DXMatrixPerspectiveFovLH(&matProjection,
		D3DXToRadian(45),    // the horizontal field of view
		(FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, // aspect ratio
		1.0f,    // the near view-plane
		1000.0f);    // the far view-plane

	pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProjection);    // set the projection

	for (DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		pd3dDevice->SetMaterial(&p_MeshMaterials[i]);

		loadedMesh->DrawSubset(i);
	}

	//mesh->DrawSubset(1);

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

	if (mesh != NULL)
		mesh->Release();
}

void AddItemToList(std::string item)
{
	adapterDetails.push_back(item);
}

HRESULT CreateMesh()
{
	// Result
	HRESULT hr;

	// D3DVERTEXELEMENT9 array
	D3DVERTEXELEMENT9 Declaration[MAX_FVF_DECL_SIZE];

	// Creates declaration needed by the D3DXCreateMesh func
	D3DXDeclaratorFromFVF(D3DFVF_CUSTOMVERTEX, Declaration);

	// Create the mesh
	hr = D3DXCreateMesh
	(
		12, // num of faces
		8, // num of vertices
		D3DXMESH_MANAGED, // index and vertex buffers created using managed memory
		Declaration, // array of D3DXVERTEXELEMENT9 objects
		pd3dDevice, // device
		&mesh // write created mesh to the mesh variable
	);

	// If result failed
	if (FAILED(hr))
		// Return fail
		return E_FAIL;

	// Return ok
	return S_OK;
}

HRESULT CreateMeshFVF()
{
	// Result
	HRESULT hr;

	// Create mesh based on vertex format instead of going through the D3DVERTEXELEMENT9 array. Mesh object is identical to the object that is created using D3DVERTEXELEMENT9 array
	hr = D3DXCreateMeshFVF
	(
		12, // num of faces
		8, // num of vertices
		D3DXMESH_MANAGED, // create index and vertex buffers using managed memory
		D3DFVF_CUSTOMVERTEX, // FVF
		pd3dDevice,
		&mesh // write the created mesh to the mesh variable
	);

	if (FAILED(hr))
		return E_FAIL;

	return S_OK;
}

HRESULT SetupMesh()
{
	// Result
	HRESULT hr;

	// Vertexes
	CUSTOMVERTEX vertices[] =
	{
		{ -3.0f, 3.0f, -3.0f, D3DCOLOR_XRGB(0, 0, 255), },    // vertex 0
		{ 3.0f, 3.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 0), },     // vertex 1
		{ -3.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(255, 0, 0), },   // vertex 2
		{ 3.0f, -3.0f, -3.0f, D3DCOLOR_XRGB(0, 255, 255), },  // vertex 3
		{ -3.0f, 3.0f, 3.0f, D3DCOLOR_XRGB(0, 0, 255), },     // vertex 4
		{ 3.0f, 3.0f, 3.0f, D3DCOLOR_XRGB(255, 0, 0), },	  // vertex 5
		{ -3.0f, -3.0f, 3.0f, D3DCOLOR_XRGB(0, 255, 0), },	  // vertex 6
		{ 3.0f, -3.0f, 3.0f, D3DCOLOR_XRGB(0, 255, 255), },   // vertex 7
	};

	void* pVertices;

	// Lock mesh vertex buffer
	hr = mesh->LockVertexBuffer(D3DLOCK_DISCARD, (void**)&pVertices);

	// If result failed
	if (FAILED(hr))
		// Return NULL
		return E_FAIL;

	// create the indices using an WORD array(16 bit values)
	// two faces per side of the cube
	WORD indices[] =
	{
		0, 1, 2,    // side 1
		2, 1, 3,
		4, 0, 6,    // side 2
		6, 0, 2,
		7, 5, 6,    // side 3
		6, 5, 4,
		3, 1, 7,    // side 4
		7, 1, 5,
		4, 5, 0,    // side 5
		0, 5, 1,
		3, 7, 2,    // side 6
		2, 7, 6,
	};

	// Copy the vertices to the void ptr pVertices
	memcpy(pVertices, vertices, sizeof(vertices));

	// Unlock the mesh vertex buffer
	mesh->UnlockVertexBuffer();

	void* pIndexPtr;

	// Lock mesh index buffer
	hr = mesh->LockIndexBuffer(0, &pIndexPtr);

	// if the buffer was not locked
	if (FAILED(hr))
		// return fail
		return E_FAIL;

	// copy indexes to the void ptr pIndexPtr
	memcpy(pIndexPtr, indices, sizeof(indices) * sizeof(WORD));

	// Unlock mesh index buffer
	mesh->UnlockIndexBuffer();

	// Return ok
	return S_OK;
}

void GetMeshDetails()
{
	std::string numVertices;
	sprintf_s((char*)numVertices.c_str(), 100, "numVertices: %d", mesh->GetNumVertices());
	MessageBoxA(NULL, numVertices.c_str(), "INFO", MB_OK);

	std::string numFaces;
	sprintf_s((char*)numFaces.c_str(), 100, "numFaces: %d", mesh->GetNumFaces());
	MessageBoxA(NULL, numFaces.c_str(), "INFO", MB_OK);
}

void OptimizeMesh()
{
	// Call optimize in place function to generate the attribute table
	mesh->OptimizeInplace(D3DXMESHOPT_ATTRSORT, 0, NULL, NULL, NULL);

	// number of attributes
	DWORD numAttr;
	
	// attribute table pointer
	D3DXATTRIBUTERANGE* attribTable = new D3DXATTRIBUTERANGE[2];

	// get num of items within the table
	mesh->GetAttributeTable(NULL, &numAttr);

	// get the whole table into the variable attribTable
	mesh->GetAttributeTable(attribTable, &numAttr);

	// Setup attributes for the first group
	attribTable[0].AttribId = 0;
	attribTable[0].FaceStart = 0;
	attribTable[0].FaceCount = 6;
	attribTable[0].VertexStart = 0;
	attribTable[0].VertexCount = 8;

	// Setup attributes for the second group
	attribTable[1].AttribId = 1;
	attribTable[1].FaceStart = 6;
	attribTable[1].FaceCount = 6;
	attribTable[1].VertexStart = 0;
	attribTable[1].VertexCount = 8;

	// Write attribute table back into the mesh
	mesh->SetAttributeTable
	(
		attribTable, // attrib table to write
		2 // group count
	);
}

HRESULT SaveMeshToXFile()
{
	HRESULT hr;
	D3DXMATERIAL material;

	hr = D3DXSaveMeshToX
	(
		L"cube.x",
		mesh,
		NULL,
		&material,
		NULL,
		1,
		D3DXF_FILEFORMAT_TEXT
	);

	if (FAILED(hr))
		return E_FAIL;

	return S_OK;
}

HRESULT LoadMeshFromXFile()
{
	HRESULT hr;

	hr = D3DXLoadMeshFromXA
	(
		"tiger.x",
		D3DXMESH_SYSTEMMEM,
		pd3dDevice,
		NULL,
		&pD3DXMtrlBuffer,
		NULL,
		&m_dwNumMaterials,
		&loadedMesh
	);

	if (FAILED(hr))
		return E_FAIL;

	D3DXMATERIAL* matMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();

	p_MeshMaterials = new D3DMATERIAL9[m_dwNumMaterials];

	for (DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		p_MeshMaterials[i] = matMaterials[i].MatD3D;

		p_MeshMaterials[i].Ambient = p_MeshMaterials[i].Diffuse;
	}

	return S_OK;
}