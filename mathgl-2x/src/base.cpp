/***************************************************************************
 * base.cpp is part of Math Graphic Library
 * Copyright (C) 2007 Alexey Balakin <balakin@appl.sci-nnov.ru>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "mgl2/base.h"
#include "mgl2/define.h"
//-----------------------------------------------------------------------------
char *mgl_strdup(const char *s)
{
	char *r = (char *)malloc((strlen(s)+1)*sizeof(char));
	memcpy(r,s,(strlen(s)+1)*sizeof(char));
	return r;
}
//-----------------------------------------------------------------------------
void mgl_strtrim(char *str)
{
	char *c = mgl_strdup(str);
	size_t n=strlen(str);
	long k;
	for(k=0;k<long(strlen(str));k++)	// óäàëÿåì íà÷àëüíûå ïðîáåëû
		if(str[k]>' ')	break;
	strcpy(c,&(str[k]));
	n = strlen(c);
	for(k=n-1;k>=0;k--)	// óäàëÿåì íà÷àëüíûå ïðîáåëû
		if(c[k]>' ')		break;
	c[k+1] = 0;
	strcpy(str,c);	free(c);
}
//-----------------------------------------------------------------------------
void mgl_strlwr(char *str)
{
	for(long k=0;k<(long)strlen(str);k++)	// óäàëÿåì íà÷àëüíûå ïðîáåëû
		str[k] = (str[k]>='A' && str[k]<='Z') ? str[k]+'a'-'A' : str[k];
}
//-----------------------------------------------------------------------------
mglBase::mglBase()
{
//	memset(this,0,sizeof(mglBase));	// since mglBase is abstract then I can do it?!!
	Flag=0;	saved=false;
#if MGL_HAVE_PTHREAD
	pthread_mutex_init(&mutexPnt,0);
	pthread_mutex_init(&mutexTxt,0);
	pthread_mutex_init(&mutexSub,0);
	pthread_mutex_init(&mutexLeg,0);
	pthread_mutex_init(&mutexPrm,0);
	pthread_mutex_init(&mutexPtx,0);
	pthread_mutex_init(&mutexStk,0);
	pthread_mutex_init(&mutexGrp,0);
	pthread_mutex_init(&mutexGlf,0);
	pthread_mutex_init(&mutexAct,0);
	pthread_mutex_init(&mutexDrw,0);
#endif
	fnt=0;	*FontDef=0;	fx=fy=fz=fa=fc=0;
	AMin = mglPoint(0,0,0,0);	AMax = mglPoint(1,1,1,1);

	InUse = 1;	SetQuality();
	// Always create default palette txt[0] and default scheme txt[1]
	Txt.reserve(3);
	MGL_PUSH(Txt,mglTexture(MGL_DEF_PAL,-1),mutexTxt);
	MGL_PUSH(Txt,mglTexture("BbcyrR",1),mutexTxt);
	memcpy(last_style,"{k5}-1\0",8);
	MinS=mglPoint(-1,-1,-1);	MaxS=mglPoint(1,1,1);
}
mglBase::~mglBase()	{	ClearEq();	}
//-----------------------------------------------------------------------------
void mglBase::AddActive(long k,int n)
{
	if(k<0 || (size_t)k>=Pnt.size())	return;
	mglActivePos p;
	const mglPnt &q=Pnt[k];
	p.x = int(q.x);	p.y = int(q.y);	p.id = ObjId;	p.n = n;
	MGL_PUSH(Act,p,mutexAct);
}
//-----------------------------------------------------------------------------
mreal mglBase::GetRatio() const	{	return 1;	}
//-----------------------------------------------------------------------------
void mglBase::StartGroup(const char *name, int id)
{
	LightScale();
	char buf[128];
	sprintf(buf,"%s_%d",name,id);
	StartAutoGroup(buf);
}
//-----------------------------------------------------------------------------
const char *mglWarn[mglWarnEnd] = {"data dimension(s) is incompatible",
								"data dimension(s) is too small",
								"minimal data value is negative",
								"no file or wrong data dimensions",
								"not enough memory",
								"data values are zero",
								"too many legend entries",
								"no legend entries",
								"slice value is out of range",
								"number of contours is zero or negative",
								"couldn't open file",
								"light: ID is out of range",
								"size(s) is zero or negative",
								"format is not supported for that build",
								"axis ranges are incompatible",
								"pointer is NULL",
								"not enough space for plot"};
//-----------------------------------------------------------------------------
void mglBase::SetWarn(int code, const char *who)
{
	WarnCode = code>0 ? code:0;
	if(code>0 && code<mglWarnEnd)
	{
		if(who && *who)	Mess = Mess+"\n"+who+": ";
		else Mess += "\n";
		Mess = Mess+mglWarn[code-1];
	}
	else if(!code)	Mess="";
	else if(who && *who)	Mess = Mess+(code==-2?"":"\n")+who;
	LoadState();
}
//-----------------------------------------------------------------------------
//		Add glyph to the buffer
//-----------------------------------------------------------------------------
void mglGlyph::Create(long Nt, long Nl)
{
	if(Nt<0 || Nl<0)	return;
	nt=Nt;	nl=Nl;	
	if(trig)	delete []trig;
	trig = nt>0?new short[6*nt]:0;
	if(line)	delete []line;
	line = nl>0?new short[4*nl]:0;
}
//-----------------------------------------------------------------------------
bool mglGlyph::operator==(const mglGlyph &g)
{
	if(nl!=g.nl || nt!=g.nt)	return false;
	if(trig && memcmp(trig,g.trig,6*nt*sizeof(short)))	return false;
	if(line && memcmp(line,g.line,4*nl*sizeof(short)))	return false;
	return true;
}
//-----------------------------------------------------------------------------
long mglBase::AddGlyph(int s, long j)
{
	// first create glyph for current typeface
	s = s&3;
	mglGlyph g(fnt->GetNt(s,j), fnt->GetNl(s,j));	
	memcpy(g.trig, fnt->GetTr(s,j), 6*g.nt*sizeof(short));
	memcpy(g.line, fnt->GetLn(s,j), 4*g.nl*sizeof(short));
	// now let find the similar glyph
	register size_t i;
	for(i=0;i<Glf.size();i++)	if(g==Glf[i])	return i;
	// if no one then let add it
	MGL_PUSH(Glf,g,mutexGlf);	return Glf.size()-1;
}
//-----------------------------------------------------------------------------
//		Add points to the buffer
//-----------------------------------------------------------------------------
long mglBase::AddPnt(mglPoint p, mreal c, mglPoint n, mreal a, int scl)
{
	if(mgl_isnan(c) || mgl_isnan(a))	return -1;
	if(scl>0)	ScalePoint(p,n,!(scl&2));
	if(mgl_isnan(p.x))	return -1;
	a = (a>=0 && a<=1) ? a : AlphaDef;
	c = (c>=0) ? c:CDef;

	mglPnt q;
	if(get(MGL_REDUCEACC))
	{
		q.x=q.xx=int(p.x*10)*0.1;	q.y=q.yy=int(p.y*10)*0.1;	q.z=q.zz=int(p.z*10)*0.1;
		q.c=int(c*100)*0.01;	q.t=q.ta=int(a*100)*0.01;
		q.u=int(n.x*100)*0.01;	q.v=int(n.y*100)*0.01;	q.w=int(n.z*100)*0.01;
	}
	else
	{
		q.x=q.xx=p.x;	q.y=q.yy=p.y;	q.z=q.zz=p.z;
		q.c=c;	q.t=q.ta=a;	q.u=n.x;	q.v=n.y;	q.w=n.z;
	}
	register long ci=long(c);
	if(ci<0 || ci>=(long)Txt.size())	ci=0;	// NOTE never should be here!!!
	const mglTexture &txt=Txt[ci];
	txt.GetC(c,a,q);	// RGBA color

	// add gap for texture coordinates for compatibility with OpenGL
	const mreal gap = 1./512;
	q.c = ci+(q.c-ci)*(1-2*gap)+gap;
	q.t = q.t*(1-2*gap)+gap;
	q.ta = q.t;
	
	if(!get(MGL_ENABLE_ALPHA))	{	q.a=1;	if(txt.Smooth!=2)	q.ta=1-gap;	}
//	if(q.ta<0.005)	q.ta = 0.005;	// bypass OpenGL/OBJ/PRC bug
	if(scl&8 && scl>0)	q.a=a;	// bypass palette for enabling alpha in Error()
	if(!get(MGL_ENABLE_LIGHT) && !(scl&4))	q.u=q.v=NAN;
	MGL_PUSH(Pnt,q,mutexPnt);	return Pnt.size()-1;
}
//-----------------------------------------------------------------------------
long mglBase::CopyNtoC(long from, mreal c)
{
	if(from<0)	return -1;
	mglPnt p=Pnt[from];
	if(!mgl_isnan(c))	{	p.c=c;	p.t=0;	Txt[long(c)].GetC(c,0,p);	}
	MGL_PUSH(Pnt,p,mutexPnt);	return Pnt.size()-1;
}
//-----------------------------------------------------------------------------
long mglBase::CopyProj(long from, mglPoint p, mglPoint n)
{
	if(from<0)	return -1;
	mglPnt q=Pnt[from];
	q.x=q.xx=p.x;	q.y=q.yy=p.y;	q.z=q.zz=p.z;
	q.u=n.x;	q.v=n.y;	q.w=n.z;
	MGL_PUSH(Pnt,q,mutexPnt);	return Pnt.size()-1;
}
//-----------------------------------------------------------------------------
void mglBase::Reserve(long n)
{
	if(TernAxis&4)	n*=4;
	Pnt.reserve(n);
}
//-----------------------------------------------------------------------------
//		Boundaries and scaling
//---------------------------------------------------------------------------
void mglBase::RecalcCRange()
{
	if(!fa)
	{	FMin.c = Min.c;	FMax.c = Max.c;	}
	else
	{
		FMin.c = 1e30;	FMax.c = -1e30;
		register int i;
		mreal a;
		int n=30;
		for(i=0;i<=n;i++)
		{
			a = fa->Calc(0,0,0,Min.c+i*(Max.c-Min.c)/n);
			if(a<FMin.c)	FMin.c=a;
			if(a>FMax.c)	FMax.c=a;
		}
	}
}
//-----------------------------------------------------------------------------
void mglBase::RecalcBorder()
{
	ZMin = 1.;
	if(!fx && !fy && !fz)
	{	FMin = Min;	FMax = Max;	}
	else
	{
		FMin = mglPoint( 1e30, 1e30, 1e30);
		FMax = mglPoint(-1e30,-1e30,-1e30);
		register int i,j;
		int n=30;
		for(i=0;i<=n;i++)	for(j=0;j<=n;j++)	// x range
		{
			SetFBord(Min.x, Min.y+i*(Max.y-Min.y)/n, Min.z+j*(Max.z-Min.z)/n);
			SetFBord(Max.x, Min.y+i*(Max.y-Min.y)/n, Min.z+j*(Max.z-Min.z)/n);
		}
		for(i=0;i<=n;i++)	for(j=0;j<=n;j++)	// y range
		{
			SetFBord(Min.x+i*(Max.x-Min.x)/n, Min.y, Min.z+j*(Max.z-Min.z)/n);
			SetFBord(Min.x+i*(Max.x-Min.x)/n, Max.y, Min.z+j*(Max.z-Min.z)/n);
		}
		for(i=0;i<=n;i++)	for(j=0;j<=n;j++)	// x range
		{
			SetFBord(Min.x+i*(Max.x-Min.x)/n, Min.y+j*(Max.y-Min.y)/n, Min.x);
			SetFBord(Min.x+i*(Max.x-Min.x)/n, Min.y+j*(Max.y-Min.y)/n, Max.z);
		}
		mreal d;
		if(!fx)	{	FMin.x = Min.x;	FMax.x = Max.x;	}
		else	{	d=0.01*(FMax.x-FMin.x);	FMin.x-=d;	FMax.x+=d;	}
		if(!fy)	{	FMin.y = Min.y;	FMax.y = Max.y;	}
		else	{	d=0.01*(FMax.y-FMin.y);	FMin.y-=d;	FMax.y+=d;	}
		if(!fz)	{	FMin.z = Min.z;	FMax.z = Max.z;	}
		else	{	d=0.01*(FMax.z-FMin.z);	FMin.z-=d;	FMax.z+=d;	}
	}
	RecalcCRange();
}
//-----------------------------------------------------------------------------
void mglBase::SetFBord(mreal x,mreal y,mreal z)
{
	if(fx)
	{
		mreal v = fx->Calc(x,y,z);
		if(FMax.x < v)	FMax.x = v;
		if(FMin.x > v)	FMin.x = v;
	}
	if(fy)
	{
		mreal v = fy->Calc(x,y,z);
		if(FMax.y < v)	FMax.y = v;
		if(FMin.y > v)	FMin.y = v;
	}
	if(fz)
	{
		mreal v = fz->Calc(x,y,z);
		if(FMax.z < v)	FMax.z = v;
		if(FMin.z > v)	FMin.z = v;
	}
}
//-----------------------------------------------------------------------------
bool mglBase::ScalePoint(mglPoint &p, mglPoint &n, bool use_nan) const
{
	mreal &x=p.x, &y=p.y, &z=p.z;
	if(mgl_isnan(x) || mgl_isnan(y) || mgl_isnan(z))	{	x=NAN;	return false;	}
	mreal x1,y1,z1,x2,y2,z2;
	x1 = x>0?x*MGL_EPSILON:x/MGL_EPSILON;	x2 = x<0?x*MGL_EPSILON:x/MGL_EPSILON;
	y1 = y>0?y*MGL_EPSILON:y/MGL_EPSILON;	y2 = y<0?y*MGL_EPSILON:y/MGL_EPSILON;
	z1 = z>0?z*MGL_EPSILON:z/MGL_EPSILON;	z2 = z<0?z*MGL_EPSILON:z/MGL_EPSILON;
	bool res = true;
	if(x2>CutMin.x && x1<CutMax.x && y2>CutMin.y && y1<CutMax.y &&
		z2>CutMin.z && z1<CutMax.z)	res = false;
	if(fc && fc->Calc(x,y,z)!=0)	res = false;

	if(get(MGL_ENABLE_CUT) || !use_nan)
	{
//		if(x1<Min.x || x2>Max.x || y1<Min.y || y2>Max.y || z1<Min.z || z2>Max.z)	res = false;
		if((x1-Min.x)*(x1-Max.x)>0 && (x2-Min.x)*(x2-Max.x)>0)	res = false;
		if((y1-Min.y)*(y1-Max.y)>0 && (y2-Min.y)*(y2-Max.y)>0)	res = false;
		if((z1-Min.z)*(z1-Max.z)>0 && (z2-Min.z)*(z2-Max.z)>0)	res = false;
	}
	else
	{
		if(x1<Min.x)	{x=Min.x;	n=mglPoint(1,0,0);}
		if(x2>Max.x)	{x=Max.x;	n=mglPoint(1,0,0);}
		if(y1<Min.y)	{y=Min.y;	n=mglPoint(0,1,0);}
		if(y2>Max.y)	{y=Max.y;	n=mglPoint(0,1,0);}
		if(z1<Min.z)	{z=Min.z;	n=mglPoint(0,0,1);}
		if(z2>Max.z)	{z=Max.z;	n=mglPoint(0,0,1);}
	}

	x1=x;	y1=y;	z1=z;	x2=y2=z2=1;
	if(fx)	{	x1 = fx->Calc(x,y,z);	x2 = fx->CalcD('x',x,y,z);	}
	if(fy)	{	y1 = fy->Calc(x,y,z);	y2 = fy->CalcD('y',x,y,z);	}
	if(fz)	{	z1 = fz->Calc(x,y,z);	z2 = fz->CalcD('z',x,y,z);	}
	if(mgl_isnan(x1) || mgl_isnan(y1) || mgl_isnan(z1))	{	x=NAN;	return false;	}

	register mreal d;	// TODO: should I update normale for infinite light source (x=NAN)?!?
	d = 1/(FMax.x - FMin.x);	x = (2*x1 - FMin.x - FMax.x)*d;	x2 *= 2*d;
	d = 1/(FMax.y - FMin.y);	y = (2*y1 - FMin.y - FMax.y)*d;	y2 *= 2*d;
	d = 1/(FMax.z - FMin.z);	z = (2*z1 - FMin.z - FMax.z)*d;	z2 *= 2*d;
	n.x *= y2*z2;	n.y *= x2*z2;	n.z *= x2*y2;
	if((TernAxis&3)==1)	// usual ternary axis
	{
		if(x+y>0)
		{
			if(get(MGL_ENABLE_CUT))	res = false;
			else	y = -x;
		}
		x += (y+1)/2;	n.x += n.y/2;
	}
	else if((TernAxis&3)==2)	// quaternary axis
	{
		if(x+y+z>-1)
		{
			if(get(MGL_ENABLE_CUT))	res = false;
			else	z = -1-y-x;
		}
		x += 1+(y+z)/2;		y += (z+1)/3;
		n.x += (n.y+n.z)/2;	n.y += n.z/3;
	}
	if(fabs(x)>MGL_EPSILON || fabs(y)>MGL_EPSILON || fabs(z)>MGL_EPSILON)	res = false;

	if(!res && use_nan)	x = NAN;	// extra sign that point shouldn't be plotted
	return res;
}
//-----------------------------------------------------------------------------
//		Ranges
//-----------------------------------------------------------------------------
void mglScaleAxis(mreal &v1, mreal &v2, mreal x1, mreal x2)
{
	if(x1==x2 || v1==v2)	return;
	mreal dv;	x2-=1;
	if(v1*v2>0 && (v2/v1>=100 || v2/v1<=0.01))	// log scale
	{	dv=log(v2/v1);	v1*=exp(dv*x1);	v2*=exp(dv*x2);	}
	else
	{	dv=v2-v1;	v1+=dv*x1;	v2+=dv*x2;	}
}
//-----------------------------------------------------------------------------
void mglBase::SetRanges(mglPoint m1, mglPoint m2)
{
	if(m1.x!=m2.x)	{	Min.x=m1.x;	Max.x=m2.x;	}
	if(m1.y!=m2.y)	{	Min.y=m1.y;	Max.y=m2.y;	}
	if(m1.z!=m2.z)	{	Min.z=m1.z;	Max.z=m2.z;	}
	if(m1.c!=m2.c)	{	Min.c=m1.c;	Max.c=m2.c;	}
	else			{	Min.c=Min.z;Max.c=Max.z;}
	if(!TernAxis)
	{
		mglScaleAxis(Min.x, Max.x, AMin.x, AMax.x);
		mglScaleAxis(Min.y, Max.y, AMin.y, AMax.y);
		mglScaleAxis(Min.z, Max.z, AMin.z, AMax.z);
		mglScaleAxis(Min.c, Max.c, AMin.c, AMax.c);
	}	
	if(Org.x<Min.x && !mgl_isnan(Org.x))	Org.x = Min.x;
	if(Org.x>Max.x && !mgl_isnan(Org.x))	Org.x = Max.x;
	if(Org.y<Min.y && !mgl_isnan(Org.y))	Org.y = Min.y;
	if(Org.y>Max.y && !mgl_isnan(Org.y))	Org.y = Max.y;
	if(Org.z<Min.z && !mgl_isnan(Org.z))	Org.z = Min.z;
	if(Org.z>Max.z && !mgl_isnan(Org.z))	Org.z = Max.z;

	CutMin = mglPoint(0,0,0);	CutMax = mglPoint(0,0,0);
	RecalcBorder();
}
//-----------------------------------------------------------------------------
void mglBase::CRange(HCDT a,bool add, mreal fact)
{
	mreal v1=a->Minimal(), v2=a->Maximal(), dv;
	dv=(v2-v1)*fact;	v1 -= dv;	v2 += dv;
	if(v1==v2)	return;
	if(!add)	{	Min.c = v1;	Max.c = v2;	}
	else if(Min.c<Max.c)
	{
		if(Min.c>v1)	Min.c=v1;
		if(Max.c<v2)	Max.c=v2;
	}
	else
	{
		dv = Min.c;
		Min.c = v1<Max.c ? v1:Max.c;
		Max.c = v2>dv ? v2:dv;
	}
	if(!TernAxis)	mglScaleAxis(Min.c, Max.c, AMin.c, AMax.c);
	if(Org.c<Min.c && !mgl_isnan(Org.c))	Org.c = Min.c;
	if(Org.c>Max.c && !mgl_isnan(Org.c))	Org.c = Max.c;
	RecalcCRange();
}
//-----------------------------------------------------------------------------
void mglBase::XRange(HCDT a,bool add,mreal fact)
{
	mreal v1=a->Minimal(), v2=a->Maximal(), dv;
	dv=(v2-v1)*fact;	v1 -= dv;	v2 += dv;
	if(v1==v2)	return;
	if(!add)	{	Min.x = v1;	Max.x = v2;	}
	else if(Min.x<Max.x)
	{
		if(Min.x>v1)	Min.x=v1;
		if(Max.x<v2)	Max.x=v2;
	}
	else
	{
		dv = Min.x;
		Min.x = v1<Max.x ? v1:Max.x;
		Max.x = v2>dv ? v2:dv;
	}
	if(!TernAxis)	mglScaleAxis(Min.x, Max.x, AMin.x, AMax.x);
	if(Org.x<Min.x && !mgl_isnan(Org.x))	Org.x = Min.x;
	if(Org.x>Max.x && !mgl_isnan(Org.x))	Org.x = Max.x;
	RecalcBorder();
}
//-----------------------------------------------------------------------------
void mglBase::YRange(HCDT a,bool add,mreal fact)
{
	mreal v1=a->Minimal(), v2=a->Maximal(), dv;
	dv=(v2-v1)*fact;	v1 -= dv;	v2 += dv;
	if(v1==v2)	return;
	if(!add)	{	Min.y = v1;	Max.y = v2;	}
	else if(Min.y<Max.y)
	{
		if(Min.y>v1)	Min.y=v1;
		if(Max.y<v2)	Max.y=v2;
	}
	else
	{
		dv = Min.y;
		Min.y = v1<Max.y ? v1:Max.y;
		Max.y = v2>dv ? v2:dv;
	}
	if(!TernAxis)	mglScaleAxis(Min.y, Max.y, AMin.y, AMax.y);
	if(Org.y<Min.y && !mgl_isnan(Org.y))	Org.y = Min.y;
	if(Org.y>Max.y && !mgl_isnan(Org.y))	Org.y = Max.y;
	RecalcBorder();
}
//-----------------------------------------------------------------------------
void mglBase::ZRange(HCDT a,bool add,mreal fact)
{
	mreal v1=a->Minimal(), v2=a->Maximal(), dv;
	dv=(v2-v1)*fact;	v1 -= dv;	v2 += dv;
	if(v1==v2)	return;
	if(!add)	{	Min.z = v1;	Max.z = v2;	}
	else if(Min.z<Max.z)
	{
		if(Min.z>v1)	Min.z=v1;
		if(Max.z<v2)	Max.z=v2;
	}
	else
	{
		dv = Min.z;
		Min.z = v1<Max.z ? v1:Max.z;
		Max.z = v2>dv ? v2:dv;
	}
	if(!TernAxis)	mglScaleAxis(Min.z, Max.z, AMin.z, AMax.z);
	if(Org.z<Min.z && !mgl_isnan(Org.z))	Org.z = Min.z;
	if(Org.z>Max.z && !mgl_isnan(Org.z))	Org.z = Max.z;
	RecalcBorder();
}
//-----------------------------------------------------------------------------
void mglBase::SetAutoRanges(mreal x1, mreal x2, mreal y1, mreal y2, mreal z1, mreal z2, mreal c1, mreal c2)
{
	if(x1!=x2)	{	Min.x = x1;	Max.x = x2;	}
	if(y1!=y2)	{	Min.y = y1;	Max.y = y2;	}
	if(z1!=z2)	{	Min.z = z1;	Max.z = z2;	}
	if(c1!=c2)	{	Min.c = c1;	Max.c = c2;	}
}
//-----------------------------------------------------------------------------
void mglBase::Ternary(int t)
{
	static mglPoint x1(-1,-1,-1),x2(1,1,1),o(NAN,NAN,NAN);
	static bool c = true;
	TernAxis = t;
	if(t&3)
	{
		if(c)	{	x1 = Min;	x2 = Max;	o = Org;	}
//		c = get(MGL_ENABLE_CUT);	clr(MGL_ENABLE_CUT);
		SetRanges(mglPoint(0,0),mglPoint(1,1,t==1?0:1));
		Org=mglPoint(0,0,0);	c = false;
	}
	else	{	SetRanges(x1,x2);	Org=o;	c=true;	}
}
//-----------------------------------------------------------------------------
//		Transformation functions
//-----------------------------------------------------------------------------
void mglBase::SetFunc(const char *EqX,const char *EqY,const char *EqZ,const char *EqA)
{
	if(fa)	delete fa;	if(fx)	delete fx;
	if(fy)	delete fy;	if(fz)	delete fz;
	if(EqX && *EqX && (EqX[0]!='x' || EqX[1]!=0))
		fx = new mglFormula(EqX);
	else	fx = 0;
	if(EqY && *EqY && (EqY[0]!='y' || EqY[1]!=0))
		fy = new mglFormula(EqY);
	else	fy = 0;
	if(EqZ && *EqZ && (EqZ[0]!='z' || EqZ[1]!=0))
		fz = new mglFormula(EqZ);
	else	fz = 0;
	if(EqA && *EqA && ((EqA[0]!='c' && EqA[0]!='a') || EqA[1]!=0))
		fa = new mglFormula(EqA);
	else	fa = 0;
	RecalcBorder();
}
//-----------------------------------------------------------------------------
void mglBase::CutOff(const char *EqC)
{
	if(fc)	delete fc;
	if(EqC && EqC[0])	fc = new mglFormula(EqC);	else	fc = 0;
}
//-----------------------------------------------------------------------------
void mglBase::SetCoor(int how)
{
	switch(how)
	{
	case mglCartesian:	SetFunc(0,0);	break;
	case mglPolar:
		SetFunc("x*cos(y)","x*sin(y)");	break;
	case mglSpherical:
		SetFunc("x*sin(y)*cos(z)","x*sin(y)*sin(z)","x*cos(y)");	break;
	case mglParabolic:
		SetFunc("x*y","(x*x-y*y)/2");	break;
	case mglParaboloidal:
		SetFunc("(x*x-y*y)*cos(z)/2","(x*x-y*y)*sin(z)/2","x*y");	break;
	case mglOblate:
		SetFunc("cosh(x)*cos(y)*cos(z)","cosh(x)*cos(y)*sin(z)","sinh(x)*sin(y)");	break;
//		SetFunc("x*y*cos(z)","x*y*sin(z)","(x*x-1)*(1-y*y)");	break;
	case mglProlate:
		SetFunc("sinh(x)*sin(y)*cos(z)","sinh(x)*sin(y)*sin(z)","cosh(x)*cos(y)");	break;
	case mglElliptic:
		SetFunc("cosh(x)*cos(y)","sinh(x)*sin(y)");	break;
	case mglToroidal:
		SetFunc("sinh(x)*cos(z)/(cosh(x)-cos(y))","sinh(x)*sin(z)/(cosh(x)-cos(y))",
			"sin(y)/(cosh(x)-cos(y))");	break;
	case mglBispherical:
		SetFunc("sin(y)*cos(z)/(cosh(x)-cos(y))","sin(y)*sin(z)/(cosh(x)-cos(y))",
			"sinh(x)/(cosh(x)-cos(y))");	break;
	case mglBipolar:
		SetFunc("sinh(x)/(cosh(x)-cos(y))","sin(y)/(cosh(x)-cos(y))");	break;
	case mglLogLog:	SetFunc("lg(x)","lg(y)");	break;
	case mglLogX:	SetFunc("lg(x)","");	break;
	case mglLogY:	SetFunc("","lg(y)");	break;
	default:	SetFunc(0,0);	break;
	}
}
//-----------------------------------------------------------------------------
void mglBase::ClearEq()
{
	if(fx)	delete fx;	if(fy)	delete fy;	if(fz)	delete fz;
	if(fa)	delete fa;	if(fc)	delete fc;
	fx = fy = fz = fc = fa = 0;
	RecalcBorder();
}
//-----------------------------------------------------------------------------
//		Colors ids
//-----------------------------------------------------------------------------
mglColorID mglColorIds[31] = {{'k', mglColor(0,0,0)},
	{'r', mglColor(1,0,0)},		{'R', mglColor(0.5,0,0)},
	{'g', mglColor(0,1,0)},		{'G', mglColor(0,0.5,0)},
	{'b', mglColor(0,0,1)},		{'B', mglColor(0,0,0.5)},
	{'w', mglColor(1,1,1)},		{'W', mglColor(0.7,0.7,0.7)},
	{'c', mglColor(0,1,1)},		{'C', mglColor(0,0.5,0.5)},
	{'m', mglColor(1,0,1)},		{'M', mglColor(0.5,0,0.5)},
	{'y', mglColor(1,1,0)},		{'Y', mglColor(0.5,0.5,0)},
	{'h', mglColor(0.5,0.5,0.5)},	{'H', mglColor(0.3,0.3,0.3)},
	{'l', mglColor(0,1,0.5)},	{'L', mglColor(0,0.5,0.25)},
	{'e', mglColor(0.5,1,0)},	{'E', mglColor(0.25,0.5,0)},
	{'n', mglColor(0,0.5,1)},	{'N', mglColor(0,0.25,0.5)},
	{'u', mglColor(0.5,0,1)},	{'U', mglColor(0.25,0,0.5)},
	{'q', mglColor(1,0.5,0)},	{'Q', mglColor(0.5,0.25,0)},
	{'p', mglColor(1,0,0.5)},	{'P', mglColor(0.5,0,0.25)},
	{' ', mglColor(-1,-1,-1)},	{0, mglColor(-1,-1,-1)}	// the last one MUST have id=0
};
//-----------------------------------------------------------------------------
void mglColor::Set(mglColor c, float br)
{
	if(br<0)	br=0;	if(br>2.f)	br=2.f;
	r = br<=1.f ? c.r*br : 1 - (1-c.r)*(2-br);
	g = br<=1.f ? c.g*br : 1 - (1-c.g)*(2-br);
	b = br<=1.f ? c.b*br : 1 - (1-c.b)*(2-br);
	a = 1;
}
//-----------------------------------------------------------------------------
void mglColor::Set(char p, float bright)
{
	Set(-1,-1,-1);
	for(long i=0; mglColorIds[i].id; i++)
		if(mglColorIds[i].id==p)
		{	Set(mglColorIds[i].col, bright);	break;	}
}
//-----------------------------------------------------------------------------
void mglTexture::Set(const char *s, int smooth, mreal alpha)
{
	// NOTE: New syntax -- colors are CCCCC or {CNCNCCCN}; options inside []
	if(!s || !s[0])	return;
	strncpy(Sch,s,259);	Smooth=smooth;	Alpha=alpha;

	register long i,j=0,m=0,l=strlen(s);
	const char *dig = "0123456789abcdefABCDEF";
	for(i=0;i<l;i++)		// find number of colors
	{
		if(s[i]=='[')	j++;	if(s[i]==']')	j--;
		if(strchr(MGL_COLORS,s[i]) && j<1)	n++;
		if(s[i]=='x' && i>0 && s[i-1]=='{' && j<1)	n++;
//		if(smooth && s[i]==':')	break;	// NOTE: should use []
	}
	if(!n)
	{
		// it seems to be the only case where new color scheme should be
		if((strchr(s,'|') || strchr(s,'!')) && !smooth)
		{	n=l=6;	s="BbcyrR";	smooth = -1;	}
		else	return;
	}
	if(strchr(s,'|') && !smooth)	smooth = -1;
	mglColor *c = new mglColor[2*n];		// Colors itself
	mreal *val = new mreal[n];
	if(mglchr(s,'%'))	smooth = 2;		// use coordinates in AddPnt() too !!!
	bool map = (smooth==2), sm = smooth>=0, man=sm;	// Use mapping, smoothed colors
	for(i=j=n=0;i<l;i++)	// fill colors
	{
		if(s[i]=='[')	j++;	if(s[i]==']')	j--;
		if(s[i]=='{')	m++;	if(s[i]=='}')	m--;
		if(strchr(MGL_COLORS,s[i]) && j<1 && (m==0 || s[i-1]=='{'))	// {CN,val} format, where val in [0,1]
		{
			if(m>0 && s[i+1]>'0' && s[i+1]<='9')// ext color
			{	c[2*n] = mglColor(s[i],(s[i+1]-'0')/5.f);	i++;	}
			else	c[2*n] = mglColor(s[i]);	// usual color
			val[n]=-1;	n++;
		}
		if(s[i]=='x' && i>0 && s[i-1]=='{' && j<1)	// {xRRGGBB,val} format, where val in [0,1]
		{
			if(strchr(dig,s[i+1]) && strchr(dig,s[i+2]) && strchr(dig,s[i+3]) && strchr(dig,s[i+4]) && strchr(dig,s[i+5]) && strchr(dig,s[i+6]))
			{
				char ss[3]="00";	c[2*n].a = 1;
				ss[0] = s[i+1];	ss[1] = s[i+2];	c[2*n].r = strtol(ss,0,16)/255.;
				ss[0] = s[i+3];	ss[1] = s[i+4];	c[2*n].g = strtol(ss,0,16)/255.;
				ss[0] = s[i+5];	ss[1] = s[i+6];	c[2*n].b = strtol(ss,0,16)/255.;
				if(strchr(dig,s[i+7]) && strchr(dig,s[i+8]))
				{	ss[0] = s[i+7];	ss[1] = s[i+8];	c[2*n].a = strtol(ss,0,16)/255.;	i+=2;	}
				val[n]=-1;	i+=6;	n++;
			}
		}
		if(s[i]==',' && m>0 && j<1 && n>0)
			val[n-1] = atof(s+i+1);
		// NOTE: User can change alpha if it placed like {AN}
		if(s[i]=='A' && j<1 && m>0 && s[i+1]>'0' && s[i+1]<='9')
		{	man=false;	alpha = 0.1*(s[i+1]-'0');	i++;	}
	}
	for(i=0;i<n;i++)	// default texture
	{	c[2*i+1]=c[2*i];	c[2*i].a=man?0:alpha;	c[2*i+1].a=alpha;	}
	if(map && sm)		// map texture
	{
		if(n==2)
		{	c[1]=c[2];	c[2]=c[0];	c[0]=BC;	c[3]=c[1]+c[2];	}
		else if(n==3)
		{	c[1]=c[2];	c[2]=c[0];	c[0]=BC;	c[3]=c[4];	n=2;}
		else
		{	c[1]=c[4];	c[3]=c[6];	n=2;	}
		for(i=0;i<4;i++)	c[i].a=alpha;
		val[0]=val[1]=-1;
	}

	// fill missed values  of val[]
	float  v1=0,v2=1;
	std::vector <long>  def;
	val[0]=0;	val[n-1]=1;	// boundary have to be [0,1]
	for(i=0;i<n;i++) if(val[i]>0 && val[i]<1) 	def.push_back(i);
	def.push_back(n-1);
	long i1=0,i2;
	for(size_t j=0;j<def.size();j++)	for(i=i1+1;i<def[j];i++)
	{
		i1 = j>0?def[j-1]:0;	i2 = def[j];
		v1 = val[i1];	v2 = val[i2];
		v2 = i2-i1>1?(v2-v1)/(i2-i1):0;
		val[i]=v1+v2*(i-i1);
	}
	// fill texture itself
	register mreal u,v=sm?(n-1)/255.:n/256.;
	for(i=0,i1=0;i<256;i++)
	{
		u = v*i;	j = long(u);	u-=j;
		if(!sm || j==n-1)
		{	col[2*i] = c[2*j];	col[2*i+1] = c[2*j+1];	}
		else if(j>n-1)	// NOTE: never should be here!
		{	col[2*i] = c[2*n-2];col[2*i+1] = c[2*n-1];	/*printf("AddTexture -- out of bounds");*/	}
		else
		{
			// advanced scheme using val
			for(;i1<n-1 && i>=255*val[i1];i1++);
			v2 = i1<n?1/(val[i1]-val[i1-1]):0;
			j=i1-1;	u=(i/255.-val[j])*v2;

			col[2*i] = c[2*j]*(1-u)+c[2*j+2]*u;
			col[2*i+1]=c[2*j+1]*(1-u)+c[2*j+3]*u;
		}
	}
	delete []c;	delete []val;
}
//-----------------------------------------------------------------------------
void mglTexture::GetC(mreal u,mreal v,mglPnt &p) const
{
	u -= long(u);
	register long i=long(255*u);	u = u*255-i;
	const mglColor *s=col+2*i;
	p.r = (s[0].r*(1-u)+s[2].r*u)*(1-v) + (s[1].r*(1-u)+s[3].r*u)*v;
	p.g = (s[0].g*(1-u)+s[2].g*u)*(1-v) + (s[1].g*(1-u)+s[3].g*u)*v;
	p.b = (s[0].b*(1-u)+s[2].b*u)*(1-v) + (s[1].b*(1-u)+s[3].b*u)*v;
	p.a = (s[0].a*(1-u)+s[2].a*u)*(1-v) + (s[1].a*(1-u)+s[3].a*u)*v;
//	p.a = (s[0].a*(1-u)+s[2].a*u)*v + (s[1].a*(1-u)+s[3].a*u)*(1-v);	// for alpha use inverted
}
//-----------------------------------------------------------------------------
long mglBase::AddTexture(const char *cols, int smooth)
{
	mglTexture t(cols,smooth,smooth==2?AlphaDef:1);
	if(t.n==0)	return smooth<0 ? 0:1;
	if(smooth<0)	CurrPal=0;
	// check if already exist
	for(size_t i=0;i<Txt.size();i++)	if(t.IsSame(Txt[i]))	return i;
	// create new one
	MGL_PUSH(Txt,t,mutexTxt);	return Txt.size()-1;
}
//-----------------------------------------------------------------------------
mreal mglBase::AddTexture(mglColor c)
{
	register size_t i,j;
	if(!c.Valid())	return -1;
	// first lets try an existed one
	for(i=0;i<Txt.size();i++)	for(j=0;j<255;j++)
		if(c==Txt[i].col[2*j])
			return i+j/255.;
	// add new texture
	mglTexture t;
	for(i=0;i<514;i++)	t.col[i]=c;
	MGL_PUSH(Txt,t,mutexTxt);	return Txt.size()-1;
}
//-----------------------------------------------------------------------------
//		Coloring and palette
//-----------------------------------------------------------------------------
mreal mglBase::NextColor(long &id)
{
	long i=abs(id)/256, n=Txt[i].n, p=abs(id)&0xff;
	if(id>=0)	{	p=(p+1)%n;	id = 256*i+p;	}
	mglColor c = Txt[i].col[int(512*(p+0.5)/n)];
	mreal dif, dmin=1;
	// try to find closest color
	for(long j=0;mglColorIds[j].id;j++)	for(long k=1;k<10;k++)
	{
		mglColor cc;	cc.Set(mglColorIds[j].col,k/5.);
		dif = (c-cc).NormS();
		if(dif<dmin)
		{
			last_style[1] = mglColorIds[j].id;
			last_style[2] = k+'0';
			dmin=dif;
		}
	}
	if(!leg_str.empty())
	{	AddLegend(leg_str.c_str(),last_style);	leg_str.clear();	}
	CDef = i + (n>0 ? (p+0.5)/n : 0);	CurrPal++;
	return CDef;
}
//-----------------------------------------------------------------------------
const char *mglchr(const char *str, char ch)
{
	if(!str || !str[0])	return NULL;
	register char c;
	register size_t l=strlen(str),i,k=0;
	for(i=0;i<l;i++)
	{
		c = str[i];
		if(c=='{')	k++;
		if(c=='}')	k--;
		if(c==ch && k==0)	return str+i;
	}
	return NULL;
}
//-----------------------------------------------------------------------------
char mglBase::SetPenPal(const char *p, long *Id)
{
	char mk=0;
	PDef = 0xffff;	// reset to solid line
	memcpy(last_style,"{k5}-1\0",8);

	Arrow1 = Arrow2 = 0;	PenWidth = 1;
	if(p && *p)
	{
//		const char *col = "wkrgbcymhRGBCYMHWlenuqpLENUQP";
		const char *stl = " -|;:ji=";
		const char *mrk = "*o+xsd.^v<>";
		const char *wdh = "123456789";
		const char *arr = "AKDTVISO_";
		long m=0;
		for(size_t i=0;i<strlen(p);i++)
		{
			if(p[i]=='{')	m++;	if(p[i]=='}')	m--;
			if(m>0)	continue;
			if(mglchr(stl,p[i]))
			{
				switch(p[i])
				{
				case '|': PDef = 0x00ff;	break;
				case ';': PDef = 0x0f0f;	break;
				case '=': PDef = 0x3333;	break;
				case ':': PDef = 0x1111;	break;
				case 'j': PDef = 0x087f;	break;
				case 'i': PDef = 0x2727;	break;
				case ' ': PDef = 0x0000;	break;
				default:  PDef = 0xffff;	break;	// '-'
				}
				last_style[4]=p[i];
			}
			else if(mglchr(mrk,p[i]))	mk = p[i];
			else if(mglchr(wdh,p[i]))
			{	last_style[5] = p[i];	PenWidth = p[i]-'0';	}
			else if(mglchr(arr,p[i]))
			{
				if(!Arrow2)	Arrow2 = p[i];
				else	Arrow1 = p[i];
			}
		}
		if(Arrow1=='_')	Arrow1=0;	if(Arrow2=='_')	Arrow2=0;
		if(mglchr(p,'#'))
		{
			if(mk=='.')	mk = 'C';
			if(mk=='+')	mk = 'P';
			if(mk=='x')	mk = 'X';
			if(mk=='o')	mk = 'O';
			if(mk=='d')	mk = 'D';
			if(mk=='s')	mk = 'S';
			if(mk=='^')	mk = 'T';
			if(mk=='v')	mk = 'V';
			if(mk=='<')	mk = 'L';
			if(mk=='>')	mk = 'R';
			if(mk=='*')	mk = 'Y';
		}
	}
	last_style[6]=mk;
	long tt, n;
	tt = AddTexture(p,-1);	n=Txt[tt].n;
	CDef = tt+((n+CurrPal-1)%n+0.5)/n;
	if(Id)	*Id=long(tt)*256+(n+CurrPal-1)%n;
	return mk;
}
//-----------------------------------------------------------------------------
mreal mglBase::GetA(mreal a) const
{
	if(fa)	a = fa->Calc(0,0,0,a);
	a = (a-FMin.c)/(FMax.c-FMin.c);
	a = (a<1?(a>0?a:0):1)/MGL_EPSILON;	// for texture a must be <1 always!!!
	return a;
}
//-----------------------------------------------------------------------------
mglPoint GetX(HCDT x, int i, int j, int k)
{
	k = k<x->GetNz() ? k : 0;
	if(x->GetNy()>1)
		return mglPoint(x->v(i,j,k),x->dvx(i,j,k),x->dvy(i,j,k));
	else
		return mglPoint(x->v(i),x->dvx(i),0);
}
//-----------------------------------------------------------------------------
mglPoint GetY(HCDT y, int i, int j, int k)
{
	k = k<y->GetNz() ? k : 0;
	if(y->GetNy()>1)
		return mglPoint(y->v(i,j,k),y->dvx(i,j,k),y->dvy(i,j,k));
	else
		return mglPoint(y->v(j),0,y->dvx(j));
}
//-----------------------------------------------------------------------------
mglPoint GetZ(HCDT z, int i, int j, int k)
{
	if(z->GetNy()>1)
		return mglPoint(z->v(i,j,k),z->dvx(i,j,k),z->dvy(i,j,k));
	else
		return mglPoint(z->v(k),0,0);
}
//-----------------------------------------------------------------------------
void mglBase::vect_plot(long p1, long p2, mreal s)
{
	if(p1<0 || p2<0)	return;
	const mglPnt &q1=Pnt[p1], &q2=Pnt[p2];
	mglPnt s1=q2,s2=q2;
	s = s<=0 ? 0.1 : s*0.1;
	s1.x=s1.xx = q2.x - 3*s*(q2.x-q1.x) + s*(q2.y-q1.y);
	s2.x=s2.xx = q2.x - 3*s*(q2.x-q1.x) - s*(q2.y-q1.y);
	s1.y=s1.yy = q2.y - 3*s*(q2.y-q1.y) - s*(q2.x-q1.x);
	s2.y=s2.yy = q2.y - 3*s*(q2.y-q1.y) + s*(q2.x-q1.x);
	s1.z=s1.zz=s2.z=s2.zz = q2.z - 3*s*(q2.z-q1.z);
	long n1,n2;
	n1=Pnt.size();	MGL_PUSH(Pnt,s1,mutexPnt);
	n2=Pnt.size();	MGL_PUSH(Pnt,s2,mutexPnt);
	line_plot(p1,p2);	line_plot(n1,p2);	line_plot(p2,n2);
}
//-----------------------------------------------------------------------------
int mglFindArg(const char *str)
{
	register long l=0,k=0,i;//,j,len=strlen(lst);
	for(i=0;i<long(strlen(str));i++)
	{
		if(str[i]=='\'') l++;
		if(str[i]=='{') k++;
		if(str[i]=='}') k--;
		if(l%2==0 && k==0)
		{
			if(str[i]=='#' || str[i]==';')	return -i;
			if(str[i]<=' ')	return i;
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
void mglBase::SetAmbient(mreal bright)	{	AmbBr = bright;	}
//-----------------------------------------------------------------------------
mreal mglBase::SaveState(const char *opt)
{
	if(!opt || !opt[0] || saved)	return NAN;
	MSS=MarkSize;	ASS=ArrowSize;
	FSS=FontSize;	ADS=AlphaDef;
	MNS=MeshNum;	CSS=Flag;	LSS=AmbBr;
	MinS=Min;		MaxS=Max;	saved=true;
	// parse option
	char *qi=mgl_strdup(opt),*q=qi, *s,*a,*b,*c;
	long n;
	mgl_strtrim(q);
	// NOTE: not consider '#' inside legend entry !!!
	s=strchr(q,'#');	if(s)	*s=0;
	mreal res=NAN;
	while(q && *q)
	{
		s=q;	q=strchr(s,';');
		if(q)	{	*q=0;	q++;	}
		mgl_strtrim(s);		a=s;
		n=mglFindArg(s);	if(n>0)	{	s[n]=0;		s=s+n+1;	}
		mgl_strtrim(a);		b=s;
		n=mglFindArg(s);	if(n>0)	{	s[n]=0;		s=s+n+1;	}
		mgl_strtrim(b);

		mreal ff=atof(b),ss;
		if(!strcmp(b,"on"))	ff=1;
		if(!strcmp(a+1,"range"))
		{
			n=mglFindArg(s);	c=s;
			if(n>0)	{	s[n]=0;	s=s+n+1;	}
			mgl_strtrim(c);		ss = atof(c);
			if(a[0]=='x')		{	Min.x=ff;	Max.x=ss;	}
			else if(a[0]=='y')	{	Min.y=ff;	Max.y=ss;	}
			else if(a[0]=='z')	{	Min.z=ff;	Max.z=ss;	}
//			else if(a[0]=='c')	{	Min.c=ff;	Max.c=ss;	}	// Bad idea since there is formula for coloring
		}
		else if(!strcmp(a,"cut"))		SetCut(ff!=0);
		else if(!strcmp(a,"meshnum"))	SetMeshNum(ff);
		else if(!strcmp(a,"alpha"))		{Alpha(true);	SetAlphaDef(ff);}
		else if(!strcmp(a,"light"))		Light(ff!=0);
		else if(!strcmp(a,"ambient"))	SetAmbient(ff);
		else if(!strcmp(a,"diffuse"))	SetDifLight(ff);
		else if(!strcmp(a,"size"))
		{	SetMarkSize(ff);	SetFontSize(ff);	SetArrowSize(ff);	}
		else if(!strcmp(a,"num") || !strcmp(a,"number") || !strcmp(a,"value"))	res=ff;
		else if(!strcmp(a,"legend"))
		{	if(*b=='\'')	{	b++;	b[strlen(b)-1]=0;	}	leg_str = b;	}
	}
	free(qi);	return res;
}
//-----------------------------------------------------------------------------
void mglBase::LoadState()
{
	if(!saved)	return;
	MarkSize=MSS;	ArrowSize=ASS;
	FontSize=FSS;	AlphaDef=ADS;
	MeshNum=MNS;	Flag=CSS;	AmbBr=LSS;
	Min=MinS;		Max=MaxS;	saved=false;
}
//-----------------------------------------------------------------------------
void mglBase::AddLegend(const wchar_t *text,const char *style)
{	if(text)	MGL_PUSH(Leg,mglText(text,style),mutexLeg);	}
//-----------------------------------------------------------------------------
void mglBase::AddLegend(const char *str,const char *style)
{
	if(!str)	return;
	size_t s = strlen(str)+1;
	wchar_t *wcs = new wchar_t[s];
	mbstowcs(wcs,str,s);
	AddLegend(wcs, style);
	delete []wcs;
}
//-----------------------------------------------------------------------------
bool mgl_check_dim2(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT a, const char *name, bool less)
{
//	if(!gr || !x || !y || !z)	return true;		// if data is absent then should be segfault!!!
	register long n=z->GetNx(),m=z->GetNy();
	if(n<2 || m<2)	{	gr->SetWarn(mglWarnLow,name);	return true;	}
	if(a && n*m*z->GetNz()!=a->GetNx()*a->GetNy()*a->GetNz())
	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	if(less)
	{
		if(x->GetNx()<n)
		{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(y->GetNx()<m && (x->GetNy()<m || y->GetNx()<n || y->GetNy()<m))
		{	gr->SetWarn(mglWarnDim,name);	return true;	}
	}
	else
	{
		if(x->GetNx()!=n)
		{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(y->GetNx()!=m && (x->GetNy()!=m || y->GetNx()!=n || y->GetNy()!=m))
		{	gr->SetWarn(mglWarnDim,name);	return true;	}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool mgl_check_dim1(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT r, const char *name, bool less)
{
//	if(!gr || !x || !y)	return true;		// if data is absent then should be segfault!!!
	register long n=y->GetNx();
	if(n<2)	{	gr->SetWarn(mglWarnLow,name);	return true;	}
	if(less)
	{
		if(x->GetNx()<n)		{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(z && z->GetNx()<n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(r && r->GetNx()<n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	}
	else
	{
		if(x->GetNx()!=n)		{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(z && z->GetNx()!=n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
		if(r && r->GetNx()!=n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	}
	return false;
}
//-----------------------------------------------------------------------------
bool mgl_check_dim3(HMGL gr, bool both, HCDT x, HCDT y, HCDT z, HCDT a, HCDT b, const char *name)
{
// 	if(!gr || !x || !y || !z || !a)	return true;		// if data is absent then should be segfault!!!
	register long n=a->GetNx(),m=a->GetNy(),l=a->GetNz();
	if(n<2 || m<2 || l<2)
	{	gr->SetWarn(mglWarnLow,name);	return true;	}
	if(!(both || (x->GetNx()==n && y->GetNx()==m && z->GetNx()==l)))
	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	if(b && b->GetNx()*b->GetNy()*b->GetNz()!=n*m*l)
	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	return false;
}
//-----------------------------------------------------------------------------
bool mgl_check_trig(HMGL gr, HCDT nums, HCDT x, HCDT y, HCDT z, HCDT a, const char *name, int d)
{
// 	if(!gr || !x || !y || !z || !a || !nums)	return true;		// if data is absent then should be segfault!!!
	long n = x->GetNx(), m = nums->GetNy();
	if(nums->GetNx()<d)	{	gr->SetWarn(mglWarnLow,name);	return true;	}
	if(y->GetNx()!=n || z->GetNx()!=n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	if(a->GetNx()!=m && a->GetNx()!=n)	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	return false;
}
//-----------------------------------------------------------------------------
bool mgl_isboth(HCDT x, HCDT y, HCDT z, HCDT a)
{
	register long n=a->GetNx(),m=a->GetNy(),l=a->GetNz();
	return x->GetNx()*x->GetNy()*x->GetNz()==n*m*l && y->GetNx()*y->GetNy()*y->GetNz()==n*m*l && z->GetNx()*z->GetNy()*z->GetNz()==n*m*l;
}
//-----------------------------------------------------------------------------
bool mgl_check_vec3(HMGL gr, HCDT x, HCDT y, HCDT z, HCDT ax, HCDT ay, HCDT az, const char *name)
{
// 	if(!gr || !x || !y || !z || !ax || !ay || !az)	return true;		// if data is absent then should be segfault!!!
	register long n=ax->GetNx(),m=ax->GetNy(),l=ax->GetNz();
	if(n*m*l!=ay->GetNx()*ay->GetNy()*ay->GetNz() || n*m*l!=az->GetNx()*az->GetNy()*az->GetNz())
	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	if(n<2 || m<2 || l<2)	{	gr->SetWarn(mglWarnLow,name);	return true;	}
	bool both = x->GetNx()*x->GetNy()*x->GetNz()==n*m*l && y->GetNx()*y->GetNy()*y->GetNz()==n*m*l && z->GetNx()*z->GetNy()*z->GetNz()==n*m*l;
	if(!(both || (x->GetNx()==n && y->GetNx()==m && z->GetNx()==l)))
	{	gr->SetWarn(mglWarnDim,name);	return true;	}
	return false;
}
//-----------------------------------------------------------------------------
