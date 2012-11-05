//==========================================================================
// Name:            fdmdv2_plot_scalar.cpp
// Purpose:         Plots scalar amplitude against time
// Created:         June 22, 2012
// Initial author:  David Witten
// Derived from:    code written by David Rowe
// License:
//
//  Copyright (C) 2012 David Witten
//
//  All rights reserved.
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License version 2.1,
//  as published by the Free Software Foundation.  This program is
//  distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
//  License for more details.
//
//  You should have received a copy of the GNU Lesser General Public License
//  along with this program; if not, see <http://www.gnu.org/licenses/>.
//
//==========================================================================
#include <string.h>
#include "wx/wx.h"
#include "fdmdv2_main.h"
#include "fdmdv2_plot_scalar.h"

BEGIN_EVENT_TABLE(PlotScalar, PlotPanel)
    EVT_PAINT           (PlotScalar::OnPaint)
    EVT_MOTION          (PlotScalar::OnMouseMove)
    EVT_LEFT_DOWN       (PlotScalar::OnMouseDown)
    EVT_LEFT_UP         (PlotScalar::OnMouseUp)
    EVT_MOUSEWHEEL      (PlotScalar::OnMouseWheelMoved)
    EVT_SIZE            (PlotScalar::OnSize)
    EVT_SHOW            (PlotScalar::OnShow)
//    EVT_ERASE_BACKGROUND(PlotScalar::OnErase)
END_EVENT_TABLE()

//----------------------------------------------------------------
// PlotScalar()
//----------------------------------------------------------------
PlotScalar::PlotScalar(wxFrame* parent, 
		       float  t_secs,             // time covered by entire x axis in seconds
		       float  sample_period_secs, // tiem between each sample in seconds
		       float  a_min,              // min ampltude of samples being plotted
		       float  a_max,              // max ampltude of samples being plotted
		       float  graticule_t_step,   // time step of x (time) axis graticule in seconds
		       float  graticule_a_step,   // step of amplitude axis graticule
		       const char a_fmt[]         // printf format string for amlitude axis labels
		       ): PlotPanel(parent)
{
    int i;

    m_t_secs = t_secs;
    m_sample_period_secs = sample_period_secs;
    m_a_min = a_min;
    m_a_max = a_max;
    m_graticule_t_step = graticule_t_step;
    m_graticule_a_step = graticule_a_step;
    assert(strlen(a_fmt) < 15);
    strcpy(m_a_fmt, a_fmt);

    // work out number of samples we will store and allocate storage

    m_samples = m_t_secs/m_sample_period_secs;
    m_mem = new float[m_samples];

    for(i = 0; i < m_samples; i++)
    {
        m_mem[i] = 0.0;
    }
}

//----------------------------------------------------------------
// ~PlotScalar()
//----------------------------------------------------------------
PlotScalar::~PlotScalar()
{
    delete m_mem;
}

//----------------------------------------------------------------
// add_new_sample()
//----------------------------------------------------------------
void PlotScalar::add_new_sample(float sample)
{
    int i;

    for(i = 0; i < m_samples-1; i++)
    {
        m_mem[i] = m_mem[i+1];
    }
    m_mem[m_samples-1] = sample;
}

//----------------------------------------------------------------
// add_new_samples()
//----------------------------------------------------------------
void  PlotScalar::add_new_short_samples(short samples[], int length, float scale_factor)
{
    int i;

    for(i = 0; i < m_samples-length; i++)
        m_mem[i] = m_mem[i+length];
    for(; i < m_samples; i++)
	m_mem[i] = (float)*samples++/scale_factor;
}

//----------------------------------------------------------------
// draw()
//----------------------------------------------------------------
void PlotScalar::draw(wxAutoBufferedPaintDC&  dc)
{
    float index_to_px;
    float a_to_py;
    int   i;
    int   x, y;
    int   prev_x, prev_y;
    float a;

    m_rCtrl = GetClientRect();
    m_rGrid = m_rCtrl;
    m_rGrid = m_rGrid.Deflate(PLOT_BORDER + (XLEFT_OFFSET/2), (PLOT_BORDER + (YBOTTOM_OFFSET/2)));

    // black background

    dc.Clear();
    m_rPlot = wxRect(PLOT_BORDER + XLEFT_OFFSET, PLOT_BORDER, m_rGrid.GetWidth(), m_rGrid.GetHeight());
    wxBrush ltGraphBkgBrush = wxBrush(BLACK_COLOR);
    dc.SetBrush(ltGraphBkgBrush);
    dc.SetPen(wxPen(BLACK_COLOR, 0));
    dc.DrawRectangle(m_rPlot);

    index_to_px = (float)m_rGrid.GetWidth()/m_samples;
    a_to_py = (float)m_rGrid.GetHeight()/(m_a_max - m_a_min);

    wxPen pen;
    pen.SetColour(DARK_GREEN_COLOR);
    pen.SetWidth(1);
    dc.SetPen(pen);

    // draw all samples

    prev_x = prev_y = 0; // stop warning

    for(i = 0; i < m_samples; i++)
    {
        x = index_to_px * i;
	a = m_mem[i];
	if (a < m_a_min) a = m_a_min;
	if (a > m_a_max) a = m_a_max;

	// invert y axis and offset by minimum

        y = m_rGrid.GetHeight() - a_to_py * a + m_a_min*a_to_py;

	// put inside plot window

	x += PLOT_BORDER + XLEFT_OFFSET;
	y += PLOT_BORDER;

	if (i)
	    dc.DrawLine(x, y, prev_x, prev_y);
	prev_x = x; prev_y = y;
    }

    drawGraticule(dc);
}

//-------------------------------------------------------------------------
// drawGraticule()
//-------------------------------------------------------------------------
void PlotScalar::drawGraticule(wxAutoBufferedPaintDC&  dc)
{
    float    t, a;
    int      x, y, text_w, text_h;
    char     buf[15];
    wxString s;
    float    sec_to_px;
    float    a_to_py;

    wxBrush ltGraphBkgBrush;
    ltGraphBkgBrush.SetStyle(wxBRUSHSTYLE_TRANSPARENT);
    ltGraphBkgBrush.SetColour(*wxBLACK);
    dc.SetBrush(ltGraphBkgBrush);
    dc.SetPen(wxPen(BLACK_COLOR, 1));

    sec_to_px = (float)m_rGrid.GetWidth()/m_t_secs;
    a_to_py = (float)m_rGrid.GetHeight()/(m_a_max - m_a_min);

    // upper LH coords of plot area are (PLOT_BORDER + XLEFT_OFFSET, PLOT_BORDER)
    // lower RH coords of plot area are (PLOT_BORDER + XLEFT_OFFSET + m_rGrid.GetWidth(), 
    //                                   PLOT_BORDER + m_rGrid.GetHeight())

    // Vertical gridlines

    dc.SetPen(m_penShortDash);
    for(t=0; t<=m_t_secs; t+=m_graticule_t_step) {
	x = t*sec_to_px;
	x += PLOT_BORDER + XLEFT_OFFSET;
        dc.DrawLine(x, m_rGrid.GetHeight() + PLOT_BORDER, x, PLOT_BORDER);
        sprintf(buf, "%2.1fs", t);
	GetTextExtent(buf, &text_w, &text_h);
        dc.DrawText(buf, x - text_w/2, m_rGrid.GetHeight() + PLOT_BORDER + YBOTTOM_TEXT_OFFSET);
    }

    // Horizontal gridlines

    dc.SetPen(m_penDotDash);
    for(a=m_a_min; a<m_a_max; a+=m_graticule_a_step) {
	y = m_rGrid.GetHeight() - a*a_to_py + m_a_min*a_to_py;
	y += PLOT_BORDER;
	dc.DrawLine(PLOT_BORDER + XLEFT_OFFSET, y, 
		    (m_rGrid.GetWidth() + PLOT_BORDER + XLEFT_OFFSET), y);
        sprintf(buf, m_a_fmt, a);
	GetTextExtent(buf, &text_w, &text_h);
        dc.DrawText(buf, PLOT_BORDER + XLEFT_OFFSET - text_w - XLEFT_TEXT_OFFSET, y-text_h/2);
   }


}

//----------------------------------------------------------------
// OnPaint()
//----------------------------------------------------------------
void PlotScalar::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    draw(dc);
}

//----------------------------------------------------------------
// OnSize()
//----------------------------------------------------------------
void PlotScalar::OnSize(wxSizeEvent& event)
{
}

//----------------------------------------------------------------
// OnShow()
//----------------------------------------------------------------
void PlotScalar::OnShow(wxShowEvent& event)
{
}
