#ifndef WIN32_API_H
#define WIN32_API_H

#include "types.h"
#include <stddef.h>

// Windows types
typedef u32 DWORD;
typedef u16 WORD;
typedef u8 BYTE;
typedef char* LPSTR;
typedef const char* LPCSTR;
typedef void* HANDLE;
typedef void* HWND;
typedef void* HDC;
typedef void* HINSTANCE;
typedef void* HMODULE;
typedef u32* LPDWORD;
typedef void* LPVOID;
typedef const void* LPCVOID;
typedef u32 BOOL;
typedef u32 UINT;
typedef u32 LONG;
typedef u64 ULONG_PTR;

// Windows constants
#define TRUE 1
#define FALSE 0
#define NULL 0
#define INVALID_HANDLE_VALUE ((HANDLE)-1)

// Windows error codes
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define ERROR_ACCESS_DENIED 5
#define ERROR_INVALID_HANDLE 6
#define ERROR_NOT_ENOUGH_MEMORY 8
#define ERROR_INVALID_PARAMETER 87

// Windows API function types
typedef HANDLE (*CreateFileA_t)(LPCSTR lpFileName, DWORD dwDesiredAccess, 
                               DWORD dwShareMode, LPVOID lpSecurityAttributes,
                               DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                               HANDLE hTemplateFile);

typedef BOOL (*ReadFile_t)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                          LPDWORD lpNumberOfBytesRead, LPVOID lpOverlapped);

typedef BOOL (*WriteFile_t)(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                           LPDWORD lpNumberOfBytesWritten, LPVOID lpOverlapped);

typedef BOOL (*CloseHandle_t)(HANDLE hObject);

typedef LPVOID (*VirtualAlloc_t)(LPVOID lpAddress, SIZE_T dwSize, 
                                DWORD flAllocationType, DWORD flProtect);

typedef BOOL (*VirtualFree_t)(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

typedef DWORD (*GetLastError_t)(void);

typedef void (*SetLastError_t)(DWORD dwErrCode);

// Windows API function pointers
extern CreateFileA_t CreateFileA;
extern ReadFile_t ReadFile;
extern WriteFile_t WriteFile;
extern CloseHandle_t CloseHandle;
extern VirtualAlloc_t VirtualAlloc;
extern VirtualFree_t VirtualFree;
extern GetLastError_t GetLastError;
extern SetLastError_t SetLastError;

// Windows API initialization
error_t win32_api_init(void);
void win32_api_shutdown(void);

// Windows API function implementations
HANDLE win32_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, 
                        DWORD dwShareMode, LPVOID lpSecurityAttributes,
                        DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes,
                        HANDLE hTemplateFile);

BOOL win32_ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead,
                   LPDWORD lpNumberOfBytesRead, LPVOID lpOverlapped);

BOOL win32_WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite,
                    LPDWORD lpNumberOfBytesWritten, LPVOID lpOverlapped);

BOOL win32_CloseHandle(HANDLE hObject);

LPVOID win32_VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, 
                         DWORD flAllocationType, DWORD flProtect);

BOOL win32_VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType);

DWORD win32_GetLastError(void);
void win32_SetLastError(DWORD dwErrCode);

#endif // WIN32_API_H 