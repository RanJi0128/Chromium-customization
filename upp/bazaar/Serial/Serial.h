#ifndef _SERIAL_H
#define _SERIAL_H

#include <Core/Core.h>

// module to manage serial ports and discover attached devices
namespace Upp {

class Serial
{
	private:
	
		// the file descriptor
#ifdef PLATFORM_POSIX
		int fd;
#else
		HANDLE fd;
#endif
		
		// error flag
		bool isError;
		
		// error code
		short errCode;
		
		// default baud rates
		static dword stdBauds[];
		static int stdBaudsCount;
		
	protected:
	
	public:
	
		// error codes
		enum
		{
			Ok,
			DeviceError,
			InvalidSpeed,
			InvalidParity,
			InvalidSize,
			InvalidStopBits
		};
	
		// parities
		enum
		{
			ParityNone,
			ParityEven,
			ParityOdd,
			ParityMark,
			ParitySpace
		};
	
		// constructor
		Serial();
		Serial(String const &port, dword speed, byte parity = ParityNone, byte bits = 8, byte stopBits = 1);
		
		// destructor
		~Serial();
		
		// open the port
		bool Open(String const &port, dword speed, byte parity = ParityNone, byte bits = 8, byte stopBits = 1);
		
		// close the port
		void Close(void);
		
		// control DTR and RTS lines
		bool SetDTR(bool on);
		bool SetRTS(bool on);

		// flush data
		bool FlushInput(void);
		bool FlushOutput(void);
		bool FlushAll(void);
		
		// check if data is available on serial port
		int Avail(void);
		
		// read a single byte, block 'timeout' milliseconds
		bool Read(byte &c, uint32_t timeout = 0);
		bool Read(char &c, uint32_t timeout = 0);
		
		// read data, requested amount, blocks 'timeout' milliseconds
		// return number of bytes got
		uint32_t Read(uint8_t *buf, uint32_t reqSize, uint32_t timeout = 0);

		// read data, requested amount, blocks 'timeout' milliseconds
		// if reqSize == 0 just read all available data, waiting for 'timeout' if != 0
		String Read(uint32_t reqSize = 0, uint32_t timeout = 0);
		
		// write a single byte
		bool Write(char c, uint32_t timeout = 0);
		
		// writes data
		bool Write(uint8_t const *buf, uint32_t len, uint32_t timeout = 0);
		bool Write(String const &data, uint32_t timeout = 0);
		
		// check if opened
		bool IsOpened(void) const;
		
		// check error flag
		bool IsError(void) const { return isError; }
		operator bool(void) const { return !isError; }
		bool operator!(void) const { return isError; }
		
		// get error code
		short GetErrorCode(void) const { return errCode; }

		// get a list of all serial ports
		static ArrayMap<String, String> GetSerialPorts(void);
		
		// get a list of standard baud rates
		static Index<dword> const &GetStandardBaudRates(void);
};

} // END_UPP_NAMESPACE

#endif
