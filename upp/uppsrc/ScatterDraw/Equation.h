#ifndef _ScatterDraw_Equation_h_
#define _ScatterDraw_Equation_h_

namespace Upp {


#define FormatCoeff(id, numDigits)		(IsNull(numDigits) ? (String("C") + FormatInt(id)) : FormatDoubleFix(coeff[id], numDigits))

class ExplicitEquation : public DataSource {
public:
	ExplicitEquation() {isExplicit = true;}

	virtual void SetDegree(int num) = 0;
	enum FitError {
		NoError = 1, 
		InadequateDataSource = -1,
		SmallDataSource = -2,
		ImproperInputParameters = -3,
		TooManyFunctionEvaluation = -4
	};
	virtual FitError Fit(DataSource &series, double &r2);
	FitError Fit(DataSource &series)		{double dummy; return Fit(series, dummy);}
	virtual void GuessCoeff(DataSource &series)	= 0;

	virtual double f(double x1) 			= 0;
	virtual double f(double x1, double x2) 	{NEVER();	return Null;}
	virtual double f(Vector <double> x) 	{NEVER();	return Null;}
	virtual String GetName() = 0;
	virtual String GetFullName()			{return GetName();}
	virtual String GetEquation(int numDigits = 3) = 0;
	virtual inline int64 GetCount()			{return Null;}
	
	void SetNumDigits(int n)				{numDigits = n;}
	int GetNumDigits()						{return numDigits;}
	void SetMaxFitFunctionEvaluations(int n){maxFitFunctionEvaluations = n;}
	int GetMaxFitFunctionEvaluations()		{return maxFitFunctionEvaluations;}
	
	friend struct Equation_functor;
	
	const Vector<double> &GetCoeff()			{return coeff;}

	template<class T>	
	static void Register(const String& name) {
		classMap().FindAdd(name, __Create<T>);
	}
	static void Unregister(const String& name) {
		int i = NameIndex(name);
		ASSERT(i >= 0);
		classMap().Remove(i);
	}
	static int            	 NameIndex(const String& name) {return classMap().Find(name);}
	static int            	 GetEquationCount()            {return classMap().GetCount();}
	static ExplicitEquation* Create(int i)                 {return classMap()[i]();}
	
	int GetNumCoeff(int num) {return coeff.GetCount();}
	
	ExplicitEquation &operator=(ExplicitEquation &other) {
		if (this != &other) {
			degree = other.degree;
			coeff = clone(other.coeff);
		}
		return *this;
	}
	double R2Y(DataSource &series, double mean = Null);
	
protected:
	Vector<double> coeff;
	int degree;
	static int numDigits, maxFitFunctionEvaluations;
	
	void SetNumCoeff(int num);
	void SetCoeff(const Vector<double>& c)          {coeff = clone(c);}
	void SetCoeff(double c0, double c1, double c2)	{coeff.Clear();	coeff << c0 << c1 << c2;}
	void SetCoeff(double c0, double c1) 			{coeff.Clear();	coeff << c0 << c1;}
	void SetCoeff(double c0) 						{coeff.Clear();	coeff << c0;}
	void SetCoeffVal(int id, double c) 				{coeff[id] = c;}
	
	typedef ExplicitEquation* (*CreateFunc)();
	template<class T>	
	static ExplicitEquation*                      __Create() {return new T;}
	static VectorMap<String, CreateFunc>& classMap() {static VectorMap<String, CreateFunc> cMap; return cMap;}
};

class AvgEquation : public ExplicitEquation {
public:
	AvgEquation() 						{SetCoeff(0);}
	AvgEquation(double c0)				{SetCoeff(c0);}
	double f(double x) 					{return coeff[0];}
	virtual String GetName() 			{return t_("Average");}
	virtual String GetEquation(int numDigits = 3) {	
		String ret = Format("%s", FormatCoeff(0, numDigits));
		return ret;
	}
	void SetDegree(int num)				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{coeff[0] = series.AvgY();}
};

class LinearEquation : public ExplicitEquation {
public:
	LinearEquation() 					{SetCoeff(0, 0);}
	LinearEquation(double c0, double c1){SetCoeff(c0, c1);}
	double f(double x) 					{
		return coeff[0] + x*coeff[1];
	}
	virtual String GetName() 			{return t_("Linear");}
	virtual String GetEquation(int numDigits = 3) {	
		String ret = Format("%s + %s*x", FormatCoeff(0, numDigits), FormatCoeff(1, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}
	void SetDegree(int num)				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{coeff[0] = series.AvgY();}
};

class PolynomialEquation : public ExplicitEquation {
public:
	PolynomialEquation() 				       	{}
	PolynomialEquation(const Vector<double>& c) {SetCoeff(c);}
	double f(double x);
	virtual String GetName() 			       	{return t_("Polynomial");}
	virtual String GetFullName() 		       	{return t_("Polynomial") + String(" n = ") + FormatInt(degree);}
	virtual String GetEquation(int numDigits = 3);
	void SetDegree(int num)				       	{degree = num;	SetNumCoeff(num + 1);}
	virtual void GuessCoeff(DataSource &series) {
		coeff[0] = series.AvgY();
		int realDegree = degree;
		for (degree = 2; degree < realDegree; degree++) 
			Fit(series);
	}
};	

class PolynomialEquation2 : public PolynomialEquation {
public:
	PolynomialEquation2() {SetDegree(2);}
};

class PolynomialEquation3 : public PolynomialEquation {
public:
	PolynomialEquation3() {SetDegree(3);}
};

class PolynomialEquation4 : public PolynomialEquation {
public:
	PolynomialEquation4() {SetDegree(4);}
};

class PolynomialEquation5 : public PolynomialEquation {
public:
	PolynomialEquation5() {SetDegree(5);}
};

class SinEquation : public ExplicitEquation {
public:
	SinEquation() 					{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1;}
	SinEquation(double offset, double A, double w, double phi) 	{Init(offset, A, w, phi);}
	void Init(double offset, double A, double w, double phi) 	{coeff.Clear();	coeff << offset << A << w << phi;}
	double f(double x)				{return coeff[0] + coeff[1]*sin(coeff[2]*x + coeff[3]);}
	virtual String GetName() 		{return t_("Sine");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s + %s*sin(%s*t + %s)", FormatCoeff(0, numDigits), FormatCoeff(1, numDigits)
													, FormatCoeff(2, numDigits), FormatCoeff(3, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int num)				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[1] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[2] = frequency;
			coeff[3] = phase;
		}
	}
};

class DampedSinEquation : public ExplicitEquation {
public:
	DampedSinEquation() 					{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1 << 0.1;}
	DampedSinEquation(double offset, double A, double lambda, double w, double phi) {Init(offset, A, lambda, w, phi);}
	void Init(double offset, double A, double lambda, double w, double phi) 	{coeff.Clear();	coeff << offset << A << lambda << w << phi;}
	double f(double x)				{return coeff[0] + coeff[1]*exp(-coeff[2]*x)*cos(coeff[3]*x + coeff[4]);}
	virtual String GetName() 		{return t_("DampedSinusoidal");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s + %s*e^(-%s*t)*cos(%s*t + %s)", FormatCoeff(0, numDigits), 
			FormatCoeff(1, numDigits), FormatCoeff(2, numDigits), FormatCoeff(3, numDigits), FormatCoeff(4, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int num)				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[2] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[3] = frequency;
			coeff[4] = phase;
		}
	}
};

class Sin_DampedSinEquation : public ExplicitEquation {
public:
	Sin_DampedSinEquation() 			{coeff.Clear();	coeff << 0. << 0.1 << 0.1 << 0.1 << 0.1 << 0.1 << 0.1 << 0.1;}
	Sin_DampedSinEquation(double offset, double A1, double w1, double phi1, double A2, 
			double lambda, double w2, double phi2) {Init(offset, A1, w1, phi1, A2, lambda, w2, phi2);}
	void Init(double offset, double A1, double w1, double phi1, double A2, double lambda, 
		double w2, double phi2) {coeff.Clear();	
								coeff << offset << A1 << w1 << phi1 << A2 << lambda << w2 << phi2;}
	double f(double x)				{return coeff[0] + coeff[1]*cos(coeff[2]*x + coeff[3]) + coeff[4]*exp(-coeff[5]*x)*cos(coeff[6]*x + coeff[7]);}
	virtual String GetName() 		{return t_("Sin_DampedSinusoidal");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s + %s*cos(%s*t + %s) + %s*e^(-%s*t)*cos(%s*t + %s)", 
			FormatCoeff(0, numDigits), FormatCoeff(1, numDigits), FormatCoeff(2, numDigits), 
			FormatCoeff(3, numDigits), FormatCoeff(4, numDigits), FormatCoeff(5, numDigits),
			FormatCoeff(6, numDigits), FormatCoeff(7, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}													
	void SetDegree(int num)				{NEVER();}
	virtual void GuessCoeff(DataSource &series)	{
		coeff[0] = series.AvgY();	
		coeff[1] = series.SinEstim_Amplitude(coeff[0]);
		double frequency, phase;
		if (series.SinEstim_FreqPhase(frequency, phase, coeff[0])) {
			coeff[2] = frequency;
			coeff[3] = phase;
		}
	}
};
	
class FourierEquation : public ExplicitEquation {
public:
	FourierEquation() 					        {}
	FourierEquation(const Vector<double>& c) 	{SetCoeff(c);}
	double f(double x);
	virtual String GetName() 			        {return t_("Fourier");}
	virtual String GetFullName() 		        {return t_("Fourier") + String(" n = ") + FormatInt(degree);}
	virtual String GetEquation(int numDigits = 3);
	virtual void GuessCoeff(DataSource &series) {coeff[0] = series.AvgY();}
	void SetDegree(int num)				        {degree = num;	SetNumCoeff(2*num + 2);}
};

class FourierEquation1 : public FourierEquation {
public:
	FourierEquation1() {SetDegree(1);}
};

class FourierEquation2 : public FourierEquation {
public:
	FourierEquation2() {SetDegree(2);}
};

class FourierEquation3 : public FourierEquation {
public:
	FourierEquation3() {SetDegree(3);}
};

class FourierEquation4 : public FourierEquation {
public:
	FourierEquation4() {SetDegree(4);}
};

class ExponentialEquation : public ExplicitEquation {
public:
	ExponentialEquation() 						{SetCoeff(1, 0);}
	ExponentialEquation(double c0, double c1)	{SetCoeff(c0, c1);}
	double f(double x) 							{return coeff[0]*exp(-x) + coeff[1];}
	virtual String GetName() 					{return t_("Exponential");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s*e^-x + %s", FormatCoeff(0, numDigits), FormatCoeff(1, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &series) {}
	void SetDegree(int num)				{NEVER();}
};

class RealExponentEquation : public ExplicitEquation {
public:
	RealExponentEquation() 						{SetCoeff(1, 1);}
	RealExponentEquation(double a, double b)	{SetCoeff(a, b);}
	double f(double x) 							{
		if (x < 0)
			return Null;
		return coeff[0]*pow(x, coeff[1]);
	}
	virtual String GetName() 					{return t_("RealExponent");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s*x^%s", FormatCoeff(0, numDigits), FormatCoeff(1, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}	
	virtual void GuessCoeff(DataSource &series) {}
	void SetDegree(int num)				{NEVER();}
};

class Rational1Equation : public ExplicitEquation {
public:
	Rational1Equation() 				{SetCoeff(1, 0, 0);}
	Rational1Equation(double c0, double c1, double c2)	{SetCoeff(c0, c1, c2);}
	double f(double x) 					{return coeff[0]/(x + coeff[1]) + coeff[2];}
	virtual String GetName() 			{return t_("Rational_1");}
	virtual String GetEquation(int numDigits = 3) {
		String ret = Format("%s/(x + %s) + %s", FormatCoeff(0, numDigits), FormatCoeff(1, numDigits), FormatCoeff(2, numDigits));
		ret.Replace("+ -", "- ");
		return ret;
	}			
	virtual void GuessCoeff(DataSource &series) {}
	void SetDegree(int num)				{NEVER();}
};

class SplineEquation : public ExplicitEquation {
public:
	SplineEquation() 					{}
	double f(double x);
	virtual String GetName() 			{return t_("Spline");}
	void SetDegree(int num)				{NEVER();}
	void GuessCoeff(DataSource &series)	{NEVER();}
	String GetEquation(int)				{return t_("Spline");}
	FitError Fit(DataSource &series, double &r2);
	FitError Fit(DataSource &series)	{double dummy; return Fit(series, dummy);}
		
private:
	struct Coeff{double a, b, c, d, x;};
    Buffer<Coeff> coeff;
    int ncoeff;
};


class EvalExpr {
public:
	EvalExpr();
	void Init()									  {variables.Clear();}
	double Eval(String line);
	String EvalStr(String line, int numDigits = 3);
	EvalExpr &SetCaseSensitivity(bool val = true) {noCase = !val;			return *this;}
	EvalExpr &SetErrorUndefined(bool val = true)  {errorIfUndefined = val;	return *this;}
	
	const String &GetFunction(int id) 						{return functions.GetKey(id);}
	int GetFunctionsCount() 								{return functions.GetCount();}
	
	void SetConstant(String name, double value = 0)			{constants.GetAdd(name) = value;}
	void SetConstant(int id, double value = 0)				{constants[id] = value;}
	double GetConstant(String name)							{return constants.Get(name, Null);}
	void GetConstant(int id, String &name, double &value)	{name = constants.GetKey(id); value = constants[id];}
	int GetConstantsCount() 								{return constants.GetCount();}

	void SetVariable(String name, double value = 0)			{variables.GetAdd(name) = value;}
	void SetVariable(int id, double value = 0)				{variables[id] = value;}
	double GetVariable(String name)							{return variables.GetAdd(name);}	
	void GetVariable(int id, String &name, double &value)	{name = variables.GetKey(id); value = variables[id];}
	int GetVariablesCount() 								{return variables.GetCount();}
	void ClearVariables();
	String &GetLastError()									{return lastError;}
	
	VectorMap<String, double> constants;
	VectorMap<String, double (*)(double)> functions;

	CParser p;

	static void EvalThrowError(CParser &p, const char *s);
	
protected:
	double Exp(CParser& p);
	
	bool noCase;
	String lastError;
	VectorMap<String, double> variables;
	bool errorIfUndefined;
	
	bool IsFunction(String str)								{return functions.Get(str, 0);}
	bool IsConstant(String str)								{return !IsNull(GetConstant(str));}
	
private:
	void *Functions_Get(CParser& p);
	double Term(CParser& p);
	double Pow(CParser& p);
	double Mul(CParser& p);
	
	String TermStr(CParser& p, int numDigits);
	String PowStr(CParser& p, int numDigits);
	String MulStr(CParser& p, int numDigits);
	String ExpStr(CParser& p, int numDigits);
};

class UserEquation : public ExplicitEquation {
public:
	UserEquation() {}
	UserEquation(String _name, String _strEquation, String varHoriz = "x")	{Init(_name, _strEquation, varHoriz);}
	void Init(String _name, String _strEquation, String varHoriz = "x") {
		name = _name;
		_strEquation.Replace(" ", "");
		StringStream str(_strEquation);
		Vector<String> parts = GetCsvLine(str, ';', CHARSET_DEFAULT);
		if (parts.IsEmpty())
			return;
		strEquation = parts[0];
		eval.Init();
		eval.SetConstant(varHoriz);
		idx = eval.GetConstantsCount() - 1;
		eval.EvalStr(strEquation);
		coeff.Clear();
		varNames.Clear();
		for (int i = 0; i < eval.GetVariablesCount(); ++i) {
			String varName;
			double dummy;
			eval.GetVariable(i, varName, dummy);
			varNames << varName;
			int istr;
			for (istr = 1; istr < parts.GetCount(); ++istr) {
				String strVar = varName + "=";
				int ifound = parts[istr].Find(strVar);
				if (ifound >= 0) {
					double val = ScanDouble(parts[istr].Mid(strVar.GetCount()));
					coeff << val;
					break;
				}
			}
			if (istr == parts.GetCount())
				coeff << 0.1;	
		}
	}
	double f(double x) {
		eval.SetConstant(idx, x);
		for (int i = 0; i < coeff.GetCount(); ++i) {
			eval.SetVariable(varNames[i], coeff[i]);
			double ret = eval.GetVariable(varNames[i]);
		}
		return eval.Eval(strEquation);
	}
	void SetName(String _name) 					    {name = _name;}
	virtual String GetName() 						{return name;}
	virtual String GetEquation(int numDigits = 3)	{return eval.EvalStr(strEquation, numDigits);}
	virtual void GuessCoeff(DataSource &series) 	{}
	void SetDegree(int num)							{NEVER();}

private:
	String name;
	String strEquation;
	Vector<String> varNames;
	EvalExpr eval;
	int idx;
};


}

#endif
