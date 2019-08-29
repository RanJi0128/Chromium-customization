#ifndef _IPNServer_IPNServer_h_
#define _IPNServer_IPNServer_h_

#include <Web/SSL/SSL.h>

NAMESPACE_UPP

class IPNServer : public ScgiServer
{
	private:
	
		void OnAccepted();
		void OnRequest();
		void OnClosed();
		
		String payServer;
		int payPort;

public:
	Callback WhenVerified;
	Callback WhenInvalid;

	// constructor - defaults to port 8787
	IPNServer(int port = 8787);
	
	// setup server and port for back confirmation message
	IPNServer &SetPayServer(String const &s) { payServer = s; return *this; }
	IPNServer &SetPayPort(int p) { payPort = p; return *this; }
	
	// setting of handler
	IPNServer &SetVerifiedHandler(Callback handler) { WhenVerified = handler; return *this; }
	IPNServer &SetInvalidHandler(Callback handler)  { WhenInvalid = handler; return *this; }
	
	// runs the server
	void Run(void);

	VectorMap<String, String> GetVerified() const;
};

END_UPP_NAMESPACE

#endif
