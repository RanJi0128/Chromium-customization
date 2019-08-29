#include "ScatterDraw.h"

#define TFILE <ScatterDraw/ScatterDraw.t>
#include <Core/t.h>

ScatterDraw& ScatterDraw::SetColor(const Color& _color) {
	graphColor = _color;
	return *this;
}

ScatterDraw& ScatterDraw::SetTitle(const String& _title) {
	title = _title;
	return *this;
}

const String& ScatterDraw::GetTitle() {
	return title;	
}

ScatterDraw& ScatterDraw::SetTitleFont(const Font& fontTitle) {
	titleFont = fontTitle;
	return *this;
}

ScatterDraw& ScatterDraw::SetTitleColor(const Color& colorTitle) {
	titleColor = colorTitle;
	return *this;
}

void ScatterDraw::SetLabels(const String& _xLabel, const String& _yLabel, const String& _yLabel2) {
	xLabel_base = _xLabel;
	yLabel_base = _yLabel;
	yLabel2_base = _yLabel2;
	labelsChanged = true;	
}

ScatterDraw& ScatterDraw::SetLabelX(const String& _xLabel) {
	xLabel_base = _xLabel;
	labelsChanged = true;
	return *this;
}

ScatterDraw& ScatterDraw::SetLabelY(const String& _yLabel) {
	yLabel_base = _yLabel;
	labelsChanged = true;
	return *this;
}

ScatterDraw& ScatterDraw::SetLabelY2(const String& _yLabel) {
	yLabel2_base = _yLabel;
	labelsChanged = true;
	return *this;
}

ScatterDraw& ScatterDraw::SetLabelsFont(const Font& fontLabels) {
	labelsFont = fontLabels;
	return *this;
}

ScatterDraw& ScatterDraw::SetLabelsColor(const Color& colorLabels) {
	labelsColor = colorLabels;
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaMargin(int hLeft, int hRight, int vTop, int vBottom) {
	hPlotLeft   = hLeft;	
	hPlotRight  = hRight;
	vPlotTop    = vTop;
	vPlotBottom = vBottom;
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaLeftMargin(int margin) {	
	hPlotLeft = margin;	
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaRightMargin(int margin) {	
	hPlotRight = margin;	
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaTopMargin(int margin) {	
	vPlotTop = margin;	
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaBottomMargin(int margin) {	
	vPlotBottom = margin;	
	return *this;
}

ScatterDraw& ScatterDraw::SetPlotAreaColor(const Color& p_a_color) {
	plotAreaColor = p_a_color;        
	return *this;
}

ScatterDraw& ScatterDraw::SetAxisColor(const Color& axis_color) {
	axisColor = axis_color;
	return *this;
}

ScatterDraw& ScatterDraw::SetAxisWidth(int axis_width) {
	axisWidth = axis_width;
	return *this;
}

ScatterDraw& ScatterDraw::SetGridColor(const Color& grid_color) {
	gridColor = grid_color;
	return *this;
}

ScatterDraw& ScatterDraw::SetGridWidth(int grid_width) {
	gridWidth = grid_width;
	return *this;
}

ScatterDraw& ScatterDraw::ShowHGrid(bool show) {
	drawHGrid = show;
	return *this;
}

ScatterDraw& ScatterDraw::ShowVGrid(bool show) {
	drawVGrid = show;
	return *this;
}

ScatterDraw &ScatterDraw::SetDrawXReticle(bool set) {
	drawXReticle = set;
	return *this;
}

ScatterDraw &ScatterDraw::SetDrawYReticle(bool set) {
	drawYReticle = set;
	return *this;
}

ScatterDraw &ScatterDraw::SetDrawY2Reticle(bool set) {
	drawY2Reticle = set;
	return *this;
}

bool ScatterDraw::PointInPlot(Point &pt) 
{
	return 	hPlotLeft <= pt.x && 				pt.x <= (GetSize().cx - hPlotRight) && 
		  	(vPlotTop + titleHeight) <= pt.y && pt.y <= (GetSize().cy - vPlotBottom);
}

bool ScatterDraw::PointInBorder(Point &pt) 
{
	return !PointInPlot(pt);
}

bool ScatterDraw::PointInLegend(Point &pt) 
{
	return false;
}

void ScatterDraw::DrawLegend(Draw& w, const Size &size, int scale) const {
	if (series.IsEmpty())
		return;
	Upp::Font scaledFont = GetStdFont();
	int rowHeight = scale*GetStdFont().GetHeight();
	int rowAscent = scale*GetStdFont().GetAscent();
	scaledFont.Height(rowHeight);
	int xWidth = scaledFont.GetWidth('X');
	int lineLen = 4*xWidth;
	
	Vector<String> legends;
	int legendWidth = 0;
	for (int i = 0; i < series.GetCount(); ++i) {
		String legend = series[i].legend;
		if (!series[i].unitsY.IsEmpty())
			legend += " [" + series[i].unitsY + "]";
		legends.Add(legend);
		legendWidth = max<int>(legendWidth, GetTextSize(legend, scaledFont).cx);
	}
	legendWidth += lineLen + 4*xWidth;
	
	int rowIncSign;
	int plotW, plotH;
	int nlr;					
	int topClip;
	int nlab = series.GetCount();		
	int plotLeft, plotTop, rectWidth, rectHeight;	
	int loclegendRowSpacing;
	if (legendAnchor == LEGEND_TOP) {
		plotLeft = plotTop = 0;
		plotW = (size.cx - hPlotLeft - hPlotRight)*scale;
		plotH = scale*(vPlotTop - 1) + titleHeight;		
		rowIncSign = -1;
		rectWidth = plotW;
		rectHeight = plotH;
		topClip = 0;
		nlr = fround(rectWidth/legendWidth);	
		loclegendRowSpacing = 0;
	} else {
		plotW = (size.cx - hPlotLeft)*scale;
		plotH = (size.cy - vPlotTop - vPlotBottom)*scale - titleHeight;
		rowIncSign = 1;
		if (IsNull(legendPos))
			return;
		
		plotLeft = scale*hPlotLeft;
		plotTop = scale*vPlotTop + titleHeight;
		rectWidth = legendWidth*legendNumCols;
		topClip = scale*vPlotTop + titleHeight;
		nlr = legendNumCols;	
		loclegendRowSpacing = legendRowSpacing*scale;
	}
	if (nlr <= 0) 
		return;
	
	int nrows = fceil(double(nlab)/nlr);	

	if (legendAnchor != LEGEND_TOP) 
		rectHeight = int(rowHeight*(nrows + 0.2)) + loclegendRowSpacing*nrows;
	
	int left = plotLeft + legendPos.x*scale;
	int right = plotLeft + plotW - (hPlotRight + legendPos.x)*scale - rectWidth;
	int top = plotTop + legendPos.y*scale;
	int bottom = plotTop + plotH - legendPos.y*scale - rectHeight;
	Rect rect;
	switch(legendAnchor) {
	case LEGEND_TOP:				rect.Set(scale*hPlotLeft, 0, rectWidth, rectHeight); 			break;
	case LEGEND_ANCHOR_LEFT_TOP: 	rect.Set(left, top, left + rectWidth, top + rectHeight);		break;
	case LEGEND_ANCHOR_RIGHT_TOP:	rect.Set(right, top, right + rectWidth, top + rectHeight);		break;
	case LEGEND_ANCHOR_LEFT_BOTTOM: rect.Set(left, bottom, left + rectWidth, bottom + rectHeight);	break; 
	case LEGEND_ANCHOR_RIGHT_BOTTOM:rect.Set(right, bottom, right + rectWidth, bottom + rectHeight);break; 
	}
	
	w.Clip(scale*hPlotLeft, topClip, plotW, plotH);
	
	if (legendAnchor != LEGEND_TOP) {
		if (!IsNull(legendFillColor))
			w.DrawRect(rect, legendFillColor);
		if (!IsNull(legendBorderColor)) {
			w.DrawLine(rect.left, rect.top, rect.right, rect.top, 1, legendBorderColor);
			w.DrawLine(rect.right, rect.top, rect.right, rect.bottom, 1, legendBorderColor);
			w.DrawLine(rect.right, rect.bottom, rect.left, rect.bottom, 1, legendBorderColor);
			w.DrawLine(rect.left, rect.bottom, rect.left, rect.top, 1, legendBorderColor);
		}
	}
	Upp::Font italic = scaledFont;
	italic.Italic();
	scaledFont.Bold();
	for(int row = 0, start = 0; row <= nrows; row++) {
		for(int i = start; i < min(start + nlr, nlab); i++) {
			double lx = rect.left + (i - start)*legendWidth + xWidth;
			double ly = (rowIncSign >= 0 ? rect.top : rect.bottom) +
					 rowIncSign*int(rowHeight*(row + 0.6) + loclegendRowSpacing*(row + 0.5));
			Vector <Pointf> line;
			line << Pointf(lx, ly) << Pointf(lx + lineLen, ly);
			if (series[i].opacity > 0 && series[i].seriesPlot)
				DrawPolylineOpa(w, line, scale, 1, series[i].thickness, series[i].color, series[i].dash);
			Pointf mark_p(lx + xWidth/*scale*7*/, ly);
			if (series[i].markWidth >= 1 && series[i].markPlot)
				series[i].markPlot->Paint(w, scale, mark_p, series[i].markWidth, series[i].markColor, 
					series[i].markBorderWidth, series[i].markBorderColor);   
			Upp::Font &font = series[i].primaryY ? scaledFont : italic;
			DrawText(w, lx + lineLen + xWidth, ly - int((2*rowAscent)/3)/*scale*6*/, 0, legends[i], font, series[i].color);                   
		}
		start += nlr;
	}
	w.End();
}	

void ScatterDraw::AdjustMinUnitX() {
	if (xMajorUnit > 0) {
		if (xMinUnit < 0)
			xMinUnit += (fabs(ceil(xMinUnit/xMajorUnit)) + 1)*xMajorUnit;
		else if (xMinUnit >= xMajorUnit)
			xMinUnit -= (fabs(floor(xMinUnit/xMajorUnit)))*xMajorUnit;
	}
}

void ScatterDraw::AdjustMinUnitY() {
	if (yMajorUnit > 0) {
		if (yMinUnit < 0)
			yMinUnit += (fabs(ceil(yMinUnit/yMajorUnit)) + 1)*yMajorUnit;
		else if (yMinUnit >= yMajorUnit)
			yMinUnit -= (fabs(floor(yMinUnit/yMajorUnit)))*yMajorUnit;
	}
}

void ScatterDraw::AdjustMinUnitY2() {
	if (yMajorUnit2 > 0) {
		if (yMinUnit2 < 0)
			yMinUnit2 += (fabs(ceil(yMinUnit2/yMajorUnit2)) + 1)*yMajorUnit2;
		else if (yMinUnit2 >= yMajorUnit2)
			yMinUnit2 -= (fabs(floor(yMinUnit2/yMajorUnit2)))*yMajorUnit2;
	}
}

ScatterDraw &ScatterDraw::SetRange(double rx, double ry, double ry2) {
	ASSERT(IsNull(rx) || rx > 0);
	ASSERT(IsNull(ry) || ry > 0);
	ASSERT(IsNull(ry2) || ry2 > 0);
	
	if (!IsNull(rx)) {
		xRange = rx;
		xMajorUnit = xRange/10;
		AdjustMinUnitX();
	}
	if (!IsNull(ry)) {
		yRange = ry;
		yMajorUnit = yRange/10;   
		AdjustMinUnitY();
	}
	if (!IsNull(ry2)) {
		yRange2 = ry2;
		yMajorUnit2 = yRange2/10;   
		AdjustMinUnitY2();
	}
	WhenSetRange();
	return *this;
}

ScatterDraw &ScatterDraw::SetMajorUnits(double ux, double uy) {
	if (!IsNull(ux)) {
		xMajorUnit = ux;
		AdjustMinUnitX();
	}
	if (!IsNull(uy)) {
		yMajorUnit = uy;
		yMajorUnit2 = yMajorUnit*yRange2/yRange;
		AdjustMinUnitY();
		AdjustMinUnitY2();
	}
	return *this;
}

ScatterDraw &ScatterDraw::SetMajorUnitsNum(int nx, int ny) {
	if (!IsNull(nx)) {
		xMajorUnit = xRange/nx;
		AdjustMinUnitX();
	}
	if (!IsNull(ny)) {
		yMajorUnit = yRange/ny;
		yMajorUnit2 = yMajorUnit*yRange2/yRange;
		AdjustMinUnitY();
		AdjustMinUnitY2();
	}
	return *this;
}

ScatterDraw &ScatterDraw::SetMinUnits(double ux, double uy) {
	if (!IsNull(ux))
		xMinUnit = ux;
	if (!IsNull(uy)) {	
		yMinUnit = uy;
		yMinUnit2 = yRange2*yMinUnit/yRange;
	}
	if (!IsNull(ux))
		AdjustMinUnitX();
	if (!IsNull(uy)) {
		AdjustMinUnitY();
		AdjustMinUnitY2();	
	}
	return *this;
}

ScatterDraw &ScatterDraw::SetXYMin(double xmin, double ymin, double ymin2) {
	if (!IsNull(xmin))
		xMin = xmin;
	if (!IsNull(ymin))
		yMin = ymin;
	if (!IsNull(ymin2))
		yMin2 = ymin2;
	WhenSetXYMin();
	return *this;
}

void ScatterDraw::ZoomToFit(bool horizontal, bool vertical, double factor) {
	ZoomToFit(horizontal, Null, Null, vertical, Null, Null, vertical, Null, Null, factor);
}

void ScatterDraw::ZoomToFit(bool horizontal, double minx, double maxx, bool vertical, double miny, 
		double maxy, bool vertical2, double miny2, double maxy2, double factor) {
	if (linkedMaster) {
		linkedMaster->ZoomToFit(horizontal, vertical);
		return;
	}
	DoFitToData(horizontal, minx, maxx, vertical, miny, maxy, vertical2, miny2, maxy2, factor);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i)
	    	linkedCtrls[i]->DoFitToData(horizontal, minx, maxx, vertical, miny, maxy, vertical2, miny2, maxy2, factor);
	}
}
		
void ScatterDraw::DoFitToData(bool horizontal, double minx, double maxx, bool vertical, double miny, 
		double maxy, bool vertical2, double miny2, double maxy2, double factor) {
	bool calcminx, calcmaxx, calcminy, calcminy2, calcmaxy, calcmaxy2;
	if (IsNull(minx)) {
		minx = -DOUBLE_NULL;
		calcminx = true;
	} else
		calcminx = false;
	if (IsNull(miny)) {
		miny = -DOUBLE_NULL;
		calcminy = true;
	} else
		calcminy = false;
	if (IsNull(miny2)) {
		miny2 = -DOUBLE_NULL;
		calcminy2 = true;
	} else
		calcminy2 = false;
	if (IsNull(maxx)) {
		maxx = DOUBLE_NULL;
		calcmaxx = true;
	} else
		calcmaxx = false;
	if (IsNull(maxy)) {
		maxy = DOUBLE_NULL;
		calcmaxy = true;
	} else
		calcmaxy = false;
	if (IsNull(maxy2)) {
		maxy2 = DOUBLE_NULL;
		calcmaxy2 = true;
	} else
		calcmaxy2 = false;
	
	try {
		if (horizontal) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				if (calcminx) {
					double mn = series[j].PointsData()->MinX();
					if (!IsNull(mn))
						minx = min(minx, mn);
				}
				if (calcmaxx) {
					double mx = series[j].PointsData()->MaxX();
					if (!IsNull(mx))
						maxx = max(maxx, mx);
				}
			}
			if (maxx != DOUBLE_NULL && minx != -DOUBLE_NULL) {
				if (maxx < minx)
					Swap(maxx, minx);
				double deltaX = (maxx - minx)*factor;
				if (calcminx)
					minx -= deltaX;
				if (calcmaxx)
					maxx += deltaX;
			}
		}
		if (vertical) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				if (series[j].primaryY) {
					if (calcminy) {
						double mn = series[j].PointsData()->MinY();
						if (!IsNull(mn))
							miny = min(miny, mn);
					}
					if (calcmaxx) {
						double mx = series[j].PointsData()->MaxY();
						if (!IsNull(mx))
							maxy = max(maxy, mx);
					}
				}
			}
			if (maxy != DOUBLE_NULL && miny != -DOUBLE_NULL) {
				if (maxy < miny)
					Swap(maxy, miny);
				double deltaY = (maxy - miny)*factor;
				if (calcminy)
					miny -= deltaY;
				if (calcmaxy)
					maxy += deltaY;		
			}
		}
		if (vertical2) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				if (!series[j].primaryY) {
					if (calcminy2) {
						double mn = series[j].PointsData()->MinY();
						if (!IsNull(mn))
							miny2 = min(miny2, mn);
					}
					if (calcmaxy2) {
						double mx = series[j].PointsData()->MaxY();
						if (!IsNull(mx))
							maxy2 = max(maxy2, mx);
					}
				}
			}
			if (maxy2 != DOUBLE_NULL && miny2 != -DOUBLE_NULL) {
				if (maxy2 < miny2)
					Swap(maxy2, miny2);
				double deltaY2 = (maxy2 - miny2)*factor;
				if (calcminy2)
					miny2 -= deltaY2;
				if (calcmaxy2)
					maxy2 += deltaY2;		
			}
		}
		if (horizontal) {
			if (maxx != DOUBLE_NULL && minx != -DOUBLE_NULL) {
				if (maxx == minx) {
					if (maxx == 0) {
						xRange = 2;
						xMin = -1;
					} else	
						xRange = 2*maxx;
				} else	
					xRange = maxx - minx;
				double deltaX = xMin - minx;
				xMin -= deltaX;
				xMinUnit += deltaX;
				xMajorUnit = xRange/10;
				AdjustMinUnitX();
			}
		}
		if (vertical) {
			if (maxy != DOUBLE_NULL && miny != -DOUBLE_NULL) {
				if (maxy == miny) 
					yRange = maxy > 0 ? 2*maxy : 1;
				else	
					yRange = maxy - miny;
				double deltaY = yMin - miny;
				yMin -= deltaY;
				yMinUnit += deltaY;
				yMajorUnit = yRange/10; 
				AdjustMinUnitY();
			}
		}
		if (vertical2) {
			if (maxy2 != DOUBLE_NULL && miny2 != -DOUBLE_NULL) {	
				if (maxy2 == miny2) 
					yRange2 = maxy2 > 0 ? 2*maxy2 : 1;
				else	
					yRange2 = maxy2 - miny2;
				double deltaY2 = yMin2 - miny2;
				yMin2 -= deltaY2;
				yMinUnit2 += deltaY2;
				yMajorUnit2 = yRange2/10; 
				AdjustMinUnitY2();
			}
		}	
	} catch (ValueTypeError err) {
		ASSERT_(true, err);
		return;
	}
	WhenSetRange();
	WhenSetXYMin();
	Refresh();
}

void ScatterDraw::ZoomToFitSmart(bool horizontal, double minx, double maxx, bool vertical, double miny, 
		double maxy, bool vertical2, double miny2, double maxy2, double factor) {
	if (linkedMaster) {
		linkedMaster->ZoomToFit(horizontal, vertical);
		return;
	}
	DoFitToDataSmart(horizontal, minx, maxx, vertical, miny, maxy, vertical2, miny2, maxy2, factor);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i)
	    	linkedCtrls[i]->DoFitToDataSmart(horizontal, minx, maxx, vertical, miny, maxy, vertical2, miny2, maxy2, factor);
	}
}
		
void ScatterDraw::DoFitToDataSmart(bool horizontal, double minx, double maxx, bool vertical, double miny, 
		double maxy, bool vertical2, double miny2, double maxy2, double factor) {
	bool calcminx, calcmaxx, calcminy, calcminy2, calcmaxy, calcmaxy2;
	if (IsNull(minx)) {
		minx = -DOUBLE_NULL;
		calcminx = true;
	} else
		calcminx = false;
	if (IsNull(miny)) {
		miny = -DOUBLE_NULL;
		calcminy = true;
	} else
		calcminy = false;
	if (IsNull(miny2)) {
		miny2 = -DOUBLE_NULL;
		calcminy2 = true;
	} else
		calcminy2 = false;
	if (IsNull(maxx)) {
		maxx = DOUBLE_NULL;
		calcmaxx = true;
	} else
		calcmaxx = false;
	if (IsNull(maxy)) {
		maxy = DOUBLE_NULL;
		calcmaxy = true;
	} else
		calcmaxy = false;
	if (IsNull(maxy2)) {
		maxy2 = DOUBLE_NULL;
		calcmaxy2 = true;
	} else
		calcmaxy2 = false;
	
	try {
		if (horizontal) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				double av = series[j].PointsData()->AvgX();
				if (calcminx) {
					if (!IsNull(av))
						minx = min(minx, av);
				}
				if (calcmaxx) {
					if (!IsNull(av))
						maxx = max(maxx, av);
				}
			}
			if (maxx != DOUBLE_NULL && minx != -DOUBLE_NULL) {
				if (maxx < minx)
					Swap(maxx, minx);
				double deltaX = (maxx - minx)*factor;
				if (calcminx)
					minx -= deltaX;
				if (calcmaxx)
					maxx += deltaX;
			}
		}
		if (vertical) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				if (series[j].primaryY) {
					double av = series[j].PointsData()->AvgY();
					if (calcminy) {
						if (!IsNull(av))
							miny = min(miny, av);
					}
					if (calcmaxx) {
						if (!IsNull(av))
							maxy = max(maxy, av);
					}
				}
			}
			if (maxy != DOUBLE_NULL && miny != -DOUBLE_NULL) {
				if (maxy < miny)
					Swap(maxy, miny);
				double deltaY = (maxy - miny)*factor;
				if (calcminy)
					miny -= deltaY;
				if (calcmaxy)
					maxy += deltaY;		
			}
		}
		if (vertical2) {
			for (int j = 0; j < series.GetCount(); j++) {
				if (series[j].opacity == 0 || series[j].PointsData()->IsExplicit())
					continue;
				if (!series[j].primaryY) {
					double av = series[j].PointsData()->AvgY();
					if (calcminy2) {
						if (!IsNull(av))
							miny2 = min(miny2, av);
					}
					if (calcmaxy2) {
						if (!IsNull(av))
							maxy2 = max(maxy2, av);
					}
				}
			}
			if (maxy2 != DOUBLE_NULL && miny2 != -DOUBLE_NULL) {
				if (maxy2 < miny2)
					Swap(maxy2, miny2);
				double deltaY2 = (maxy2 - miny2)*factor;
				if (calcminy2)
					miny2 -= deltaY2;
				if (calcmaxy2)
					maxy2 += deltaY2;		
			}
		}
		if (horizontal) {
			if (maxx != DOUBLE_NULL && minx != -DOUBLE_NULL) {
				if (maxx == minx) {
					if (maxx == 0) {
						xRange = 2;
						xMin = -1;
					} else	
						xRange = 2*maxx;
				} else	
					xRange = maxx - minx;
				double deltaX = xMin - minx;
				xMin -= deltaX;
				xMinUnit += deltaX;
				xMajorUnit = xRange/10;
				AdjustMinUnitX();
			}
		}
		if (vertical) {
			if (maxy != DOUBLE_NULL  && miny != -DOUBLE_NULL) {
				if (maxy == miny) 
					yRange = maxy > 0 ? 2*maxy : 1;
				else	
					yRange = maxy - miny;
				double deltaY = yMin - miny;
				yMin -= deltaY;
				yMinUnit += deltaY;
				yMajorUnit = yRange/10; 
				AdjustMinUnitY();
			}
		}
		if (vertical2) {
			if (maxy2 != DOUBLE_NULL && miny2 != -DOUBLE_NULL) {	
				if (maxy2 == miny2) 
					yRange2 = maxy2 > 0 ? 2*maxy2 : 1;
				else	
					yRange2 = maxy2 - miny2;
				double deltaY2 = yMin2 - miny2;
				yMin2 -= deltaY2;
				yMinUnit2 += deltaY2;
				yMajorUnit2 = yRange2/10; 
				AdjustMinUnitY2();
			}
		}	
	} catch (ValueTypeError err) {
		ASSERT_(true, err);
		return;
	}
	WhenSetRange();
	WhenSetXYMin();
	Refresh();
}

ScatterDraw &ScatterDraw::Graduation_FormatX(Formats fi) {
	switch (fi) {
		case EXP: cbModifFormatX = THISBACK(ExpFormat); break;
		case MON: cbModifFormatX = THISBACK(MonFormat);	break;
		case DY:  cbModifFormatX = THISBACK(DyFormat);	break;
		default:break;
	}
	return *this;
}

ScatterDraw &ScatterDraw::Graduation_FormatY(Formats fi) {
	switch (fi) {
		case EXP: cbModifFormatY = THISBACK(ExpFormat);	break;
		case MON: cbModifFormatY = THISBACK(MonFormat);	break;
		case DY:  cbModifFormatY = THISBACK(DyFormat);	break;
		default:break;
	}
	return *this;
}

ScatterDraw &ScatterDraw::Graduation_FormatY2(Formats fi) {
	switch (fi) {
		case EXP: cbModifFormatY2 = THISBACK(ExpFormat);	break;
		case MON: cbModifFormatY2 = THISBACK(MonFormat);	break;
		case DY:  cbModifFormatY2 = THISBACK(DyFormat);		break;
		default:break;
	}
	return *this;
}

String ScatterDraw::VariableFormat(double range, double d) {
	if (fabs(d) <= 1e-15)
		d = 0;
	if 		(0.001 <= range && range < 0.01)   return FormatDouble(d, 5);
	else if (0.01  <= range && range < 0.1)    return FormatDouble(d, 4);
	else if (0.1   <= range && range < 1) 	   return FormatDouble(d, 3);
	else if (1	   <= range && range < 10) 	   return FormatDouble(d, 2);
	else if (10	   <= range && range < 100)    return FormatDouble(d, 1);
	else if (100   <= range && range < 10000000) {
						/*if (d < 1 && d > -1)   return "0";	// Never -0
						else*/				   return FormatDouble(d, 0);
	} else return FormatDoubleExp(d, 2);
}

Color ScatterDraw::GetNewColor(int index, int version) {
	Color old[20] = {LtBlue(), LtRed(), LtGreen(), Black(), LtGray(), Brown(), Blue(), Red(), Green(), Gray(), 
					 LtBlue(), LtRed(), LtGreen(), Black(), LtGray(), Brown(), Blue(), Red(), Green(), Gray()};
	// Colors from http://tools.medialab.sciences-po.fr/iwanthue/
	Color nwc[20] = {Color(197,127,117), Color(115,214,74), Color(205,80,212), Color(124,193,215), Color(85,82,139),
					 Color(63,72,41), Color(109,212,161), Color(207,72,48), Color(209,206,59), Color(194,134,55),
					 Color(201,63,109), Color(193,192,158), Color(91,134,56), Color(105,48,38), Color(201,170,200),
					 Color(86,117,119), Color(188,91,165), Color(124,120,216), Color(195,208,119), Color(79,46,75)};
	if (index < 20) {
		if (version == 0) 
			return old[index];
		else
			return nwc[index];
	} else
		return Color(Random(), Random(), Random());
}
	
String ScatterDraw::GetNewDash(int index) {
	switch(index) {
	case 0:		return LINE_SOLID;
	case 1:		return LINE_DOTTED;
	case 2:		return LINE_DASHED;
	case 3:		return LINE_DASH_DOT;
	case 4:		return LINE_SOLID;
	case 5:		return LINE_DOTTED;
	case 6:		return LINE_DASHED;
	case 7:		return LINE_DASH_DOT;
	case 8:		return LINE_SOLID;
	case 9:		return LINE_DOTTED;
	case 10:	return LINE_DASHED;
	case 11:	return LINE_DASH_DOT;
	}
	dword r = Random();
	if (r < 8000)
		r += 2000;
	String ret = FormatInt(r).Right(4);
	String space = " ";
	return ret.Mid(0, 1) + space + ret.Mid(1, 1) + space + ret.Mid(2, 1) + space + ret.Mid(3, 1);
}

MarkPlot *ScatterDraw::GetNewMarkPlot(int index) {
	switch(index) {
	case 0:	return new CircleMarkPlot();
	case 1:	return new SquareMarkPlot();
	case 2:	return new TriangleMarkPlot();
	case 3:	return new CrossMarkPlot();
	case 4:	return new XMarkPlot();
	case 5:	return new RhombMarkPlot();
	}
	return new CircleMarkPlot();
}

Color GetOpaqueColor(const Color &color, const Color &background, double opacity) {
	if (opacity == 1)
		return color;
	if (opacity == 0)
		return background;
	return Color(int(opacity*(color.GetR() - background.GetR()) + background.GetR()),
	             int(opacity*(color.GetG() - background.GetG()) + background.GetG()),
	             int(opacity*(color.GetB() - background.GetB()) + background.GetB()));
} 
		
ScatterDraw::ScatterBasicSeries::ScatterBasicSeries() { 
	color = Null;
	thickness = 3;
	legend = "";
	unitsX = "";
	unitsY = "";
	opacity = 1;
	primaryY = true;
	sequential = false;
	dash = LINE_SOLID;	
	seriesPlot = new LineSeriesPlot();
	markPlot = new CircleMarkPlot();
	markWidth = 8;
	markColor = Null;
	markBorderWidth = 1;
	markBorderColor = Null;
	fillColor = Null;
	barWidth = 10;
	isClosed = false;
}

void ScatterDraw::ScatterBasicSeries::Init(int index) {
	color = GetNewColor(index);
	markColor = Color(max(color.GetR()-30, 0), max(color.GetG()-30, 0), max(color.GetB()-30, 0));
	
	dash = GetNewDash(int(index/6));
	markPlot = GetNewMarkPlot(index);
}

ScatterDraw &ScatterDraw::AddSeries(DataSource &data) {
	ScatterSeries &s = series.Add();
	s.Init(series.GetCount()-1);
	s.SetDataSource(&data, false);
	Refresh();
	return *this;	
}

DataSource &ScatterDraw::GetSeries(int index) {
	return series[index].GetDataSource();
}

ScatterDraw &ScatterDraw::_AddSeries(DataSource *data) {
	ScatterSeries &s = series.Add();
	s.Init(series.GetCount()-1);
	s.SetDataSource(data, true);
	Refresh();
	return *this;	
}

ScatterDraw &ScatterDraw::InsertSeries(int index, double *yData, int numData, double x0, double deltaX) {
	return InsertSeries<CArray>(index, yData, numData, x0, deltaX);
}
	
ScatterDraw &ScatterDraw::InsertSeries(int index, double *xData, double *yData, int numData) {
	return InsertSeries<CArray>(index, xData, yData, numData);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, Vector<double> &xData, Vector<double> &yData) {
	return InsertSeries<VectorDouble>(index, xData, yData);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, Upp::Array<double> &xData, Upp::Array<double> &yData) {
	return InsertSeries<ArrayDouble>(index, xData, yData);
}
		
ScatterDraw &ScatterDraw::InsertSeries(int index, Vector<Pointf> &points) {
	return InsertSeries<VectorPointf>(index, points);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, Upp::Array<Pointf> &points) {
	return InsertSeries<ArrayPointf>(index, points);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, double (*function)(double))	 {
	return InsertSeries<FuncSource>(index, function);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, Pointf (*function)(double), int np, double from, double to)	 {
	return InsertSeries<FuncSourcePara>(index, function, np, from, to);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, PlotExplicFunc &function)	 {									
	return InsertSeries<PlotExplicFuncSource>(index, function);
}

ScatterDraw &ScatterDraw::InsertSeries(int index, PlotParamFunc function, int np, double from, double to)	 {
	return InsertSeries<PlotParamFuncSource>(index, function, np, from, to);
}

ScatterDraw &ScatterDraw::_InsertSeries(int index, DataSource *data) {
	ASSERT(IsValid(index));
	ScatterSeries &s = series.Insert(index);
	s.Init(index);
	s.SetDataSource(data, true);
	Refresh();	
	return *this;
}

int64 ScatterDraw::GetCount(int index) {
	ASSERT(IsValid(index));
	return series[index].PointsData()->GetCount();
}

void ScatterDraw::GetValues(int index, int64 idata, double &x, double &y) {
	ASSERT(IsValid(index) && !IsNull(GetCount(index)));
	ASSERT(idata >= 0 && idata < series[index].PointsData()->GetCount());
	try {
		x = series[index].PointsData()->x(idata);
		y = series[index].PointsData()->y(idata);
	} catch(ValueTypeError error) {
		ASSERT_(true, error);
		x = y = Null;
	}
}

double ScatterDraw::GetValueX(int index, int64 idata) {
	ASSERT(IsValid(index) && !IsNull(GetCount(index)));
	ASSERT(idata >= 0 && idata < series[index].PointsData()->GetCount());
	try {
		return series[index].PointsData()->x(idata);
	} catch(ValueTypeError error) {
		ASSERT_(true, error);
		return Null;
	}
}

Value ScatterDraw::GetStringX(int index, int64 idata) {
	double ret = GetValueX(index, idata);
	if (IsNull(ret))
		return Null;
	if (cbModifFormatX) {
		String sret;
		cbModifFormatX(sret, int(idata), ret);
		return sret;
	} else
		return ret;
}
	

double ScatterDraw::GetValueY(int index, int64 idata) {
	ASSERT(IsValid(index) && !IsNull(GetCount(index)));
	ASSERT(idata >= 0 && idata < series[index].PointsData()->GetCount());
	try {
		return series[index].PointsData()->y(idata);
	} catch(ValueTypeError error) {
		ASSERT_(true, error);
		return Null;
	}
}

Value ScatterDraw::GetStringY(int index, int64 idata) {
	double ret = GetValueY(index, idata);
	if (IsNull(ret))
		return Null;
	String sret;
	if (cbModifFormatY) {
		String sret;
		cbModifFormatY(sret, int(idata), ret);
		return sret;
	} else
		return ret;
}
	
ScatterDraw &ScatterDraw::SetNoPlot(int index) {
 	ASSERT(IsValid(index));
 	
 	series[index].seriesPlot = NULL;
 	return *this;
}	
	
	
ScatterDraw &ScatterDraw::PlotStyle(int index, SeriesPlot *data) {
	ASSERT(IsValid(index));
	
	series[index].seriesPlot = data;
	return *this;
}

ScatterDraw &ScatterDraw::PlotStyle(int index, const String name) {
	ASSERT(IsValid(index));
	
	int typeidx = SeriesPlot::TypeIndex(name);
	
	if (typeidx >= 0)
		series[index].seriesPlot = SeriesPlot::Create(typeidx);
	else
		series[index].seriesPlot = 0;
	return *this;
}

const String ScatterDraw::GetPlotStyleName(int index) {
	ASSERT(IsValid(index));
	
	if (series[index].seriesPlot)
		return SeriesPlot::TypeName(series[index].seriesPlot->GetType());
	else
		return t_("No series");
}

ScatterDraw &ScatterDraw::MarkStyle(int index, MarkPlot *data) {
	ASSERT(IsValid(index));
	
	series[index].markPlot = data;
	return *this;
}

ScatterDraw &ScatterDraw::MarkStyle(int index, const String name) {
	int typeidx = MarkPlot::TypeIndex(name);
	return MarkStyle(index, typeidx);
}

ScatterDraw &ScatterDraw::MarkStyle(int index, int typeidx) {
	ASSERT(IsValid(index));
	
	if (typeidx >= 0)
		series[index].markPlot = MarkPlot::Create(typeidx);
	else
		series[index].markPlot = 0;
	return *this;
}

const String ScatterDraw::GetMarkStyleName(int index) {
	ASSERT(IsValid(index));
	
	if (series[index].markPlot)
		return MarkPlot::TypeName(series[index].markPlot->GetType());
	else
		return t_("No mark");
}

int ScatterDraw::GetMarkStyleType(int index) {
	ASSERT(IsValid(index));
	
	if (series[index].markPlot)
		return series[index].markPlot->GetTypeType();
	else
		return -1;
}

/*ScatterDraw &ScatterDraw::SetMarkStyleType(int index, int type) {
	ASSERT(IsValid(index));
	
	if (series[index].markPlot)
		series[index].markPlot->SetTypeType(type);
	
	return *this;	
}*/

ScatterDraw &ScatterDraw::Stroke(int index, double thickness, Color color) {
	ASSERT(IsValid(index));
	
	series[index].thickness = thickness;
	if (IsNull(color))
		color = GetNewColor(index);
	series[index].color = color;
	//series[index].dash = GetNewDash(index);
	
	Refresh();
	return *this;		
}

void ScatterDraw::GetStroke(int index, double &thickness, Color &color) {
	ASSERT(IsValid(index));
	
	color = series[index].color;
	thickness = series[index].thickness;
}

ScatterDraw &ScatterDraw::Closed(int index, bool closed) {
	ASSERT(IsValid(index));
	
	series[index].isClosed = closed;
	
	Refresh();
	return *this;
}

bool ScatterDraw::IsClosed(int index) {
	ASSERT(IsValid(index));
	
	return series[index].isClosed;
}

ScatterDraw &ScatterDraw::BarWidth(int index, double width) {
	ASSERT(IsValid(index));
	
	series[index].barWidth = width;
	
	Refresh();
	return *this;
}

double ScatterDraw::GetBarWidth(int index) {
	ASSERT(IsValid(index));
	
	return series[index].barWidth;
}

ScatterDraw &ScatterDraw::Fill(Color color) {
	int index = series.GetCount() - 1;

	if (IsNull(color)) {
		color = GetNewColor(index);
		color = Color(min(color.GetR()+30, 255), min(color.GetG()+30, 255), min(color.GetB()+30, 255));
	}
	series[index].fillColor = color;
	
	Refresh();
	return *this;	
}

ScatterDraw &ScatterDraw::MarkColor(Color color) {
	int index = series.GetCount() - 1;

	if (IsNull(color)) {
		color = GetNewColor(index);
		color = Color(max(color.GetR()-30, 0), max(color.GetG()-30, 0), max(color.GetB()-30, 0));
	}
	series[index].markColor = color;
	
	Refresh();
	return *this;	
}

ScatterDraw &ScatterDraw::MarkBorderColor(Color color) {
	int index = series.GetCount() - 1;

	if (IsNull(color)) {
		color = GetNewColor(index + 1);
		color = Color(max(color.GetR()-30, 0), max(color.GetG()-30, 0), max(color.GetB()-30, 0));
	}
	series[index].markBorderColor = color;
	
	Refresh();
	return *this;	
}

ScatterDraw &ScatterDraw::MarkWidth(double markWidth) {
	int index = series.GetCount() - 1;
	
	series[index].markWidth = markWidth;
	
	Refresh();
	return *this;
}

ScatterDraw &ScatterDraw::MarkBorderWidth(double markWidth) {
	int index = series.GetCount() - 1;
	
	series[index].markBorderWidth = markWidth;
	
	Refresh();
	return *this;
}

ScatterDraw &ScatterDraw::Dash(const char *dash) {
	int index = series.GetCount() - 1;
	
	return Dash(index, dash);		
}

ScatterDraw &ScatterDraw::Dash(int index, const char *dash) {
	ASSERT(IsValid(index));
	
	series[index].dash = dash;
	Refresh();
	return *this;		
}

const String ScatterDraw::GetDash(int index) {
	ASSERT(IsValid(index));
	
	return series[index].dash;
}

ScatterDraw &ScatterDraw::Legend(const String legend) {
	int index = series.GetCount() - 1;
	
	return Legend(index, legend);
}

ScatterDraw& ScatterDraw::Legend(int index, const String legend) {
	ASSERT(IsValid(index));
	
	series[index].legend = legend;
	return *this;
}

const String& ScatterDraw::GetLegend(int index) {
	ASSERT(IsValid(index));
	return series[index].legend;
}

ScatterDraw &ScatterDraw::Units(const String unitsY, const String unitsX) {
	int index = series.GetCount() - 1;
	
	return Units(index, unitsY, unitsX);
}

ScatterDraw& ScatterDraw::Units(int index, const String unitsY, const String unitsX) {
	ASSERT(IsValid(index));
	
	series[index].unitsX = unitsX;
	series[index].unitsY = unitsY;
	labelsChanged = true;
	
	return *this;
}

const String ScatterDraw::GetUnitsX(int index) {
	ASSERT(IsValid(index));
	return series[index].unitsX;
}

const String ScatterDraw::GetUnitsY(int index) {
	ASSERT(IsValid(index));
	return series[index].unitsY;
}

/*
ScatterDraw& ScatterDraw::SetDataColor(int index, const Color& color) {
	ASSERT(IsValid(index));
	series[index].color = color;
	Refresh();
	return *this;
}

Color ScatterDraw::GetDataColor(int index) const {
	ASSERT(IsValid(index));
	return series[index].color;
}

ScatterDraw& ScatterDraw::SetDataThickness(int index, double thickness) {
	ASSERT(IsValid(index));
	series[index].thickness = thickness;
	Refresh();
	return *this;
}

double ScatterDraw::GetDataThickness(int index) const {
	ASSERT(IsValid(index));
	return series[index].thickness;
}*/

ScatterDraw& ScatterDraw::SetFillColor(int index, const Color& color) {
	ASSERT(IsValid(index));
	series[index].fillColor = color;
	Refresh();
	return *this;
}

Color ScatterDraw::GetFillColor(int index) const {
	ASSERT(IsValid(index));
	return series[index].fillColor;
}

ScatterDraw &ScatterDraw::SetMarkBorderWidth(int index, double width) { 
	ASSERT(IsValid(index));
	series[index].markBorderWidth = width;
	Refresh();
	return *this;
}
 
double ScatterDraw::GetMarkBorderWidth(int index) { 
 	ASSERT(IsValid(index));
 	return series[index].markBorderWidth;
}

ScatterDraw &ScatterDraw::SetMarkWidth(int index, double markWidth) {
	ASSERT(IsValid(index));
	series[index].markWidth = markWidth;
	Refresh();
	return *this;
}

double ScatterDraw::GetMarkWidth(int index) {
	ASSERT(IsValid(index));
	return series[index].markWidth;
}

ScatterDraw &ScatterDraw::SetMarkColor(int index, const Color& color) {
	ASSERT(IsValid(index));
	series[index].markColor = color;
	Refresh();
	return *this;
}

Color ScatterDraw::GetMarkColor(int index) const {
	ASSERT(IsValid(index));
	return series[index].markColor;
}

void ScatterDraw::SetMarkBorderColor(int index, const Color& color) {
	ASSERT(IsValid(index));
	series[index].markBorderColor = color;
	Refresh();
}

Color ScatterDraw::GetMarkBorderColor(int index) const {
	ASSERT(IsValid(index));
	return series[index].markBorderColor;
}

void ScatterDraw::NoMark(int index) {
	ASSERT(IsValid(index));
	series[index].markWidth = 0;
}

bool ScatterDraw::IsShowMark(int index) {
	ASSERT(IsValid(index));
	return series[index].markWidth > 0;
}


void ScatterDraw::SetDataPrimaryY(int index, bool primary) {
	ASSERT(IsValid(index));
	series[index].primaryY = primary;
	Refresh();
}

ScatterDraw &ScatterDraw::SetDataPrimaryY(bool primary) {
	SetDataPrimaryY(series.GetCount()-1, primary);
	return *this;
}

bool ScatterDraw::IsDataPrimaryY(int index) {
	ASSERT(IsValid(index));
	return series[index].primaryY;	
}

void ScatterDraw::SetSequentialX(int index, bool sequential) {
	ASSERT(IsValid(index));
	series[index].sequential = sequential;
	Refresh();
}

ScatterDraw &ScatterDraw::SetSequentialX(bool sequential) {
	SetSequentialX(series.GetCount()-1, sequential);
	return *this;
}

bool ScatterDraw::GetSequentialX(int index) {
	ASSERT(IsValid(index));
	return series[index].sequential;
}

bool ScatterDraw::GetSequentialX() {
	return GetSequentialX(series.GetCount()-1);
}

ScatterDraw &ScatterDraw::SetSequentialXAll(bool sequential) {
	for (int i = 0; i < series.GetCount(); ++i)
		SetSequentialX(i, sequential);
	sequentialXAll = sequential;
	return *this;
}

void ScatterDraw::Show(int index, bool show) {
	ASSERT(IsValid(index));
	series[index].opacity = show ? 1 : 0;
	Refresh();
}

bool ScatterDraw::IsVisible(int index) {
	ASSERT(IsValid(index));
	return series[index].opacity > 0;
}

ScatterDraw &ScatterDraw::ShowAll(bool show) {
	for (int i = 0; i < series.GetCount(); ++i)
		series[i].opacity = 1;
	return *this;
}

ScatterDraw& ScatterDraw::Id(int id) {
	return Id(series.GetCount()-1, id);
}

ScatterDraw& ScatterDraw::Id(int index, int id) {
	ASSERT(IsValid(index));
	series[index].id = id;
	return *this;
}

int ScatterDraw::GetId(int index) {
	ASSERT(IsValid(index));
	return series[index].id;
}

void ScatterDraw::RemoveSeries(int index) {
	ASSERT(IsValid(index));
	series.Remove(index);
	Refresh();
}

void ScatterDraw::RemoveAllSeries() {
	series.Clear();
	Refresh();
}

Drawing ScatterDraw::GetDrawing(const Size _size, int scale) {
	Size size = IsNull(_size) ? GetSize() : _size;
	DrawingDraw ddw(scale*size);
	
	SetDrawing(ddw, size, scale, true);
	PlotTexts(ddw, size, scale);

	return ddw;
}

Image ScatterDraw::GetImage(const Size &size, int scale) {
#ifndef flagGUI
	ASSERT(mode != MD_DRAW);
#endif
	ImageBuffer ib(scale*size);	
	BufferPainter bp(ib, mode);	
	
	bp.LineCap(LINECAP_BUTT);
	bp.LineJoin(LINEJOIN_MITER);
	SetDrawing(bp, size, scale, false);

	return ib;
}

Image ScatterDraw::GetImage(int scale) {
	return GetImage(GetSize(), scale);
}

double ScatterDraw::GetXByPoint(double x) {
	return (x - hPlotLeft)*GetXRange()/(GetSize().cx - (hPlotLeft + hPlotRight) - 1) + GetXMin();		
}

double ScatterDraw::GetYByPoint(double y) {
	return (GetSize().cy - vPlotTop - y - 1)*GetYRange()/(GetSize().cy - (vPlotTop + vPlotBottom) - GetTitleFont().GetHeight() - 1) + GetYMin();
}

double ScatterDraw::GetY2ByPoint(double y) {
	return (GetSize().cy - vPlotTop - y - 1)*GetY2Range()/(GetSize().cy - (vPlotTop + vPlotBottom) - GetTitleFont().GetHeight() - 1) + GetYMin2();
}

double ScatterDraw::GetXPointByValue(double x) {
	return (x - GetXMin())/GetXRange()*(GetSize().cx - (hPlotLeft + hPlotRight) - 1) + hPlotLeft;
}

double ScatterDraw::GetYPointByValue(double y) {
	return (GetSize().cy - vPlotTop - 1) - (y - GetYMin())/GetYRange()*(GetSize().cy - (vPlotTop + vPlotBottom) - GetTitleFont().GetHeight() - 1);
}


ScatterDraw &ScatterDraw::SetRangeLinked(double rx, double ry, double ry2) {
	double actual_rx = GetXRange();
	double actual_ry = GetYRange();
	double actual_ry2 = GetY2Range();
	SetRangeLinkedEach(rx, actual_rx, ry, actual_ry, ry2, actual_ry2);
	return *this;
}

void ScatterDraw::SetRangeLinkedEach(double rx, double rx0, double ry, double ry0, double ry2, double ry20) {	
	if (linkedMaster) {
		linkedMaster->SetRangeLinkedEach(rx, rx0, ry, ry0, ry2, ry20);
		linkedMaster->Refresh();
	}
	double actual_rx = GetXRange();
	double actual_ry = GetYRange();
	double actual_ry2 = GetY2Range();
	SetRange(actual_rx == rx0   ? rx  : Null, 
			 actual_ry == ry0   ? ry  : Null, 
			 actual_ry2 == ry20 ? ry2 : Null);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i) {
			double linked_rx = linkedCtrls[i]->GetXRange();
			double linked_ry = linkedCtrls[i]->GetYRange();
			double linked_ry2 = linkedCtrls[i]->GetY2Range();
	
	    	linkedCtrls[i]->SetRange(rx0 == linked_rx   ? rx  : Null, 
									 ry0 == linked_ry   ? ry  : Null, 
									 ry20 == linked_ry2 ? ry2 : Null);
	    	linkedCtrls[i]->Refresh();
		}
	}
}

ScatterDraw &ScatterDraw::SetXYMinLinked(double xmin, double ymin, double ymin2) {
	double actual_xmin = GetXMin();
	double actual_ymin = GetYMin();
	double actual_ymin2 = GetY2Min();
	SetXYMinLinkedEach(xmin, actual_xmin, ymin, actual_ymin, ymin2, actual_ymin2);
	return *this;
}

void ScatterDraw::SetXYMinLinkedEach(double xmin, double xmin0, double ymin, double ymin0, double ymin2, double ymin20) {
	if (linkedMaster) {
		linkedMaster->SetXYMinLinkedEach(xmin, xmin0, ymin, ymin0, ymin2, ymin20);
		linkedMaster->Refresh();
	}
	double actual_xmin = GetXMin();
	double actual_ymin = GetYMin();
	double actual_ymin2 = GetY2Min();
	SetXYMin(actual_xmin == xmin0   ? xmin  : Null, 
			 actual_ymin == ymin0   ? ymin  : Null, 
			 actual_ymin2 == ymin20 ? ymin2 : Null);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i) {
			double linked_xmin = linkedCtrls[i]->GetXMin();
			double linked_ymin = linkedCtrls[i]->GetYMin();
			double linked_ymin2 = linkedCtrls[i]->GetY2Min();
		
	    	linkedCtrls[i]->SetXYMin(xmin0 == linked_xmin   ? xmin  : Null, 
									 ymin0 == linked_ymin   ? ymin  : Null, 
									 ymin20 == linked_ymin2 ? ymin2 : Null);
	    	linkedCtrls[i]->Refresh();
		}
	}
}

ScatterDraw &ScatterDraw::SetMouseHandling(bool valx, bool valy) 
{
	mouseHandlingX = valx;
	mouseHandlingY = valy;
	return *this;
}

ScatterDraw &ScatterDraw::SetMouseHandlingLinked(bool valx, bool valy) {
	if (linkedMaster) {
		linkedMaster->SetMouseHandlingLinked(valx, valy);
		linkedMaster->Refresh();
		return *this;
	}
	SetMouseHandling(valx, valy);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i) {
	    	linkedCtrls[i]->SetMouseHandling(valx, valy);
	    	linkedCtrls[i]->Refresh();
		}
	}
	return *this;
}

void ScatterDraw::Zoom(double scale, bool mouseX, bool mouseY) {
	if (linkedMaster) {
		linkedMaster->Zoom(scale, mouseX, mouseY);
		return;
	}
	DoZoom(scale, mouseX, mouseY);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i) 
	    	linkedCtrls[i]->DoZoom(scale, mouseX, mouseY);
	}
}

void ScatterDraw::DoZoom(double scale, bool mouseX, bool mouseY) {
	if (scale == 1)
		return;
	lastRefresh_sign = (scale >= 0) ? 1 : -1;
	
	if (scale > 1) {
		if (maxXRange > 0) {
			if (xRange*scale > maxXRange) {
				highlight_0 = GetTickCount();
				if (xRange == maxXRange) {
					Refresh();
					return;
				} else 
					scale = maxXRange/xRange;
			} 
		}
		if (maxYRange > 0) {
			if (yRange*scale > maxYRange) {
				highlight_0 = GetTickCount();
				if (yRange == maxYRange) {
					Refresh();
					return;
				} else 
					scale = maxYRange/xRange;
			} 
		}
	} else {
		if (maxXRange > 0) {
			if (xRange*scale < minXRange) {
				highlight_0 = GetTickCount();
				if (xRange == minXRange) {
					Refresh();
					return;
				} else 
					scale = minXRange/xRange;
			} 
		}
		if (maxYRange > 0) {
			if (yRange*scale < minYRange) {
				highlight_0 = GetTickCount();
				if (yRange == minYRange) {
					Refresh();
					return;
				} else 
					scale = minYRange/xRange;
			} 
		}
	}
	//double plotW = scale*(GetSize().cx - (hPlotLeft + hPlotRight));
	//double plotH = scale*(GetSize().cy - (vPlotTop + vPlotBottom)) - titleHeight;
	//double dw = plotW*xMajorUnit/double(xRange);
	//double dh = plotH*yMajorUnit/double(yRange);
	
	bool can = true;//min<double>(dw, dh) > 20 || scale < 1;
	if (mouseX && can) {
		if (zoomStyleX == TO_CENTER) {
			if (!IsNull(minXmin) && xMin + xRange*(1-scale)/2. <= minXmin) {
				highlight_0 = GetTickCount();
				Refresh();
				return;
			}
			if (!IsNull(maxXmax) && xMin + xRange*scale + xRange*(1-scale)/2. >= maxXmax) {
				highlight_0 = GetTickCount();
				Refresh();
				return;
			}
			double oldXMin = xMin;
			xMin += xRange*(1-scale)/2.;
			xMinUnit = oldXMin + xMinUnit - xMin;
			AdjustMinUnitX();
		}
		xRange *= scale;
		if (!IsNull(maxMajorUnitsX)) {
			if (xRange < 2*xMajorUnit)
				xMajorUnit /= 5;
			else if (xRange/xMajorUnit > maxMajorUnitsX)
				xMajorUnit *= 5;
			AdjustMinUnitX();
		}
		lastxRange = xRange;
	}
	if (mouseY && can) {
		if (zoomStyleY == TO_CENTER) {
			if (!IsNull(minYmin) && yMin + yRange*(1-scale)/2. <= minYmin) {
				highlight_0 = GetTickCount();
				Refresh();
				return;
			}
			if (!IsNull(maxYmax) && yMin + yRange*scale + yRange*(1-scale)/2. >= maxYmax) {
				highlight_0 = GetTickCount();
				Refresh();
				return;
			}
			double oldYMin = yMin;
			yMin += yRange*(1 - scale)/2.;
			yMinUnit = oldYMin + yMinUnit - yMin;
			AdjustMinUnitY();
			double oldYMin2 = yMin2;
			yMin2 += yRange2*(1-scale)/2.;
			yMinUnit2 = oldYMin2 + yMinUnit2 - yMin2;
			AdjustMinUnitY2();
		}
		yRange *= scale;
		yRange2 *= scale;
		if (!IsNull(maxMajorUnitsY)) {
			if (yRange < 2*yMajorUnit) {
				yMajorUnit /= 5;
				yMajorUnit2 /= 5;
			} else if (yRange/yMajorUnit > maxMajorUnitsY) {
				yMajorUnit *= 5;
				yMajorUnit2 *= 5;
			}
			AdjustMinUnitY();
		}	
		lastyRange = yRange;	
	}
	if ((mouseX || mouseY) && can) {
		WhenSetRange();
		if (zoomStyleX == TO_CENTER || zoomStyleY == TO_CENTER)
			WhenSetXYMin();
		Refresh();
		WhenZoomScroll();
	}
}

void ScatterDraw::Scroll(double factorX, double factorY) {
	if (linkedMaster) {
		linkedMaster->Scroll(factorX, factorY);
		return;
	}
	DoScroll(factorX, factorY);
	if (!linkedCtrls.IsEmpty()) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i)
	    	linkedCtrls[i]->DoScroll(factorX, factorY);
	}
}

void ScatterDraw::DoScroll(double factorX, double factorY) {
	if (factorX != 0) {
		double deltaX = factorX*xRange;
		if (!IsNull(minXmin) && factorX > 0) {
			if (xMin > minXmin) {
				if (xMin - deltaX < minXmin) {
					highlight_0 = GetTickCount();
					deltaX = xMin - minXmin;
				}
			} else {
				factorX = Null;
				highlight_0 = GetTickCount();
			}
		}
		if (!IsNull(maxXmax) && factorX < 0) {
			if (xMin + xRange < maxXmax) {
				if (xMin + xRange - deltaX > maxXmax) {
					highlight_0 = GetTickCount();
					deltaX = xMin + xRange - maxXmax;
				}
			} else {
				factorX = Null;
				highlight_0 = GetTickCount();
			}
		}
		if (factorX != 0 && !IsNull(factorX)) {	
			xMin -= deltaX;
			xMinUnit += deltaX;
			AdjustMinUnitX();
		}
	}
	if (factorY != 0) {
		double deltaY = factorY*yRange;
		if (!IsNull(minYmin) && factorY > 0) {
			if (yMin > minYmin) {
				if (yMin - deltaY < minYmin) {
					highlight_0 = GetTickCount();
					deltaY = yMin - minYmin;
				}
			} else {
				factorY = Null;
				highlight_0 = GetTickCount();
			}
		}
		if (!IsNull(maxYmax) && factorY < 0) {
			if (yMin + yRange < maxYmax) {
				if (yMin + yRange - deltaY > maxYmax) {
					highlight_0 = GetTickCount();
					deltaY = yMin + yRange - maxYmax;
				}
			} else {
				factorY = Null;
				highlight_0 = GetTickCount();
			}
		}	
		if (factorY != 0 && !IsNull(factorY)) {	
			yMin -= deltaY;
			yMinUnit += deltaY;
			AdjustMinUnitY();
			if (drawY2Reticle) {
				double deltaY2 = factorY*yRange2;
				yMin2 -= deltaY2;
				yMinUnit2 += deltaY2;
				AdjustMinUnitY2();
			}
		}
	}
	if (IsNull(factorX) || IsNull(factorY)) 
		Refresh();
	else if (factorX != 0 || factorY != 0) {	
		WhenSetXYMin();	
		Refresh();
		WhenZoomScroll();
	}
}

Pointf PointAtLen(const Pointf &p0, const Pointf &p1, double len) {
	Pointf ret;
	if (p1.y == p0.y) {
		ret.x = p0.x + ((p0.x < p1.x) ? len : -len);
		ret.y = p0.y;
	} else if (p1.x == p0.x) {
		ret.x = p0.x;
		ret.y = p0.y + ((p0.y < p1.y) ? len : -len);
	} else {
		double k = sqr((p1.x-p0.x)/(p1.y-p0.y));
		double deltax = len/sqrt(1/k + 1);
		double deltay = len/sqrt(k + 1);
		ret.x = p0.x + ((p0.x < p1.x) ? deltax : -deltax);
		ret.y = p0.y + ((p0.y < p1.y) ? deltay : -deltay);
	}
	return ret;
}

inline bool Even(int val)	  	{return !(val%2);}

Vector <double> GetLineDash(String dash) {
	Vector<double> d;
	CParser p(dash);
	try {
		while(!p.IsEof())
			if(!p.Char(':')) {
				double data = p.ReadDouble();
				if (data > 0)
					d.Add(p.ReadDouble());
			}
	}
	catch(CParser::Error) {}

	if(d.GetCount() & 1) {
		Vector<double> dash1;
		dash1.Append(d);
		dash1.Append(d);
		return dash1;
	}		
	return d;
}

Vector <double> &GetDashedArray(String dash) {
	static VectorMap <String, Vector <double> > pats;
	
	int pos = pats.Find(dash);
	if (pos < 0) {
		pats.Add(dash, GetLineDash(dash));
		pos = pats.GetCount()-1;
	} 
	return pats.GetValues()[pos];
}

void ScatterDraw::ParseTextMultiline(const String &text, Upp::Font fnt, Upp::Array <String> &texts, 
		Upp::Array <Size> &sizes) {
	Size ret(0, 0);
	int npos = 0;
	for (int pos = 0; npos != -1; pos = npos+1) {
		npos = text.Find('\n', pos);
		String &t = texts.Add();
		if (npos != -1)
			t = text.Mid(pos, npos-pos);
		else
			t = text.Mid(pos);
		Size &s = sizes.Add();
		s.cx = GetTextSize(t, fnt).cx;
		s.cy = GetTextSize(t, fnt).cy;
	}
}

Size GetTextSizeMultiline(Upp::Array <Size> &sizes) {
	Size ret(0, 0);
	for (int i = 0; i < sizes.GetCount(); ++i) {
		if (sizes[i].cx > ret.cx)
			ret.cx = sizes[i].cx;
		ret.cy += sizes[i].cy;
	}
	return ret;
}

ScatterDraw &ScatterDraw::LinkedWith(ScatterDraw &ctrl) {
	Unlinked();

	if (ctrl.linkedMaster) 
		linkedMaster = ctrl.linkedMaster;
	else 
		linkedMaster = &ctrl;

	linkedMaster->linkedCtrls.FindAdd(this);

	return *this;
}

void ScatterDraw::Unlinked() {
	if (!linkedMaster) {
		for (int i = 0; i < linkedCtrls.GetCount(); ++i) 
			linkedCtrls[i]->linkedMaster = 0;
		linkedCtrls.Clear();
	} else {
		linkedMaster->linkedCtrls.RemoveKey(this);
		linkedMaster = 0;
	}
}
	
ScatterDraw::ScatterDraw() {
	mode = MD_ANTIALIASED;
	size = Size(800, 400);
	titleColor = SColorText();
	graphColor = White();
	titleFont = Roman(20);
	labelsFont = GetStdFont();
	labelsColor = SColorText();
	plotAreaColor = White();//SColorLtFace();
	axisColor = SColorText();
	axisWidth = 6;
	hPlotLeft = hPlotRight = vPlotTop = vPlotBottom = 30;
	xRange = yRange = yRange2 = 100.0;
	xMin = yMin = yMin2 = xMinUnit = yMinUnit = yMinUnit2 = 0;
	gridColor = SColorDkShadow();
	gridWidth = 1;
	drawXReticle = true; drawYReticle = true;
	drawY2Reticle = false;
	drawVGrid = drawHGrid = showLegend = true;
	minXRange = maxXRange = minYRange = maxYRange = -1;
	minXmin = minYmin = maxXmax = maxYmax = Null;
	fastViewX = false;
	sequentialXAll = false;
	zoomStyleX = zoomStyleY = TO_CENTER;
	maxMajorUnitsX = maxMajorUnitsY = 10;
	SetMajorUnitsNum(5, 10);
	//Color(graphColor);	
	isPolar = false;
	lastxRange = xRange;
	lastyRange = yRange;
	highlight_0 = Null;
	labelsChanged = false;
	legendAnchor = LEGEND_ANCHOR_RIGHT_TOP;
	legendPos = Point(5, 5);
	legendNumCols = 1;
	legendAnchor = LEGEND_TOP;
	legendFillColor = White();
	legendBorderColor = Black();
	legendRowSpacing = 5;
	linkedMaster = 0;
	stacked = false;
	serializeFormat = true;
	mouseHandlingX = mouseHandlingY = false;
	selectedSeries = Null;
}

void DrawLine(Draw &w, double x0, double y0, double x1, double y1, double width, const Color &color) {
	w.DrawLine(fround(x0), fround(y0), fround(x1), fround(y1), fround(width), color);
}

void DrawLine(Painter &w, double x0, double y0, double x1, double y1, double width, const Color &color) {
	w.Move(x0, y0).Line(x0, y0).Line(x1, y1).Stroke(width, color);
}

void DrawVArrow(Draw &w, double x0, double y0, double x1, double y1, double width, double aWidth, double aHeight, const Color &color) {
	DrawLine(w, x0, y0, x1, y1, width, color);
	if (fabs(y0 - y1) > 2*aHeight) {
		if (x0 > x1)
			Swap(x0, x1);
		if (y0 > y1)
			Swap(y0, y1);
		Vector <Point> arrowU, arrowL;
		arrowU << Point(fround(x0), fround(y0)) << Point(fround(x0-aWidth), fround(y0+aHeight)) << Point(fround(x0+aWidth), fround(y0+aHeight));
		w.DrawPolygon(arrowU, SColorHighlight());
		arrowL << Point(fround(x0), fround(y1)) << Point(fround(x0-aWidth), fround(y1-aHeight)) << Point(fround(x0+aWidth), fround(y1-aHeight));
		w.DrawPolygon(arrowL, SColorHighlight());
	}
}

void DrawVArrow(Painter &w, double x0, double y0, double x1, double y1, double width, double aWidth, double aHeight, const Color &color) {
	DrawLine(w, x0, y0, x1, y1, width, color);
	if (fabs(y0 - y1) > 2*aHeight) {
		if (x0 > x1)
			Swap(x0, x1);
		if (y0 > y1)
			Swap(y0, y1);
		w.Move(x0, y0).Line(x0-aWidth, y0+aHeight).Line(x0+aWidth, y0+aHeight).Fill(SColorHighlight());
		w.Move(x0, y1).Line(x0-aWidth, y1-aHeight).Line(x0+aWidth, y1-aHeight).Fill(SColorHighlight());
	}
}

void DrawHArrow(Draw &w, double x0, double y0, double x1, double y1, double width, double aWidth, double aHeight, const Color &color) {
	DrawLine(w, x0, y0, x1, y1, width, color);
	if (fabs(x0 - x1) > 2*aHeight) {
		if (x0 > x1)
			Swap(x0, x1);
		if (y0 > y1)
			Swap(y0, y1);
		Vector <Point> arrowL, arrowR;
		arrowL << Point(fround(x0), fround(y0)) << Point(fround(x0+aHeight), fround(y0+aWidth)) << Point(fround(x0+aHeight), fround(y0-aWidth));
		w.DrawPolygon(arrowL, SColorHighlight());
		arrowR << Point(fround(x1), fround(y0)) << Point(fround(x1-aHeight), fround(y0+aWidth)) << Point(fround(x1-aHeight), fround(y0-aWidth));
		w.DrawPolygon(arrowR, SColorHighlight());
	}
}

void DrawHArrow(Painter &w, double x0, double y0, double x1, double y1, double width, double aWidth, double aHeight, const Color &color) {
	DrawLine(w, x0, y0, x1, y1, width, color);
	if (fabs(x0 - x1) > 2*aHeight) {
		if (x0 > x1)
			Swap(x0, x1);
		if (y0 > y1)
			Swap(y0, y1);
		w.Move(x0, y0).Line(x0+aHeight, y0+aWidth).Line(x0+aHeight, y0-aWidth).Fill(SColorHighlight());
		w.Move(x1, y0).Line(x1-aHeight, y0+aWidth).Line(x1-aHeight, y0-aWidth).Fill(SColorHighlight());
	}
}

void DrawText(Draw &w, double x, double y, int angle, const String &text, Upp::Font font, Color color) {
	w.DrawText(fround(x), fround(y), angle, text, font, color);
}

void DrawText(Painter &w, double x, double y, int angle, const String &text, Upp::Font font, Color color) {
	w.Begin();
	w.Translate(x, y).Rotate(-angle*M_PI/1800.);
	w.Text(0, 0, text, font).Fill(color);
	w.End();
}

void Clip(Draw &w, double x, double y, double cx, double cy) {
	w.Clip(fround(x), fround(y), fround(cx), fround(cy));	
}

void Clip(Painter &w, double x, double y, double cx, double cy) {
	w.Rectangle(x, y, cx, cy).Clip();	
}

void ClipEnd(Draw &w) {
	w.End();	
}

void ClipEnd(Painter &w) {
	;	
}

void DrawLineOpa(Draw& w, double x0, double y0, double x1, double y1, int scale, double opacity, 
				double thick, const Color &_color, String dash, const Color &background) {
	Vector<Pointf> p;
	p << Pointf(x0, y0) << Pointf(x1, y1);
	DrawPolylineOpa(w, p, scale, opacity, thick, _color, dash, background);
}

void DrawCircleOpa(Draw& w, double x, double y, double r, int scale, double opacity, 
				double thick, const Color &_color, String dash, const Color &background) {
	Vector<Pointf> p;
	for (double ang = 0; ang <= 2*M_PI; ang += 2*M_PI/50) 
		p << Pointf(fround(x + r*cos(ang)), fround(y + r*sin(ang)));
	DrawPolylineOpa(w, p, scale, opacity, thick, _color, dash, background);
}

void DashScaled(Painter& w, const String dash, double scale) {
	if (!dash.IsEmpty()) {		
		Vector<double> d;
		double start = 0;
		CParser p(dash);
		try {
			while(!p.IsEof())
				if(p.Char(':'))
					start = p.ReadDouble();
				else
					d.Add(scale*p.ReadDouble());
		}
		catch(CParser::Error) {}
		w.Dash(d, scale*start);
	}
}

void DrawLineOpa(Painter& w, double x0, double y0, double x1, double y1, int scale, 
				double opacity, double thick, const Color &color, String dash, 
				const Color &background) {	
	w.Move(Pointf(x0, y0));
	w.Line(Pointf(x1, y1));
	DashScaled(w, dash, scale);
	w.Opacity(opacity);				// Before Stroke()
	w.Stroke(thick*scale, color);
}

void DrawCircleOpa(Painter& w, double x, double y, double r, int scale, 
				double opacity, double thick, const Color &color, String dash, 
				const Color &background) {	
	w.Circle(x, y, r);
	DashScaled(w, dash, scale);
	w.Opacity(opacity);				// Before Stroke()
	w.Stroke(thick*scale, color);
}

void FillRectangleOpa(Draw &w, double x0, double y0, double x1, double y1, int scale, 
					double opacity, const Color &background, const Color &color) {
	if (IsNull(color))
		return;
	Color opacolor = GetOpaqueColor(color, background, opacity);
	if (x0 > x1) 
		Swap(x0, x1);
	if (y0 > y1) 
		Swap(y0, y1);
	w.DrawRect(int(x0), int(y0), abs(int(x1 - x0)), abs(int(y1 - y0)), opacolor);
}

void FillRectangleOpa(Painter &w, double x0, double y0, double x1, double y1, int scale, 
					double opacity, const Color &background, const Color &color) {
	if (IsNull(color))
		return;
	w.Rectangle(x0, y0, x1 - x0, y1 - y0).Opacity(opacity).Fill(color);
}

void DrawPolylineOpa(Draw& w, const Vector<Pointf> &p, int scale, double opacity, 
				double thick, const Color &_color, String dash, const Color &background) {
	ASSERT(!p.IsEmpty());
	Color color = GetOpaqueColor(_color, background, opacity) ;
	if (dash == LINE_SOLID) {
		//w.DrawPolyline(p, fround(thick*scale), color);
		for (int i = 1; i < p.GetCount(); ++i) 
			w.DrawLine(p[i-1], p[i], fround(thick*scale), color);
	} else {
		Vector <double> &pat = GetDashedArray(dash);
		if (pat.IsEmpty())
			return;
		int iPat = 0; 
		
		double len = pat[0]*scale;			// Pixels per bar
		Pointf begin, end;
		begin = p[0];
		for (int i = 1; i < p.GetCount();) {
			double d = Distance(begin, p[i]);
			if (d >= len) 
				end = PointAtLen(begin, p[i], len);
			else {
				end = p[i];
				len -= d;
				++i;
			}
			if (Even(iPat)) 
				w.DrawLine(begin, end, fround(thick*scale), color);
			if (d >= len) {
				iPat++;
				if (iPat == pat.GetCount())
					iPat = 0;				 
				len = pat[iPat]*scale;
			}
			begin = end;
		}
	}		
}

void DrawPolylineOpa(Painter& w, const Vector<Pointf> &p, int scale, double opacity, 
				double thick, const Color &color, String dash, const Color &background) {	
	ASSERT(!p.IsEmpty());
	w.Move(p[0]);
	for (int i = 1; i < p.GetCount(); ++i) 
		w.Line(p[i]);
	DashScaled(w, dash, scale);
	w.Opacity(opacity);				// Before Stroke()
	w.Stroke(thick*scale, color);
}

void FillPolylineOpa(Draw& w, const Vector<Pointf> &p, int scale, double opacity, 
				const Color &background, const Color &fillColor) {
	ASSERT(!p.IsEmpty());
	Color opacolor = GetOpaqueColor(fillColor, background, opacity) ;

	Vector<Point> pi;
	pi.SetCount(p.GetCount());
	for (int i = 0; i < pi.GetCount(); ++i) {
		pi[i].x = (int)p[i].x;
		pi[i].y = (int)p[i].y;
	}
	w.DrawPolygon(pi, opacolor);
}

void FillPolylineOpa(Painter& w, const Vector<Pointf> &p, int scale, double opacity, 
				const Color &background, const Color &fillColor) {	
	ASSERT(!p.IsEmpty());
	w.Move(p[0]);
	for (int i = 1; i < p.GetCount(); ++i) 
		w.Line(p[i]);
	w.Opacity(opacity);
	w.Fill(fillColor);		// Before Stroke()
}

void debug_h() {
	;			// It does nothing. Just to set a breakpoint in templated functions
}
	
INITBLOCK {
	SeriesPlot::Register<LineSeriesPlot>("Line");
	SeriesPlot::Register<StaggeredSeriesPlot>("Staggered");
	SeriesPlot::Register<BarSeriesPlot>("Bar");
	
	MarkPlot::Register<CircleMarkPlot>("Circle");
	MarkPlot::Register<SquareMarkPlot>("Square");
	MarkPlot::Register<TriangleMarkPlot>("Triangle");
	MarkPlot::Register<CrossMarkPlot>("Cross");
	MarkPlot::Register<XMarkPlot>("X");
	MarkPlot::Register<RhombMarkPlot>("Rhomb");
	MarkPlot::Register<RangePlot>("Range");
	MarkPlot::Register<BubblePlot>("Bubble");
	
	DashStyle::Register("LINE_SOLID", "");
	DashStyle::Register("LINE_DOTTED", "4 10");
	DashStyle::Register("LINE_DOTTED_SEP", "4 20");
	DashStyle::Register("LINE_DASHED", "12 12");
	DashStyle::Register("LINE_DASH_DOT", "20 10 5 10");
}
