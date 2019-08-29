struct LinearPathConsumer {
	virtual void Move(const Pointf& p) = 0;
	virtual void Line(const Pointf& p) = 0;
	virtual void End();
};

void ApproximateQuadratic(LinearPathConsumer& t,
                          const Pointf& p1, const Pointf& p2, const Pointf& p3,
                          double tolerance);
void ApproximateCubic(LinearPathConsumer& t,
                      const Pointf& x0, const Pointf& x1, const Pointf& x2, const Pointf& x,
                      double tolerance);

struct LinearPathFilter : LinearPathConsumer {
	virtual void End();

	LinearPathConsumer *target;

	void PutMove(const Pointf& p)               { target->Move(p); }
	void PutLine(const Pointf& p)               { target->Line(p); }
	void PutEnd()                               { target->End(); }
};

class Stroker : public LinearPathFilter {
public:
	virtual void Move(const Pointf& p);
	virtual void Line(const Pointf& p);
	virtual void End();

private:
	double w2;
	double qmiter;
	double fid;
	double tw;

	Pointf p0, v0, o0, a0, b0;
	Pointf p1, v1, o1, a1, b1;
	Pointf p2;
	int    linecap;
	int    linejoin;
	Rectf  preclip;
	int    lines;
	
	void   Finish();
	void   Round(const Pointf& p, const Pointf& v1, const Pointf& v2, double r);
	void   Cap(const Pointf& p0, const Pointf& v0, const Pointf& o0,
	           const Pointf& a0, const Pointf& b0);
	bool   PreClipped(Pointf p2, Pointf p3);

public:	
	void Init(double width, double miterlimit, double tolerance, int linecap, int linejoin, const Rectf& preclip);
};

class Dasher : public LinearPathFilter {
public:
	virtual void Move(const Pointf& p);
	virtual void Line(const Pointf& p);

private:
	const Vector<double> *pattern;
	int            patterni;
	double         sum, rem;
	bool           flag;
	Pointf         p0;

	void    Put(const Pointf& p);

public:
	void Init(const Vector<double>& pattern, double distance);
};

struct Transformer : public LinearPathFilter {
public:
	virtual void Move(const Pointf& p);
	virtual void Line(const Pointf& p);

private:
	const Xform2D& xform;

public:
	Transformer(const Xform2D& xform) : xform(xform) {}
};

inline RGBA Mul8(const RGBA& s, int mul)
{
	RGBA t;
	t.r = (mul * s.r) >> 8;
	t.g = (mul * s.g) >> 8;
	t.b = (mul * s.b) >> 8;
	t.a = (mul * s.a) >> 8;
	return t;
}

inline void AlphaBlend(RGBA& t, const RGBA& c)
{
	int alpha = 256 - (c.a + (c.a >> 7));
	t.r = c.r + (alpha * t.r >> 8);
	t.g = c.g + (alpha * t.g >> 8);
	t.b = c.b + (alpha * t.b >> 8);
	t.a = c.a + ((256 - c.a) * t.a >> 8);
}

inline void AlphaBlendCover8(RGBA& t, const RGBA& c, int cover)
{
	int a = c.a * cover >> 8;
	int alpha = 256 - a - (a >> 7);
	t.r = (c.r * cover >> 8) + (alpha * t.r >> 8);
	t.g = (c.g * cover >> 8) + (alpha * t.g >> 8);
	t.b = (c.b * cover >> 8) + (alpha * t.b >> 8);
	t.a = a + (alpha * t.a >> 8);
}

class Rasterizer : public LinearPathConsumer {
public:
	virtual void Move(const Pointf& p);
	virtual void Line(const Pointf& p);

private:
	struct Cell : Moveable<Cell> {
		int16 x;
		int16 cover;
		int   area;

		bool operator<(const Cell& b) const { return x < b.x; }
    };
	struct CellArray {
		int  count;
		int  alloc;
	};

	Rectf                   cliprect;
	Pointf                  p0;
	Buffer<CellArray *>     cell;

	int                     min_y;
	int                     max_y;
	Size                    sz;
	int                     mx;

	static CellArray       *AllocArray(int n);

	void  Init();
	Cell *AddCells(int y, int n);
	void  RenderHLine(int ey, int x1, int y1, int x2, int y2);
	void  LineClip(double x1, double y1, double x2, double y2);
	int   CvX(double x);
	int   CvY(double y);
	void  CvLine(double x1, double y1, double x2, double y2);
	bool  BeginRender(int y, const Cell *&c, const Cell *&e);
	void  Free();

	static int Q8Y(double y) { return int(y * 256 + 0.5); }
	int Q8X(double x)        { return int(x * mx + 0.5); }

public:
	struct Filler {
		virtual void Start(int x, int len) = 0;
		virtual void Render(int val) = 0;
		virtual void Render(int val, int len) = 0;
		virtual void End();
	};

	void LineRaw(int x1, int y1, int x2, int y2);
	
	void  SetClip(const Rectf& rect);
	Rectf GetClip() const                     { return cliprect; }

	int  MinY() const                         { return min_y; }
	int  MaxY() const                         { return max_y; }
	void Render(int y, Filler& g, bool evenodd);

	void Reset();

	void Create(int cx, int cy, bool subpixel);
	
	Rasterizer(int cx, int cy, bool subpixel);
	Rasterizer() { sz = Size(0, 0); }
	~Rasterizer();
};

struct SpanSource {
	virtual void Get(RGBA *span, int x, int y, unsigned len) = 0;
	virtual ~SpanSource() {}
};

class ClippingLine : NoCopy {
	byte *data;
	
public:
	void Clear()                     { if(!IsFull()) delete[] data; data = NULL; }
	void Set(const byte *s, int len) { data = new byte[len]; memcpy(data, s, len); }
	void SetFull()                   { ASSERT(!data); data = (byte *)1; }

	bool IsEmpty() const             { return !data; }
	bool IsFull() const              { return data == (byte *)1; }
	operator const byte*() const     { return data; }
	
	ClippingLine()                       { data = NULL; }
	~ClippingLine()                      { Clear(); }
};

class LinearInterpolator {
	struct Dda2 {
		int count, lift, rem, mod, p;
		
		void  Set(int a, int b, int len);
		int   Get();
	};

	Xform2D xform;
	Dda2    ddax, dday;

	static int Q8(double x) { return int(256 * x + 0.5); }
	
public:
	void   Set(const Xform2D& m)                    { xform = m; }

	void   Begin(int x, int y, int len);
	Point  Get();
};

struct PainterTarget : LinearPathConsumer {
	virtual void Fill(double width, SpanSource *ss, const RGBA& color);
};

class BufferPainter : public Painter {
protected:
	virtual void   ClearOp(const RGBA& color);

	virtual void   MoveOp(const Pointf& p, bool rel);
	virtual void   LineOp(const Pointf& p, bool rel);
	virtual void   QuadraticOp(const Pointf& p1, const Pointf& p, bool rel);
	virtual void   QuadraticOp(const Pointf& p, bool rel);
	virtual void   CubicOp(const Pointf& p1, const Pointf& p2, const Pointf& p, bool rel);
	virtual void   CubicOp(const Pointf& p2, const Pointf& p, bool rel);
	virtual void   ArcOp(const Pointf& c, const Pointf& r, double angle, double sweep, bool rel);
	virtual void   SvgArcOp(const Pointf& r, double xangle, bool large, bool sweep,
	                        const Pointf& p, bool rel);
	virtual void   CloseOp();
	virtual void   DivOp();

	virtual void   CharacterOp(const Pointf& p, int ch, Font fnt);

	virtual void   FillOp(const RGBA& color);
	virtual void   FillOp(const Image& image, const Xform2D& transsrc, dword flags);
	virtual void   FillOp(const Pointf& p1, const RGBA& color1,
	                      const Pointf& p2, const RGBA& color2,
	                      int style);
	virtual void   FillOp(const RGBA& color1, const RGBA& color2, const Xform2D& transsrc,
	                      int style);
	virtual void   FillOp(const Pointf& f, const RGBA& color1,
	                      const Pointf& c, double r, const RGBA& color2,
	                      int style);
	virtual void   FillOp(const Pointf& f, const RGBA& color1, const RGBA& color2,
	                      const Xform2D& transsrc, int style);

	virtual void   StrokeOp(double width, const RGBA& rgba);
	virtual void   StrokeOp(double width, const Image& image, const Xform2D& transsrc,
	                        dword flags);
	virtual void   StrokeOp(double width, const Pointf& p1, const RGBA& color1,
	                        const Pointf& p2, const RGBA& color2,
	                        int style);
	virtual void   StrokeOp(double width, const RGBA& color1, const RGBA& color2,
	                        const Xform2D& transsrc,
	                        int style);
	virtual void   StrokeOp(double width, const Pointf& f, const RGBA& color1,
	                        const Pointf& c, double r, const RGBA& color2,
	                        int style);
	virtual void   StrokeOp(double width, const Pointf& f,
	                        const RGBA& color1, const RGBA& color2,
	                        const Xform2D& transsrc, int style);

	virtual void   ClipOp();

	virtual void   ColorStopOp(double pos, const RGBA& color);
	virtual void   ClearStopsOp();
	
	virtual void   OpacityOp(double o);
	virtual void   LineCapOp(int linecap);
	virtual void   LineJoinOp(int linejoin);
	virtual void   MiterLimitOp(double l);
	virtual void   EvenOddOp(bool evenodd);
	virtual void   DashOp(const Vector<double>& dash, double start);
	virtual void   InvertOp(bool invert);

	virtual void   TransformOp(const Xform2D& m);

	virtual void   BeginOp();
	virtual void   EndOp();

	virtual void   BeginMaskOp();
	virtual void   BeginOnPathOp(double q, bool abs);

private:
	enum {
		MOVE, LINE, QUADRATIC, CUBIC, CHAR
	};
	struct LinearData {
		Pointf p;
	};
	struct QuadraticData : LinearData {
		Pointf p1;
	};
	struct CubicData : QuadraticData {
		Pointf p2;
	};
	struct CharData : LinearData {
		int  ch;
		int  _filler;
		Font fnt;
	};
	struct PathLine : Moveable<PathLine> {
		Pointf p;
		double len;
	};
	struct Attr : Moveable<Attr> {
		Xform2D                         mtx;
		bool                            evenodd;
		byte                            join;
		byte                            cap;
		double                          miter_limit;
		WithDeepCopy< Vector<double> >  dash;
		WithDeepCopy< Vector<double> >  stop;
		WithDeepCopy< Vector<RGBA> >    stop_color;
		double                          dash_start;
		double                          opacity;
		bool                            invert;

		int                             cliplevel;
		bool                            hasclip;
		bool                            mask;
		bool                            onpath;
	};
	
	PainterTarget             *alt = NULL;
	double                     alt_tolerance = Null;
	ImageBuffer                dummy;
	ImageBuffer&               ib;
	int                        mode;
	Buffer<int16>              subpixel;
	int                        render_cx;
	int                        dopreclip;

	Attr                       attr;
	Array<Attr>                attrstack;
	Vector< Buffer<ClippingLine> > clip;
	Array< ImageBuffer >       mask;
	Vector< Vector<PathLine> > onpathstack;
	Vector<double>             pathlenstack;
	
	Image                      gradient;
	RGBA                       gradient1, gradient2;
	int                        gradientn;

	Vector<String> path;
	bool           ischar;
	Pointf         path_min, path_max;
	Attr           pathattr;

	Pointf       current, ccontrol, qcontrol, move;
	
	Rasterizer       rasterizer;
	Buffer<RGBA>     span;

	Vector<PathLine> onpath;
	double           pathlen;
	
	struct OnPathTarget;
	friend struct OnPathTarget;
	
	bool co = false;

	struct OnPathTarget : LinearPathConsumer {
		Vector<BufferPainter::PathLine> path;
		Pointf pos;
		double len;
		
		virtual void Move(const Pointf& p) {
			BufferPainter::PathLine& t = path.Add();
			t.len = 0;
			pos = t.p = p;
		}
		virtual void Line(const Pointf& p) {
			BufferPainter::PathLine& t = path.Add();
			len += (t.len = Distance(pos, p));
			pos = t.p = p;
		}
		
		OnPathTarget() { len = 0; pos = Pointf(0, 0); }
	};
	
	struct PathJob {
		Transformer   trans;
		Stroker       stroker;
		Dasher        dasher;
		OnPathTarget  onpathtarget;
		bool          evenodd;
		bool          regular;
		double        tolerance;
		bool          preclipped;

		LinearPathConsumer *g;

		PathJob(Rasterizer& r, double width, bool ischar, int dopreclip, Pointf path_min, Pointf path_max, const Attr& attr);
	};
	
	struct CoJob {
		Attr         attr;
		String       path;
		double       width;
		RGBA         color;
		bool         ischar;
		Rasterizer   rasterizer;
		Pointf       path_min, path_max;
		bool         evenodd;
		RGBA         c;
		void DoPath(const BufferPainter& sw);
		
		CoJob() {}
	};
	
	friend struct CoJob;
	
	Array<CoJob> cojob;
	int          jobcount;
	
	void         PathAddRaw(int type, const void *data, int size);
	template <class T> void PathAdd(int type, const T& data) { return PathAddRaw(type, &data, sizeof(T)); }

	Pointf           PathPoint(const Pointf& p, bool rel);
	Pointf           EndPoint(const Pointf& p, bool rel);
	void             DoMove0();
	void             ClearPath();
	static void      ApproximateChar(LinearPathConsumer& t, const CharData& ch, double tolerance);
	Buffer<ClippingLine> RenderPath(double width, Event<One<SpanSource>&> ss, const RGBA& color);
	void             RenderImage(double width, const Image& image, const Xform2D& transsrc,
	                             dword flags);
	void             RenderRadial(double width, const Pointf& f, const RGBA& color1,
	                              const Pointf& c, double r, const RGBA& color2,
	                              const Xform2D& m, int style);
	void             RenderRadial(double width, const Pointf& f, const RGBA& color1, const RGBA& color2,
	                              const Xform2D& transsrc, int style);
	void             MakeGradient(RGBA color1, RGBA color2, int cx);
	void             Gradient(const RGBA& color1, const RGBA& color2, const Pointf& p1, const Pointf& p2);
	void             ColorStop0(Attr& a, double pos, const RGBA& color);
	void             FinishMask();

	static void RenderPathSegments(LinearPathConsumer *g, const String& path,
	                               const Attr *attr, double tolerance);

	enum { FILL = -1, CLIP = -2, ONPATH = -3 };

public:
	ImageBuffer&       GetBuffer()                             { return ib; }
	const ImageBuffer& GetBuffer() const                       { return ib; }

	void               Finish();
	
	BufferPainter&     PreClip(bool b = true)                  { dopreclip = b; return *this; }
	BufferPainter&     PreClipDashed()                         { dopreclip = 2; return *this; }
	BufferPainter&     Co(bool b = true)                       { co = b; return *this; }

	BufferPainter(ImageBuffer& ib, int mode = MODE_ANTIALIASED);
	BufferPainter(PainterTarget& t, double tolerance = Null);
	
	~BufferPainter()                                           { Finish(); }
};

#include "Interpolator.hpp"
