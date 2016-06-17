#include <windows.h>
#include <stdint.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <immintrin.h>

#define internal static 
#define local_persist static 
#define global_variable static 

typedef int8_t  int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int BytesPerPixel;
	int Pitch;
};

global_variable bool Running;
global_variable win32_offscreen_buffer GlobalBackBuffer;

struct win32_window_dimension
{
	int Width;
	int Height;
};

struct yInfo
{
	int64_t start;
	int64_t end;
};

double scale = 1.0;
double x_pos = 0.0;
double y_pos = 0.0;

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
	win32_window_dimension Result;

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return Result;
}

internal void
OutputDebugInt64(int64_t number)
{
	char buffer[256];
	int i = 0;

	if(number > 0)
	{
		// Get 10^(Digit Count)
		int64_t power = 1;
		while((number % (power * 10)) != number)
			power = power * 10;

		// Find each digit
		while(power != 0)
		{
			int remains = number % power;
			buffer[i++] = ((number - remains) / power) + 48;
			number = remains;
			power = power / 10;
		}
	}

	buffer[i++] = '\n';
	buffer[i] = '\0';

	OutputDebugStringA(buffer);
}

extern "C" 
__m256d mandelbrot_iteration(
		__m256d x_scaled_zero_x4,
		__m256d y_scaled_zero_x4,
		__m256d n_x4,
		__m256d m_x4,
		__m256d iteration_x4,
		__m256d result_iteration_x4,
		__m256d negative_one_x4,
		__m256d one_x4,
		__m256d two_x4,
		__m256d s1024_x4);

internal void
render(win32_offscreen_buffer *Buffer, int starty, int endy)
{
	double x_max = 1.0 * scale + x_pos;
	double x_min = -2.5 * scale + x_pos;
	double y_max = 1.0 * scale + y_pos;
	double y_min = -1.0 * scale + y_pos;

	double x_scaler = (x_max - x_min) / (double)Buffer->Width;
	double y_scaler = (y_max - y_min) / (double)Buffer->Height;

	__m256d x_scaler_x4 = _mm256_set1_pd(x_scaler);
	__m256d x_min_x4 = _mm256_set1_pd(x_min);

	double two_array[4] = {2,2,2,2};
	double s1024_array[4] = {1 << 16, 1 << 16, 1 << 16, 1 << 16};
	double negative_1_array[4] = {-1, -1, -1, -1};
	__m256d negative_one_x4 = _mm256_set1_pd(-1);
	__m256d one_x4 = _mm256_set1_pd(1);
	__m256d two_x4 = _mm256_load_pd(two_array);
	__m256d s1024_x4 = _mm256_load_pd(s1024_array);
	__m256d numbers = _mm256_set_pd(3,2,1,0);
	__m256d _255_x4 = _mm256_set1_pd(255.0);

	int max_iteration = 500;
	__m256d max_iteration_x4 = _mm256_set1_pd(500.0);

	int64_t lowestCount = 1000000;
	int64_t iterationCount = 0;

	uint8 *Row = (uint8 *)Buffer->Memory;
	Row = Row + (Buffer->Pitch * starty);
	for(int Y = starty; 
		Y < endy; 
		++Y)
	{
		double y_scaled_zero = (Y * y_scaler) + y_min;  // scaled x (-2.5, 1)
		double y_scaled_zero_array[4] = {y_scaled_zero, y_scaled_zero, y_scaled_zero, y_scaled_zero};
		__m256d y_scaled_zero_x4 = _mm256_load_pd(y_scaled_zero_array);

		__m256d n_x4 = _mm256_setzero_pd();
		__m256d m_x4 = _mm256_setzero_pd();
		__m256d zero_x4_1 = _mm256_setzero_pd();

		__m256d four_x4 = _mm256_set1_pd(4);
		__m256d X_x4 = _mm256_set_pd(3,2,1,0);

		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0; 
			X < Buffer->Width; 
			X = X + 4)
		{
			// Scale x and y
			X_x4 = _mm256_add_pd(X_x4, four_x4);
			__m256d x_scaled_zero_x4 = _mm256_add_pd(_mm256_mul_pd(X_x4, x_scaler_x4), x_min_x4);

			n_x4 = zero_x4_1;
			m_x4 = zero_x4_1;
			__m256d iteration_x4 = _mm256_setzero_pd();
			__m256d result_iteration_x4 = _mm256_load_pd(negative_1_array);

			result_iteration_x4 = mandelbrot_iteration(
					x_scaled_zero_x4,
					y_scaled_zero_x4,
					n_x4,
					m_x4,
					iteration_x4,
					result_iteration_x4,
					negative_one_x4,
					one_x4,
					two_x4,
					s1024_x4);

			//result_iteration_x4 = mandelbrot_iteration(x_scaled_zero_x4, y_scaled_zero_x4);

			// (1 - d) * 255 : Inverse color and cast to 8 bit (for subpixel assignment)
			__m256d color_x4 = _mm256_sub_pd(one_x4, _mm256_div_pd(result_iteration_x4, max_iteration_x4));
			color_x4 = _mm256_mul_pd(color_x4, _255_x4);
			__m128i color_i32_x4 = _mm256_cvtpd_epi32(color_x4);

			// pixel = color << 16 | color << 8 | color : Now we need to assign subpixels
			__m128i pixel_result_x4 = color_i32_x4;
			__m128i shift_8_x4 = _mm_slli_epi32(color_i32_x4, 8);
			__m128i shift_16_x4 = _mm_slli_epi32(color_i32_x4, 16);
			pixel_result_x4 = _mm_or_si128(pixel_result_x4, shift_8_x4);
			pixel_result_x4 = _mm_or_si128(pixel_result_x4, shift_16_x4);

			// Assign each pixel result to the appropriate pixel buffer location
			*(__m128i *)Pixel = pixel_result_x4;
			Pixel = Pixel + 4;
		}

		//if(Y > 100 && Y % 200 == 0)
		//	OutputDebugInt64(10000000);

		Row += Buffer->Pitch;
	}
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
	if(Buffer->Memory)
	{
		VirtualFree(
			Buffer->Memory,
			0,
			MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = (Buffer->Width * Buffer->Height) * Buffer->BytesPerPixel;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

	Buffer->Pitch = Buffer->Width * Buffer->BytesPerPixel;
}

internal void
Win32CopyBufferToWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, 
						int WindowWidth, int WindowHeight)
{
	StretchDIBits(DeviceContext, 
		0, 0, WindowWidth, WindowHeight,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK 
Win32MainWindowCallback(HWND   Window,
						UINT   Message,
						WPARAM WParam,
						LPARAM LParam)
{
	LRESULT Result = 0;

	switch(Message)
	{
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			uint32 VKCode = WParam;
			bool WasDown = ((LParam & 1 << 30) != 0);
			bool IsDown = ((LParam & 1 << 31) == 0);

			//if(WasDown == IsDown) // comment out for repeating input
			//	break;

			switch(VKCode)
			{
				case VK_UP:
				{
					if(IsDown)
						y_pos = y_pos - (0.7 * scale);
				} break;
				case VK_DOWN:
				{
					if(IsDown)
						y_pos = y_pos + (0.7 * scale);
				} break;
				case VK_LEFT:
				{
					if(IsDown)
						x_pos = x_pos - (0.7 * scale);
				} break;
				case VK_RIGHT:
				{
					if(IsDown)
						x_pos = x_pos + (0.7 * scale);
				} break;
				case VK_OEM_PLUS:
				{
					if(IsDown)
						scale = scale / 2.0;
				} break;
				case VK_OEM_MINUS:
				{
					if(IsDown)
						scale = scale * 2.0;
				} break;
				case VK_ESCAPE:
				{
					if(IsDown)
						Running = false;
				} break;
			}
		} break;

		case WM_SIZE: // when user resizes window
		{
		} break;

		case WM_DESTROY:
		{
			Running = false;
			OutputDebugStringA("WM_DESTROY\n");
		} break;

		case WM_CLOSE:
		{
			Running = false;
			OutputDebugStringA("WM_CLOSE\n");
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;
			LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;

			win32_window_dimension Dimension = Win32GetWindowDimension(Window);

			Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, 
									Dimension.Width, Dimension.Height);
			EndPaint(Window, &Paint);

		} break;

		default:
		{
			//OutputDebugStringA("defeault\n");
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} 
	}

	return(Result);
}

// TODO: Remove
DWORD myThreadProc(LPVOID lpParameter)
{
	for(;;)
	{
		int64_t count = __rdtsc();

		yInfo* y_info = (yInfo *)lpParameter;
		render(&GlobalBackBuffer, y_info->start, y_info->end);

		count = __rdtsc() - count;
		OutputDebugInt64(count);
	}
}

int CALLBACK
WinMain(HINSTANCE Instance,
  		HINSTANCE PrevInstance,
  		LPSTR     CommandLine,
  		int       ShowCode)
{
	WNDCLASS WindowClass = {};

	int BufferWidth = 1200;
	int BufferHeight = 800;

	Win32ResizeDIBSection(&GlobalBackBuffer, BufferWidth, BufferHeight);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	//WindowClass.hIcon = ;
	//WindowClass.hCursor = ;
	//WindowClass.hbrBackground = ;
	//WindowClass.lpszMenuName = 0;
	WindowClass.lpszClassName = "MandelbrotWindowClass";

	if(RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowEx(
			0,
			WindowClass.lpszClassName,
			"Mandelbrot",
			WS_OVERLAPPEDWINDOW|WS_VISIBLE,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			BufferWidth, //or use CW_USEDEFAULT
			BufferHeight,  //or use CW_USEDEFAULT
			0,
			0,
			Instance,
			0);

		if(Window)
		{
			const int threadCount = 12;
			int dHeight = BufferHeight / threadCount;
			yInfo y_info[threadCount];
			for(int i = 0; i < threadCount; ++i)
			{
				int start = i * dHeight;
				int end = i == (threadCount - 1) ? BufferHeight : start + dHeight;
				y_info[i].start = start;
				y_info[i].end = end;
				HANDLE myThread = CreateThread(0, 0, myThreadProc, &y_info[i], 0, 0);
				CloseHandle(myThread);
				//render(&GlobalBackBuffer, &y_info);
			}

			Running = true;
			while(Running)
			{
				MSG Message;
				while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					if(Message.message == WM_QUIT)
					{
						Running = false;
					}

					TranslateMessage(&Message);
					DispatchMessage(&Message);
				}

#if 0
				int64_t count = __rdtsc();
				render(&GlobalBackBuffer, 0, BufferHeight);
				count = __rdtsc() - count;
				OutputDebugInt64(count);
#endif

				HDC DeviceContext = GetDC(Window);
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32CopyBufferToWindow(&GlobalBackBuffer, DeviceContext, 
										Dimension.Width, Dimension.Height);
				ReleaseDC(Window, DeviceContext);
			}
		}
		else
		{
			// TODO(Ashkan): Logging
		}
	}
	else
	{
		// TODO(Ashkan): Logging
	}

	return(0);
}
