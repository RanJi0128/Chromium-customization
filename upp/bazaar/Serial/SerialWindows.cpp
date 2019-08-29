#include "Serial.h"

#ifdef WIN32

#include <tchar.h>
typedef DWORD REGSAM;

#ifdef USE_SETUPAPI
#include <setupapi.h>
#endif

namespace Upp
{

	dword Serial::stdBauds[] =
	{
		50, 75, 110, 134, 150, 200,
		300, 600, 1200, 1800, 2400,
		4800, 9600, 19200,
		38400, 57600, 115200, 128000, 256000
	};

	int Serial::stdBaudsCount = sizeof(Serial::stdBauds) / sizeof(dword);

	// constructor
	Serial::Serial()
	{
		isError = false;
		errCode = Ok;
		fd = INVALID_HANDLE_VALUE;
	}

	Serial::Serial(String const &port, unsigned long speed, byte parity, byte bits, byte stopBits)
	{
		isError = false;
		errCode = Ok;
		fd = INVALID_HANDLE_VALUE;
		Open(port, speed, parity, bits, stopBits);
	}

	// destructor
	Serial::~Serial()
	{
		Close();
	}

	// open the port
	bool Serial::Open(String const &port, dword speed, byte parity, byte bits, byte stopBits)
	{
		// open the device
		fd = CreateFile("\\\\.\\" + port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

		if (fd == INVALID_HANDLE_VALUE)
		{
			isError = true;
			errCode = DeviceError;
			return false;
		}

		DCB dcb;

		char dcbSz[50];
		COMMTIMEOUTS cmt;

		// clear dcb and set length
		FillMemory(&dcb, sizeof(dcb), 0);
		dcb.DCBlength = sizeof(dcb);

		// check baud is valid

		if (GetStandardBaudRates().Find(speed) < 0)
		{
			Close();
			isError = true;
			errCode = InvalidSpeed;
			return false;
		}

		// check parity */
		char cParity;

		switch (parity)
		{

			case ParityNone :
				cParity = 'n';
				break;

			case ParityEven :
				cParity = 'e';
				break;

			case ParityOdd :
				cParity = 'o';
				break;

			case ParityMark :

			case ParitySpace :

			default:
				{
					Close();
					isError = true;
					errCode = InvalidParity;
					return false;
				}
		}

		// check data bits

		if (bits < 5 || bits > 8)
		{
			Close();
			isError = true;
			errCode = InvalidSize;
			return false;
		}

		// check stop bits

		if (stopBits < 1 || stopBits > 2)
		{
			Close();
			isError = true;
			errCode = InvalidStopBits;
			return false;
		}

		// build dcb
		sprintf_s(dcbSz, 49, "%d,%c,%d,%d", speed, cParity, bits, stopBits);

		if (!BuildCommDCB(dcbSz, &dcb))
		{
			Close();
			isError = true;
			errCode = DeviceError;
			return false;
		}

		/*
		    dcb.BaudRate = speed;
		    if(stopBits == 1)
			    dcb.StopBits = ONESTOPBIT;
		    else
			    dcb.StopBits = TWOSTOPBITS;
		    switch(parity)
		    {
		        case ParityNone:
				    dcb.Parity = PARITY_NONE;
				    break;
		        case ParityEven:
				    dcb.Parity = PARITY_EVEN;
				    break;
		        case ParityOdd:
				    dcb.Parity = PARITY_ODD;
				    break;
				default:
					break;
		    }
		    switch(bits)
		    {
		        case 5:
				    dcb.ByteSize = DATABITS_5;
				    break;
		        case 7:
				    dcb.ByteSize = DATABITS_7;
				    break;
		        case 8:
				    dcb.ByteSize = DATABITS_8;
				    break;
				default:
					break;
		    }
		    dcb.fDtrControl = 0;
		    dcb.fRtsControl = 0;
		*/


		// set dcb to serial port */

		if (!SetCommState(fd, &dcb))
		{
			Close();
			isError = true;
			errCode = DeviceError;
			return false;
		}

		// set input and output bufffer sizes

		if (!SetupComm(fd, 1024, 1024))
		{
			Close();
			isError = true;
			errCode = DeviceError;
			return false;
		}

		// set timeouts to 0 -- non blocking
		cmt.ReadIntervalTimeout = MAXDWORD;

		cmt.ReadTotalTimeoutMultiplier = 0;

		cmt.ReadTotalTimeoutConstant = 0;

		cmt.WriteTotalTimeoutMultiplier = 0;

		cmt.WriteTotalTimeoutConstant = MAXDWORD;

		if (!SetCommTimeouts(fd, &cmt))
		{
			Close();
			isError = true;
			errCode = DeviceError;
			return false;
		}

		return true;
	}

	// close the port
	void Serial::Close(void)
	{
		if (fd == INVALID_HANDLE_VALUE)
			return;

		// cancel pending i/o
		CancelIo(fd);

		// close the device
		CloseHandle(fd);

		fd = INVALID_HANDLE_VALUE;
	}

	// control DTR and RTS lines
	bool Serial::SetDTR(bool on)
	{
		if (on)
			EscapeCommFunction(fd, SETDTR);
		else
			EscapeCommFunction(fd, CLRDTR);

		return true;
	}

	bool Serial::SetRTS(bool on)
	{
		if (on)
			EscapeCommFunction(fd, SETRTS);
		else
			EscapeCommFunction(fd, CLRRTS);

		return true;
	}

	// flush data
	bool Serial::FlushInput(void)
	{
		return PurgeComm(fd, PURGE_RXCLEAR);
	}

	bool Serial::FlushOutput(void)
	{
		return PurgeComm(fd, PURGE_TXCLEAR);
	}

	bool Serial::FlushAll(void)
	{
		return PurgeComm(fd, PURGE_RXCLEAR) & PurgeComm(fd, PURGE_TXCLEAR);
	}


	// check if data is available on serial port
	int Serial::Avail(void)
	{
		dword errors;
		COMSTAT stat;

		if (!ClearCommError(fd, &errors, &stat))
			return 0;

		return stat.cbInQue;
	}


	// read a single byte, block 'timeout' milliseconds
	bool Serial::Read(byte &c, uint32_t timeout)
	{
		char buf[1];
		unsigned long count = 0;

		uint32_t tim = (uint32_t) msecs() + timeout;

		for (;;)
		{
			count = 0;
			ReadFile(fd, buf, 1, &count, NULL);

			if (!count)
			{
				if ((uint32_t) msecs() > tim)
					return false;

				continue;
			}

			c = (byte) buf[0];

			return true;
		}
	}

	bool Serial::Read(char &c, uint32_t timeout)
	{
		char buf[1];
		unsigned long count = 0;

		uint32_t tim = (uint32_t) msecs() + timeout;

		for (;;)
		{
			count = 0;
			ReadFile(fd, buf, 1, &count, NULL);

			if (!count)
			{
				if ((uint32_t) msecs() > tim)
					return false;

				continue;
			}

			c = buf[0];

			return true;
		}
	}

	// read data, requested amount, blocks 'timeout' milliseconds
	// return number of bytes got
	uint32_t Serial::Read(uint8_t *buf, uint32_t reqSize, uint32_t timeout)
	{
		if(!reqSize || !buf)
			return 0;
		
		unsigned long count;
		uint32_t tim = (uint32_t)msecs() + timeout;
		uint32_t req = reqSize;
		while(req)
		{
			count = 0;
			ReadFile(fd, buf, req, &count, NULL);

			req -= count;
			buf += count;
			
			if(!req || (uint32_t)msecs() > tim)
				break;
		}
		
		return reqSize - req;
	}

	// read data, requested amount, blocks 'timeout' milliseconds
	// if reqSize == 0 just read all available data, waiting for 'timeout' if != 0
	String Serial::Read(uint32_t reqSize, uint32_t timeout)
	{
		char buf[1001];
		String res;

		unsigned long count = 0;
		unsigned long n;
		uint32_t tim = (uint32_t) msecs() + timeout;

		if (reqSize)
		{
			n = min(reqSize, (size_t) 1000);

			while (reqSize)
			{
				count = 0;
				ReadFile(fd, buf, n, &count, NULL);

				if (!count)
				{
					if ((uint32_t) msecs() > tim)
						break;

					continue;
				}

				tim = (uint32_t) msecs() + timeout;

				if (count)
				{
					reqSize -= count;
					res.Cat(buf, count);
				}

				n = min(reqSize, (size_t) 1000);
			}
		}

		else
		{
			for (;;)
			{
				count = 0;
				ReadFile(fd, buf, 1000, &count, NULL);

				if (!count)
				{
					if ((uint32_t) msecs() > tim)
						break;

					continue;
				}

				tim = (uint32_t) msecs() + timeout;

				if (count)
					res.Cat(buf, count);
			}
		}

		return res;
	}

	// write a single byte
	bool Serial::Write(char c, uint32_t timeout)
	{
		unsigned long count = 0;

		if (!timeout)
		{
			WriteFile(fd, &c, 1, &count, NULL);
			return count == 1;
		}

		else
		{
			uint32_t tim = msecs() + timeout;

			for (;;)
			{
				WriteFile(fd, &c, 1, &count, NULL);
				if (count == 1)
					return true;

				if ((uint32_t) msecs() > tim)
					break;
			}

			return false;
		}
	}

	// write buffer
	bool Serial::Write(uint8_t const *buf, uint32_t len, uint32_t timeout)
	{
		unsigned long count = 0;

		if (!timeout)
		{
			WriteFile(fd, buf, len, &count, NULL);
			return count == len;
		}

		uint32_t tim = msecs() + timeout;
		const uint8_t *dPos = buf;

		for (;;)
		{
			WriteFile(fd, dPos, len, &count, NULL);
			if (count == len)
				return true;

			if (count > 0)
			{
				dPos += count;
				len -= count;
				continue;
			}

			if ((uint32_t)msecs() >= tim)
				break;
		}
		return false;
	}

	// writes string
	bool Serial::Write(String const &data, uint32_t timeout)
	{
		return Write((const uint8_t *)~data, data.GetCount(), timeout);
	}

	// check if opened
	bool Serial::IsOpened(void) const
	{
		return fd != INVALID_HANDLE_VALUE;
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_SETUPAPI

// ENUMERATE PORTS USING SETUPAPI

#ifndef GUID_DEVINTERFACE_COMPORT
	DEFINE_GUID(GUID_DEVINTERFACE_COMPORT, 0x86E0D1E0L, 0x8089, 0x11D0, 0x9C, 0xE4, 0x08, 0x00, 0x3E, 0x30, 0x1F, 0x73);
#endif

	typedef HKEY(__stdcall SETUPDIOPENDEVREGKEY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, DWORD, DWORD, REGSAM);
	typedef BOOL (__stdcall SETUPDICLASSGUIDSFROMNAME)(LPCTSTR, LPGUID, DWORD, PDWORD);
	typedef BOOL (__stdcall SETUPDIDESTROYDEVICEINFOLIST)(HDEVINFO);
	typedef BOOL (__stdcall SETUPDIENUMDEVICEINFO)(HDEVINFO, DWORD, PSP_DEVINFO_DATA);
	typedef HDEVINFO(__stdcall SETUPDIGETCLASSDEVS)(LPGUID, LPCTSTR, HWND, DWORD);
	typedef BOOL (__stdcall SETUPDIGETDEVICEREGISTRYPROPERTY)(HDEVINFO, PSP_DEVINFO_DATA, DWORD, PDWORD, PBYTE, DWORD, PDWORD);

	class CAutoHModule
	{

		public:
			CAutoHModule() : m_hModule(NULL), m_dwError(ERROR_SUCCESS) {}

			explicit CAutoHModule(HMODULE hModule) : m_hModule(hModule), m_dwError(GetLastError()) {}

			explicit CAutoHModule(HMODULE hModule, DWORD dwError) : m_hModule(hModule),	m_dwError(dwError) {}

			~CAutoHModule()
			{
				if (m_hModule != NULL)
				{
					FreeLibrary(m_hModule);
					m_hModule = NULL;
				}

				SetLastError(m_dwError);
			}

			operator HMODULE()
			{
				return m_hModule;
			}

			HMODULE m_hModule;
			DWORD m_dwError;
	};

	static HMODULE LoadLibraryFromSystem32(LPCTSTR lpFileName)
	{
		//Get the Windows System32 directory
		TCHAR szFullPath[_MAX_PATH];
		szFullPath[0] = _T('\0');

		if (::GetSystemDirectory(szFullPath, _countof(szFullPath)) == 0)
			return NULL;

		//Setup the full path and delegate to LoadLibrary
#pragma warning(suppress: 6102) //There is a bug with the SAL annotation of GetSystemDirectory in the Windows 8.1 SDK
		_tcscat_s(szFullPath, _countof(szFullPath), _T("\\"));

		_tcscat_s(szFullPath, _countof(szFullPath), lpFileName);

		return LoadLibrary(szFullPath);
	}

	static BOOL RegQueryValueString(HKEY kKey, LPCTSTR lpValueName, LPTSTR& pszValue)
	{
		//Initialize the output parameter
		pszValue = NULL;

		//First query for the size of the registry value
		DWORD dwType = 0;
		DWORD dwDataSize = 0;
		LONG nError = RegQueryValueEx(kKey, lpValueName, NULL, &dwType, NULL, &dwDataSize);

		if (nError != ERROR_SUCCESS)
		{
			SetLastError(nError);
			return FALSE;
		}

		//Ensure the value is a string

		if (dwType != REG_SZ)
		{
			SetLastError(ERROR_INVALID_DATA);
			return FALSE;
		}

		//Allocate enough bytes for the return value
		DWORD dwAllocatedSize = dwDataSize + sizeof(TCHAR);    //+sizeof(TCHAR) is to allow us to NULL terminate the data if it is not null terminated in the registry

		pszValue = reinterpret_cast<LPTSTR>(LocalAlloc(LMEM_FIXED, dwAllocatedSize));

		if (pszValue == NULL)
			return FALSE;

		//Recall RegQueryValueEx to return the data
		pszValue[0] = _T('\0');

		DWORD dwReturnedSize = dwAllocatedSize;

		nError = RegQueryValueEx(kKey, lpValueName, NULL, &dwType, reinterpret_cast<LPBYTE>(pszValue), &dwReturnedSize);

		if (nError != ERROR_SUCCESS)
		{
			LocalFree(pszValue);
			pszValue = NULL;
			SetLastError(nError);
			return FALSE;
		}

		//Handle the case where the data just returned is the same size as the allocated size. This could occur where the data
		//has been updated in the registry with a non null terminator between the two calls to ReqQueryValueEx above. Rather than
		//return a potentially non-null terminated block of data, just fail the method call

		if (dwReturnedSize >= dwAllocatedSize)
		{
			SetLastError(ERROR_INVALID_DATA);
			return FALSE;
		}

		//NULL terminate the data if it was not returned NULL terminated because it is not stored null terminated in the registry

		if (pszValue[dwReturnedSize/sizeof(TCHAR) - 1] != _T('\0'))
			pszValue[dwReturnedSize/sizeof(TCHAR)] = _T('\0');

		return TRUE;
	}

	static BOOL IsNumeric(LPCSTR pszString, BOOL bIgnoreColon)
	{
		size_t nLen = strlen(pszString);

		if (nLen == 0)
			return FALSE;

		//What will be the return value from this function (assume the best)
		BOOL bNumeric = TRUE;

		for (size_t i = 0; i < nLen && bNumeric; i++)
		{
			bNumeric = (isdigit(static_cast<int>(pszString[i])) != 0);

			if (bIgnoreColon && (pszString[i] == ':'))
				bNumeric = TRUE;
		}

		return bNumeric;
	}

	static BOOL IsNumeric(LPCWSTR pszString, BOOL bIgnoreColon)
	{
		size_t nLen = wcslen(pszString);

		if (nLen == 0)
			return FALSE;

		//What will be the return value from this function (assume the best)
		BOOL bNumeric = TRUE;

		for (size_t i = 0; i < nLen && bNumeric; i++)
		{
			bNumeric = (iswdigit(pszString[i]) != 0);

			if (bIgnoreColon && (pszString[i] == L':'))
				bNumeric = TRUE;
		}

		return bNumeric;
	}

	static BOOL QueryRegistryPortName(HKEY hDeviceKey, int& nPort)
	{
		//What will be the return value from the method (assume the worst)
		BOOL bAdded = FALSE;

		//Read in the name of the port
		LPTSTR pszPortName = NULL;

		if (RegQueryValueString(hDeviceKey, _T("PortName"), pszPortName))
		{
			//If it looks like "COMX" then
			//add it to the array which will be returned
			size_t nLen = _tcslen(pszPortName);

			if (nLen > 3)
			{
				if ((_tcsnicmp(pszPortName, _T("COM"), 3) == 0) && IsNumeric((pszPortName + 3), FALSE))
				{
					//Work out the port number
					nPort = _ttoi(pszPortName + 3);

					bAdded = TRUE;
				}
			}

			LocalFree(pszPortName);
		}

		return bAdded;
	}

	// get a list of all serial ports
	ArrayMap<String, String> Serial::GetSerialPorts(void)
	{
		ArrayMap<String, String> res;

		//Get the various function pointers we require from setupapi.dll
		CAutoHModule setupAPI(LoadLibraryFromSystem32(_T("SETUPAPI.DLL")));

		if (setupAPI == NULL)
			return res;

		SETUPDIOPENDEVREGKEY* lpfnSETUPDIOPENDEVREGKEY = reinterpret_cast<SETUPDIOPENDEVREGKEY*>(GetProcAddress(setupAPI, "SetupDiOpenDevRegKey"));

		/*
		#if defined _UNICODE
		*/
		SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = reinterpret_cast<SETUPDIGETCLASSDEVS*>(GetProcAddress(setupAPI, "SetupDiGetClassDevsW"));

		SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = reinterpret_cast<SETUPDIGETDEVICEREGISTRYPROPERTY*>(GetProcAddress(setupAPI, "SetupDiGetDeviceRegistryPropertyW"));

		/*
		#else
			SETUPDIGETCLASSDEVS* lpfnSETUPDIGETCLASSDEVS = reinterpret_cast<SETUPDIGETCLASSDEVS*>(GetProcAddress(setupAPI, "SetupDiGetClassDevsA"));

			SETUPDIGETDEVICEREGISTRYPROPERTY* lpfnSETUPDIGETDEVICEREGISTRYPROPERTY = reinterpret_cast<SETUPDIGETDEVICEREGISTRYPROPERTY*>(GetProcAddress(setupAPI, "SetupDiGetDeviceRegistryPropertyA"));

		#endif
		*/
		SETUPDIDESTROYDEVICEINFOLIST* lpfnSETUPDIDESTROYDEVICEINFOLIST = reinterpret_cast<SETUPDIDESTROYDEVICEINFOLIST*>(GetProcAddress(setupAPI, "SetupDiDestroyDeviceInfoList"));

		SETUPDIENUMDEVICEINFO* lpfnSETUPDIENUMDEVICEINFO = reinterpret_cast<SETUPDIENUMDEVICEINFO*>(GetProcAddress(setupAPI, "SetupDiEnumDeviceInfo"));

		if ((lpfnSETUPDIOPENDEVREGKEY == NULL) || (lpfnSETUPDIDESTROYDEVICEINFOLIST == NULL) ||
			(lpfnSETUPDIENUMDEVICEINFO == NULL) || (lpfnSETUPDIGETCLASSDEVS == NULL) || (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY == NULL))
		{
			//Set the error to report
			setupAPI.m_dwError = ERROR_CALL_NOT_IMPLEMENTED;

			return res;
		}

		//Now create a "device information set" which is required to enumerate all the ports
		GUID guid = GUID_DEVINTERFACE_COMPORT;

		HDEVINFO hDevInfoSet = lpfnSETUPDIGETCLASSDEVS(&guid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

		if (hDevInfoSet == INVALID_HANDLE_VALUE)
		{
			//Set the error to report
			setupAPI.m_dwError = GetLastError();

			return res;
		}

		//Finally do the enumeration
		BOOL bMoreItems = TRUE;

		int nIndex = 0;

		SP_DEVINFO_DATA devInfo;

		while (bMoreItems)
		{
			//Enumerate the current device
			devInfo.cbSize = sizeof(SP_DEVINFO_DATA);
			bMoreItems = lpfnSETUPDIENUMDEVICEINFO(hDevInfoSet, nIndex, &devInfo);

			if (bMoreItems)
			{
				//Did we find a serial port for this device
				BOOL bAdded = FALSE;
				String port, friendly;

				//Get the registry key which stores the ports settings
				HKEY hDeviceKey = lpfnSETUPDIOPENDEVREGKEY(hDevInfoSet, &devInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);

				if (hDeviceKey != INVALID_HANDLE_VALUE)
				{
					int nPort = 0;

					if (QueryRegistryPortName(hDeviceKey, nPort))
					{
						port = Format("COM%d", nPort);
						bAdded = TRUE;
					}

					//Close the key now that we are finished with it
					RegCloseKey(hDeviceKey);
				}

				//If the port was a serial port, then also try to get its friendly name

				if (bAdded)
				{
					TCHAR szFriendlyName[1024];
					szFriendlyName[0] = _T('\0');
					DWORD dwSize = sizeof(szFriendlyName);
					DWORD dwType = 0;

					if (lpfnSETUPDIGETDEVICEREGISTRYPROPERTY(hDevInfoSet, &devInfo, SPDRP_DEVICEDESC, &dwType, reinterpret_cast<PBYTE>(szFriendlyName), dwSize, &dwSize) && (dwType == REG_SZ))
						friendly = szFriendlyName;
					else
						friendly = "";

					res.Add(port, friendly);
				}
			}

			++nIndex;
		}

		//Free up the "device information set" now that we are finished with it
		lpfnSETUPDIDESTROYDEVICEINFOLIST(hDevInfoSet);

		return res;
	}

#else

	// ENUMERATE PORTS BY REGISTI ENTRIES

	// get a list of all serial ports
	ArrayMap<String, String> Serial::GetSerialPorts(void)
	{
		ArrayMap<String, String> res;

		HKEY hSERIALCOMM;

		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_QUERY_VALUE, &hSERIALCOMM) == ERROR_SUCCESS)
		{
			//Get the max value name and max value lengths
			DWORD dwMaxValueNameLen;
			DWORD dwMaxValueLen;
			DWORD dwQueryInfo = RegQueryInfoKey(hSERIALCOMM, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &dwMaxValueNameLen, &dwMaxValueLen, NULL, NULL);

			if (dwQueryInfo == ERROR_SUCCESS)
			{
				DWORD dwMaxValueNameSizeInChars = dwMaxValueNameLen + 1; //Include space for the NULL terminator
				DWORD dwMaxValueNameSizeInBytes = dwMaxValueNameSizeInChars * sizeof(TCHAR);
				DWORD dwMaxValueDataSizeInChars = dwMaxValueLen / sizeof(TCHAR) + 1;    //Include space for the NULL terminator
				DWORD dwMaxValueDataSizeInBytes = dwMaxValueDataSizeInChars * sizeof(TCHAR);

				//Allocate some space for the value name and value data
				TCHAR *valueName = (TCHAR *)malloc(dwMaxValueNameSizeInBytes + 1);
				TCHAR *valueData = (TCHAR *)malloc(dwMaxValueDataSizeInChars + 1);

				//Enumerate all the values underneath HKEY_LOCAL_MACHINE\HARDWARE\DEVICEMAP\SERIALCOMM
				DWORD dwIndex = 0;
				DWORD dwType;
				DWORD dwValueNameSize = dwMaxValueNameSizeInChars;
				DWORD dwDataSize = dwMaxValueDataSizeInBytes;
				memset(valueName, 0, dwMaxValueNameSizeInBytes);
				memset(valueData, 0, dwMaxValueDataSizeInBytes);
				TCHAR* szValueName = valueName;
				BYTE* byValue = (BYTE *)valueData;
				LONG nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);

				while (nEnum == ERROR_SUCCESS)
				{
					//If the value is of the correct type, then add it to the array
					if (dwType == REG_SZ)
					{
						TCHAR* szPort = reinterpret_cast<TCHAR*>(byValue);
						res.Add(szPort, szPort);
					}

					//Prepare for the next time around
					dwValueNameSize = dwMaxValueNameSizeInChars;
					dwDataSize = dwMaxValueDataSizeInBytes;
					memset(valueName, 0, dwMaxValueNameSizeInBytes);
					memset(valueData, 0, dwMaxValueDataSizeInBytes);
					++dwIndex;
					nEnum = RegEnumValue(hSERIALCOMM, dwIndex, szValueName, &dwValueNameSize, NULL, &dwType, byValue, &dwDataSize);
				}

				free(valueName);
				free(valueData);
			}

			//Close the registry key now that we are finished with it
			RegCloseKey(hSERIALCOMM);

			if (dwQueryInfo != ERROR_SUCCESS)
				SetLastError(dwQueryInfo);
		}

		return res;
	}
#endif

} // END_UPP_NAMESPACE

#endif
