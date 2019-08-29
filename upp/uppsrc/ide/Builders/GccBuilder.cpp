#include "Builders.h"
#include "BuilderUtils.h"

void   GccBuilder::AddFlags(Index<String>& cfg)
{
}

String GccBuilder::CompilerName() const
{
	if(!IsNull(compiler)) return compiler;
	return "c++";
}

String GccBuilder::CmdLine(const String& package, const Package& pkg)
{
	String cc = CompilerName();
	cc << " -c ";
	cc << IncludesDefinesTargetTime(package, pkg);
	return cc;
}

void GccBuilder::BinaryToObject(String objfile, CParser& binscript, String basedir,
                                const String& package, const Package& pkg)
{
	String fo = BrcToC(binscript, basedir);
	String tmpfile = ForceExt(objfile, ".c");
	SaveFile(tmpfile, fo);
	String cc = CmdLine(package, pkg);
	cc << " -c -o " << GetHostPathQ(objfile) << " -x c " << GetHostPathQ(tmpfile);
	int slot = AllocSlot();
	if(slot < 0 || !Run(cc, slot, objfile, 1))
		throw Exc(NFormat("Error compiling binary object '%s'.", objfile));
}

bool GccBuilder::BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>& immfile,
	String& linkoptions, const Vector<String>& all_uses, const Vector<String>& all_libraries,
	int opt)
{
	SaveBuildInfo(package);
	
	int i;
	String packagepath = PackagePath(package);
	Package pkg;
	pkg.Load(packagepath);
	String packagedir = GetFileFolder(packagepath);
	ChDir(packagedir);
	PutVerbose("cd " + packagedir);
	IdeConsoleBeginGroup(package);
	Vector<String> obj;

	bool is_shared = HasFlag("SO");
	String shared_ext = (HasFlag("WIN32") ? ".dll" : ".so");

	Vector<String> sfile, isfile;
	Vector<String> soptions, isoptions;
	bool           error = false;

	String pch_header;
	
	Index<String> nopch, noblitz;

	bool blitz = HasFlag("BLITZ");
	bool release = !HasFlag("DEBUG");

	for(i = 0; i < pkg.GetCount(); i++) {
		if(!IdeIsBuilding())
			return false;
		if(!pkg[i].separator) {
			String gop = Gather(pkg[i].option, config.GetKeys());
			Vector<String> srcfile = CustomStep(pkg[i], package, error);
			if(srcfile.GetCount() == 0)
				error = true;
			for(int j = 0; j < srcfile.GetCount(); j++) {
				String fn = srcfile[j];
				String ext = GetSrcType(fn);
				if(findarg(ext, ".c", ".cpp", ".cc", ".cxx", ".brc", ".m", ".mm", ".s", ".ss") >= 0 ||
				   (!release && blitz && ext == ".icpp") ||
				   ext == ".rc" && HasFlag("WIN32")) {
					sfile.Add(fn);
					soptions.Add(gop);
				}
				else
				if(ext == ".icpp") {
					isfile.Add(fn);
					isoptions.Add(gop);
				}
				else
				if(ext == ".o")
					obj.Add(fn);
				else
				if(ext == ".a" || ext == ".so")
					linkfile.Add(fn);
				else
				if(IsHeaderExt(ext) && pkg[i].pch && allow_pch && !blitz) {
					if(pch_header.GetCount())
						PutConsole(GetFileName(fn) + ": multiple PCHs are not allowed. Check your package configuration");
					else
						pch_header = fn;
				}
				if(pkg[i].nopch) {
					nopch.Add(fn);
					if(allow_pch && release)
						noblitz.Add(fn);
				}
				if(pkg[i].noblitz)
					noblitz.Add(fn);
				if(ext == ".c")
					nopch.Add(fn);
			}
		}
	}

	String cc = CmdLine(package, pkg);
	if(HasFlag("WIN32")/* && HasFlag("MT")*/)
		cc << " -mthreads";
	if(HasFlag("DEBUG_MINIMAL"))
		cc << (HasFlag("WIN32") ? " -g1" : " -ggdb -g1");
	if(HasFlag("DEBUG_FULL"))
		cc << (HasFlag("WIN32") ? " -g2" : " -ggdb -g2");
	String fuse_cxa_atexit;
	if(is_shared /*&& !HasFlag("MAIN")*/) {
		cc << " -shared -fPIC";
		fuse_cxa_atexit = " -fuse-cxa-atexit";
	}
	if(!HasFlag("SHARED") && !is_shared)
		cc << " -static ";
//	else if(!HasFlag("WIN32")) // TRC 05/03/08: dynamic fPIC doesn't seem to work in MinGW
//		cc << " -dynamic -fPIC "; // TRC 05/03/30: dynamic fPIC doesn't seem to work in GCC either :-)
	cc << ' ' << Gather(pkg.option, config.GetKeys());
	cc << " -fexceptions";

	if (HasFlag("OSX11")) {
	  if (HasFlag("POWERPC"))
		cc << " -arch ppc";
	  if (HasFlag("X86"))
		cc << " -arch i386";
	}

//	if(HasFlag("SSE2")) {
//		cc << " -msse2";
//		if(!HasFlag("CLANG"))
//			cc << " -mfpmath=sse";
//	}

	if(!release)
		cc << " -D_DEBUG " << debug_options;
	else
		cc << ' ' << release_options;

	if(pkg.nowarnings)
		cc << " -w";

	int recompile = 0;
	Blitz b;
	if(blitz) {
		BlitzBuilderComponent bc(this);
		b = bc.MakeBlitzStep(sfile, soptions, obj, immfile, ".o", noblitz);
		recompile = b.build;
	}

	for(i = 0; i < sfile.GetCount(); i++) {
		String fn = sfile[i];
		String ext = ToLower(GetFileExt(fn));
		if(findarg(ext, ".rc", ".brc", ".c") < 0 && HdependFileTime(sfile[i]) > GetFileTime(CatAnyPath(outdir, GetFileTitle(fn) + ".o")))
			recompile++;
	}

	String pch_use;
	String pch_file;
	
	if(recompile > 2 && pch_header.GetCount()) {
		String pch_header2 = CatAnyPath(outdir, GetFileTitle(pch_header) + "$pch.h");
		pch_file = pch_header2 + ".gch";
		SaveFile(pch_header2, "#include <" + pch_header + ">"); // CLANG needs a copy of header
		
		int pch_slot = AllocSlot();
		StringBuffer sb;

		sb << Join(cc, cpp_options) << " -x c++-header " << GetHostPathQ(pch_header) << " -o " << GetHostPathQ(pch_file);

		PutConsole("PCH: " + GetFileName(pch_header));
		if(pch_slot < 0 || !Run(~sb, pch_slot, GetHostPath(pch_file), 1))
			error = true;
		Wait();

		pch_use = " -I" + GetHostPathQ(outdir) + " -include " + GetFileName(pch_header2) + " -Winvalid-pch ";
	}

	if(blitz && b.build) {
		PutConsole("BLITZ:" + b.info);
		int slot = AllocSlot();
		if(slot < 0 || !Run(String().Cat() << Join(cc, cpp_options) << ' '
		                    << GetHostPathQ(b.path)
		                    << " -o " << GetHostPathQ(b.object), slot, GetHostPath(b.object), b.count))
			error = true;
	}

	int first_ifile = sfile.GetCount();
	sfile.AppendPick(pick(isfile));
	soptions.AppendPick(pick(isoptions));

	int ccount = 0;
	for(i = 0; i < sfile.GetCount(); i++) {
		if(!IdeIsBuilding())
			return false;
		String fn = sfile[i];
		String ext = ToLower(GetFileExt(fn));
		bool rc = (ext == ".rc");
		bool brc = (ext == ".brc");
		bool init = (i >= first_ifile);
		String objfile = CatAnyPath(outdir, GetFileTitle(fn) + (rc ? "$rc.o" : brc ? "$brc.o" : ".o"));
		if(HdependFileTime(fn) > GetFileTime(GetHostPath(objfile))) {
			PutConsole(GetFileName(fn));
			int time = GetTickCount();
			bool execerr = false;
			if(rc) {
				String exec;
				exec << GetHostPathShort(FindInDirs(host->GetExecutablesDirs(), "windres.exe")) << " -i " << GetHostPathQ(fn);
				if(cc.Find(" -m32 ") >= 0)
					exec << " --target=pe-i386 ";
				exec << " -o " << GetHostPathQ(objfile) << IncludesShort(" --include-dir=", package, pkg)
				     << DefinesTargetTime(" -D", package, pkg) + (HasFlag("DEBUG")?" -D_DEBUG":"");
				PutVerbose(exec);
				int slot = AllocSlot();
				execerr = (slot < 0 || !Run(exec, slot, GetHostPath(objfile), 1));
			}
			else if(brc) {
				try {
					String brcdata = LoadFile(fn);
					if(brcdata.IsVoid())
						throw Exc(NFormat("error reading file '%s'", fn));
					CParser parser(brcdata, fn);
					BinaryToObject(GetHostPath(objfile), parser, GetFileDirectory(fn), package, pkg);
				}
				catch(Exc e) {
					PutConsole(e);
					execerr = true;
				}
			}
			else {
				String exec = cc;
				if(ext == ".c")
					exec << Join(" -x c", c_options) << ' ';
				else if(ext == ".s" || ext == ".S")
					exec << " -x assembler-with-cpp ";
				else if (ext == ".m" || ext == ".mm")
					exec << fuse_cxa_atexit << " -x objective-c++ ";
				else {
					exec << fuse_cxa_atexit << Join(" -x c++", cpp_options) << ' ';
					exec << pch_use;
				}
				exec << GetHostPathQ(fn)  << " " << soptions[i] << " -o " << GetHostPathQ(objfile);
				PutVerbose(exec);
				int slot = AllocSlot();
				execerr = (slot < 0 || !Run(exec, slot, GetHostPath(objfile), 1));
			}
			if(execerr)
				DeleteFile(objfile);
			error |= execerr;
			PutVerbose("compiled in " + GetPrintTime(time));
			ccount++;
		}
		immfile.Add(objfile);
		if(init)
			linkfile.Add(objfile);
		else
			obj.Add(objfile);
	}

	if(error) {
//		if(ccount)
//			PutCompileTime(time, ccount);
		IdeConsoleEndGroup();
		return false;
	}

	linkoptions << Gather(pkg.link, config.GetKeys());
	if(linkoptions.GetCount())
		linkoptions << ' ';

	Vector<String> libs = Split(Gather(pkg.library, config.GetKeys()), ' ');
	linkfile.Append(libs);

	if(pch_file.GetCount())
		OnFinish(callback1(DeletePCHFile, pch_file));

	int libtime = GetTickCount();
	if(!HasFlag("MAIN")) {
		if(HasFlag("BLITZ") && !HasFlag("SO")  || HasFlag("NOLIB")) {
			linkfile.Append(obj); // Simply link everything as .o files...
			IdeConsoleEndGroup();
//			if(ccount)
//				PutCompileTime(time, ccount);
			return true;
		}
		IdeConsoleEndGroup();
		if(!Wait())
			return false;
		String product;
		if(is_shared)
			product = GetSharedLibPath(package);
		else
			product = CatAnyPath(outdir, GetAnyFileName(package) + ".a");
		String hproduct = GetHostPath(product);
		Time producttime = GetFileTime(hproduct);
//		LOG("hproduct = " << hproduct << ", time = " << producttime);
		if(obj.GetCount()) {
			linkfile.Add(GetHostPath(product));
			immfile.Add(GetHostPath(product));
		}
		for(int i = 0; i < obj.GetCount(); i++)
			if(GetFileTime(obj[i]) > producttime) {
				String lib;
				if(is_shared) {
					lib = CompilerName();
					lib << " -shared -fPIC -fuse-cxa-atexit";
					if(!HasFlag("SHARED") && !is_shared)
						lib << " -static";
//					else if(!HasFlag("WIN32")) // TRC 05/03/08: dynamic fPIC causes trouble in MinGW
//						lib << " -dynamic -fPIC"; // TRC 05/03/30: dynamic fPIC doesn't seem to work in GCC either :-)
					if(HasFlag("GCC32"))
						lib << " -m32";
					Point p = ExtractVersion();
					if(!IsNull(p.x) && HasFlag("WIN32")) {
						lib << " -Xlinker --major-image-version -Xlinker " << p.x;
						if(!IsNull(p.y))
							lib << " -Xlinker --minor-image-version -Xlinker " << p.y;
					}
					lib << ' ' << Gather(pkg.link, config.GetKeys());
					
					lib << " -o ";
				}
				else
					lib = "ar -sr ";
				lib << GetHostPathQ(product);
				
		
				String llib;
				for(int i = 0; i < obj.GetCount(); i++)
					llib << ' ' << GetHostPathQ(obj[i]);
				PutConsole("Creating library...");
				DeleteFile(hproduct);
				if(is_shared) {
					for(int i = 0; i < libpath.GetCount(); i++)
						llib << " -L" << GetHostPathQ(libpath[i]);
					for(int i = 0; i < all_uses.GetCount(); i++)
						llib << ' ' << GetHostPathQ(GetSharedLibPath(all_uses[i]));
					for(int i = 0; i < all_libraries.GetCount(); i++)
						llib << " -l" << GetHostPathQ(all_libraries[i]);
					
					if(HasFlag("POSIX"))
						llib << " -Wl,-soname," << GetSoname(product);
				}

				String tmpFileName;
				if(lib.GetCount() + llib.GetCount() >= 8192)
				{
					tmpFileName = GetTempFileName();
					// we can't simply put all data on a single line
					// as it has a limit of around 130000 chars too, so we split
					// in multiple lines
					FileOut f(tmpFileName);
					while(llib != "")
					{
						int found = 0;
						bool quotes = false;
						int lim = min(8192, llib.GetCount());
						for(int i = 0; i < lim; i++)
						{
							char c = llib[i];
							if(isspace(c) && !quotes)
								found = i;
							else if(c == '"')
								quotes = !quotes;
						}
						if(!found)
							found = llib.GetCount();

						// replace all '\' with '/'
						llib = UnixPath(llib);
						
						f.PutLine(llib.Left(found));
						llib.Remove(0, found);
					}
					f.Close();
					lib << " @" << tmpFileName;
				}
				else
					lib << llib;

				int res = Execute(lib);
				if(tmpFileName != "")
					FileDelete(tmpFileName);
#ifdef PLATFORM_POSIX
				String folder, libF, soF, linkF;
				if(HasFlag("SO"))
				{
					folder = GetFileFolder(hproduct);
					libF = GetFileName(hproduct);
					soF = AppendFileName(folder, GetSoname(hproduct));
					linkF = AppendFileName(folder, GetSoLinkName(hproduct));
				}
#endif
				if(res) {
					DeleteFile(hproduct);
#ifdef PLATFORM_POSIX
					if(HasFlag("SO")) {
						DeleteFile(libF);
						DeleteFile(linkF);
					}
 #endif
					return false;
				}
#ifdef PLATFORM_POSIX
				if(HasFlag("SO"))
				{
					int r;
					r = symlink(libF, soF);
					r = symlink(libF, linkF);
					(void)r;
				}
#endif
				PutConsole(String().Cat() << hproduct << " (" << GetFileInfo(hproduct).length
				           << " B) created in " << GetPrintTime(libtime));
				break;
			}
		return true;
	}

	IdeConsoleEndGroup();
	obj.Append(linkfile);
	linkfile = pick(obj);
	return true;
}


bool GccBuilder::Link(const Vector<String>& linkfile, const String& linkoptions, bool createmap)
{
	if(!Wait())
		return false;
	PutLinking();
	int time = GetTickCount();
	for(int i = 0; i < linkfile.GetCount(); i++)
		if(GetFileTime(linkfile[i]) > targettime) {
			Vector<String> lib;
			String lnk = CompilerName();
			if(HasFlag("GCC32"))
				lnk << " -m32";
			if(HasFlag("DLL"))
				lnk << " -shared";
			if(!HasFlag("SHARED") && !HasFlag("SO"))
				lnk << " -static";
//			else if(!HasFlag("WIN32")) // TRC 05/03/08: see above
//				lnk << " -dynamic -fPIC"; // TRC 05/03/30: dynamic fPIC doesn't seem to work in GCC either :-)
			if(HasFlag("WINCE"))
				lnk << " -mwindowsce";
			else if(HasFlag("WIN32") && !HasFlag("CLANG")) {
				lnk << " -mwindows";
				// if(HasFlag("MT"))
					lnk << " -mthreads";
				if(!HasFlag("GUI"))
					lnk << " -mconsole";
			}
			lnk << " -o " << GetHostPathQ(target);
			if(createmap)
				lnk << " -Wl,-Map," << GetHostPathQ(GetFileDirectory(target) + GetFileTitle(target) + ".map");
			if(HasFlag("DEBUG_MINIMAL") || HasFlag("DEBUG_FULL"))
				lnk << " -ggdb";
			else
				lnk << (!HasFlag("OSX11") ? " -Wl,-s" : "");
			for(i = 0; i < libpath.GetCount(); i++)
				lnk << " -L" << GetHostPathQ(libpath[i]);
//			lnk << " -Wl,--gc-sections,-O,2 ";
			if(!HasFlag("OSX11"))
				lnk << " -Wl,-O,2 ";
			lnk << linkoptions;

			if (HasFlag("OSX11")) {
				if (HasFlag("POWERPC"))
					lnk << " -arch ppc";
				if (HasFlag("X86"))
					lnk << " -arch i386";
			}

			String lfilename;
			if(HasFlag("OBJC")) {
				String lfilename;
				String linklist;
				for(i = 0; i < linkfile.GetCount(); i++)
					if(ToLower(GetFileExt(linkfile[i])) == ".o" || ToLower(GetFileExt(linkfile[i])) == ".a")
						linklist << GetHostPath(linkfile[i]) << '\n';

				String linklistM = "Producing link file list ...\n";
				String odir = GetFileDirectory(linkfile[0]);
				lfilename << GetHostPath(GetFileFolder(linkfile[0])) << ".LinkFileList";
					
				linklistM  << lfilename;
				UPP::SaveFile(lfilename, linklist);
				lnk << " -L" << GetHostPathQ(odir)
				    << " -F" << GetHostPathQ(odir)
				          << " -filelist " << lfilename << " ";
				PutConsole( linklistM );
			}
			else
				for(i = 0; i < linkfile.GetCount(); i++) {
					if(ToLower(GetFileExt(linkfile[i])) == ".o")
						lnk  << ' ' << GetHostPathQ(linkfile[i]);
					else
						lib.Add(linkfile[i]);
				}

			if(!HasFlag("SOLARIS") && !HasFlag("OSX11") && !HasFlag("OBJC"))
				lnk << " -Wl,--start-group ";
			for(int pass = 0; pass < 2; pass++) {
				for(i = 0; i < lib.GetCount(); i++) {
					String ln = lib[i];
					String ext = ToLower(GetFileExt(ln));
					
					// unix shared libs shall have version number AFTER .so (sic)
					// so we shall find the true extension....
					if(HasFlag("POSIX") && ext != ".so")
					{
						const char *c = ln.Last();
						while(--c >= ~ln)
							if(!IsDigit(*c) && *c != '.')
								break;
						int pos = int(c - ~ln - 2);
						if(pos >= 0 && ToLower(ln.Mid(pos, 3)) == ".so")
							ext = ".so";
					}
							
					if(pass == 0) {
						if(ext == ".a")
							lnk << ' ' << GetHostPathQ(FindInDirs(libpath, lib[i]));
					}
					else
						if(ext != ".a") {
							if(ext == ".so" || ext == ".dll" || ext == ".lib")
								lnk << ' ' << GetHostPathQ(FindInDirs(libpath, lib[i]));
							else
								lnk << " -l" << ln;
						}
				}
				if(pass == 1 && !HasFlag("SOLARIS") && !HasFlag("OSX11"))
					lnk << " -Wl,--end-group";
			}
			PutConsole("Linking...");
			bool error = false;
			CustomStep(".pre-link", Null, error);
			if(!error && Execute(lnk) == 0) {
				CustomStep(".post-link", Null, error);
				PutConsole(String().Cat() << GetHostPath(target) << " (" << GetFileInfo(target).length
				           << " B) linked in " << GetPrintTime(time));
				return !error;
			}
			else {
				DeleteFile(target);
				return false;
			}
		}
	PutConsole(String().Cat() << GetHostPath(target) << " (" << GetFileInfo(target).length
	           << " B) is up to date.");
	return true;
}

bool GccBuilder::Preprocess(const String& package, const String& file, const String& target, bool asmout)
{
	Package pkg;
	String packagePath = PackagePath(package);
	pkg.Load(packagePath);
	String packageDir = GetFileFolder(packagePath);
	ChDir(packageDir);
	PutVerbose("cd " + packageDir);

	String cmd = CmdLine(package, pkg);
	cmd << " " << Gather(pkg.option, config.GetKeys());
	cmd << " -o " << target;
	cmd << (asmout ? " -S " : " -E ") << GetHostPathQ(file);
	if(BuilderUtils::IsCFile(file))
		cmd << " " << c_options;
	else
	if(BuilderUtils::IsCppFile(file))
		cmd << " " << cpp_options;
	return Execute(cmd);
}

Builder *CreateGccBuilder()
{
	return new GccBuilder;
}

INITIALIZER(GccBuilder)
{
	RegisterBuilder("GCC", CreateGccBuilder);
	RegisterBuilder("CLANG", CreateGccBuilder);
}
