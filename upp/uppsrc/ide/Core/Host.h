enum { REMOTE_TIMEOUT = 2000 };

extern String LinuxHostConsole;

class Host {
public:
	virtual ~Host() {}
	
	struct FileInfo : Time, Moveable<FileInfo> {
		int length;
	};
	
	virtual String                GetEnvironment() = 0;
	virtual String                GetHostPath(const String& path) = 0;
	virtual String                GetLocalPath(const String& path) = 0;
	virtual String                NormalizePath(const String& path) = 0;
	virtual Vector<FileInfo>      GetFileInfo(const Vector<String>& path) = 0;
	virtual void                  DeleteFile(const Vector<String>& path) = 0;
	virtual void                  DeleteFolderDeep(const String& dir) = 0;
	virtual void                  ChDir(const String& path) = 0;
	virtual bool                  RealizeDir(const String& path) = 0;
	virtual bool                  SaveFile(const String& path, const String& data) = 0;
	virtual String                LoadFile(const String& path) = 0;
	virtual int                   Execute(const char *cmdline) = 0;
	virtual int                   ExecuteWithInput(const char *cmdline, bool noconvert) = 0;
	virtual int                   Execute(const char *cmdline, Stream& out, bool noconvert = false) = 0;
	virtual int                   AllocSlot() = 0;
	virtual bool                  Run(const char *cmdline, int slot, String key, int blitz_count) = 0;
	virtual bool                  Run(const char *cmdline, Stream& out, int slot, String key, int blitz_count) = 0;
	virtual bool                  Wait() = 0;
	virtual bool                  Wait(int slot) = 0;
	virtual void                  OnFinish(Event<>  cb) = 0;
	virtual One<AProcess>         StartProcess(const char *cmdline) = 0;
	virtual void                  Launch(const char *cmdline, bool console = false) = 0;
	virtual void                  AddFlags(Index<String>& cfg) = 0;
	
	virtual const Vector<String>& GetExecutablesDirs() const = 0;
	virtual const HostTools&      GetTools() const = 0;
};

class LocalHost : public Host {
public:
	Vector<String> exedirs;
	String         environment;

	String        *cmdout;
	void    DoDir(const String& s);

public: /* Host */
	LocalHost();
	
	String                GetEnvironment() override;
	String                GetHostPath(const String& path) override;
	String                GetLocalPath(const String& path) override;
	String                NormalizePath(const String& path) override;
	Vector<FileInfo>      GetFileInfo(const Vector<String>& path) override;
	void                  DeleteFile(const Vector<String>& path) override;
	void                  DeleteFolderDeep(const String& dir) override;
	void                  ChDir(const String& path) override;
	bool                  RealizeDir(const String& path) override;
	bool                  SaveFile(const String& path, const String& data) override;
	String                LoadFile(const String& path) override;
	int                   Execute(const char *cmdline) override;
	int                   ExecuteWithInput(const char *cmdline, bool noconvert) override;
	int                   Execute(const char *cmdline, Stream& out, bool noconvert = false) override;
	int                   AllocSlot() override;
	bool                  Run(const char *cmdline, int slot, String key, int blitz_count) override;
	bool                  Run(const char *cmdline, Stream& out, int slot, String key, int blitz_count) override;
	bool                  Wait() override;
	bool                  Wait(int slot) override;
	void                  OnFinish(Event<>  cb) override;
	One<AProcess>         StartProcess(const char *cmdline) override;
	void                  Launch(const char *cmdline, bool console) override;
	void                  AddFlags(Index<String>& cfg) override;
	
	const Vector<String>& GetExecutablesDirs() const override;
	const HostTools&      GetTools() const override;
	
private:
	bool HasPlatformFlag(const Index<String>& cfg);
	
private:
	One<HostTools> tools;
};

/*
struct RemoteHost : Host {
	String         host;
	int            port;
	String         os_type;
//	bool           transfer_files;
	Vector<String> path_map_local;
	Vector<String> path_map_remote;
	String         chdir_path;
	String         environment;

	static Time                TimeBase() { return Time(2000, 1, 1); }

	virtual String             GetEnvironment();
	virtual String             GetHostPath(const String& path);
	virtual String             GetLocalPath(const String& path);
	virtual String             NormalizePath(const String& path);
	virtual Vector<FileInfo>   GetFileInfo(const Vector<String>& path);
	virtual void               DeleteFile(const Vector<String>& path);
	virtual void               DeleteFolderDeep(const String& dir);
	virtual void               ChDir(const String& path);
	virtual void               RealizeDir(const String& path);
	virtual void               SaveFile(const String& path, const String& data);
	virtual String             LoadFile(const String& path);
	virtual int                Execute(const char *cmdline);
	virtual int                ExecuteWithInput(const char *cmdline);
	virtual int                Execute(const char *cmdline, Stream& out);
	virtual int                AllocSlot();
	virtual bool               Run(const char *cmdline, int slot, String key, int blitz_count);
	virtual bool               Run(const char *cmdline, Stream& out, int slot, String key, int blitz_count);
	virtual bool               Wait();
	virtual One<AProcess>  StartProcess(const char *cmdline);
	virtual void               Launch(const char *cmdline, bool console);
	virtual void               AddFlags(Index<String>& cfg);

	String                     RemoteExec(String cmd);
};
*/