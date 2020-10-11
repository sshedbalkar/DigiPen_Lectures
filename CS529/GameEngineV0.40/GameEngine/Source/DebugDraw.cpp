///////////////////////////////////////////////////////////////////////////////////////
//
//	DebugDraw.cpp
//
//	Authors: Chris Peters 
//	Copyright 2010, Digipen Institute of Technology
//
///////////////////////////////////////////////////////////////////////////////////////
#include "Precompiled.h"
#include "DebugDraw.h"

namespace Framework
{
	Drawer Drawer::Instance;

	void Drawer::MoveTo(Vec2 v)
	{
		WritePosition = v;
	}

	void Drawer::SetColor(Vec4 color)
	{
		Flush();
		Color = color;
	}

	void Drawer::LineTo(Vec2 newPosition)
	{
		LineSegment seg = {WritePosition,newPosition};
		LineSegments.push_back( seg );
		++SegmentsDrawn;
		WritePosition = newPosition;
	}

	void Drawer::Flush()
	{
		if( SegmentsDrawn > 0 )
		{
			LineSet set = { Color , SegmentsDrawn };
			Sets.push_back( set );
			SegmentsDrawn = 0;
		}
	}

	void Drawer::Clear()
	{
		SegmentsDrawn = 0;
		Sets.clear();
		LineSegments.clear();
	}

	void Drawer::DrawSegment(Vec2 start,Vec2 end)
	{
		MoveTo(start);
		LineTo(end);;
	}

	void  Drawer::DrawCircle(Vec2 center, float radius)
	{
		const unsigned numberOfSegments = 16;
		const float increment = 2.0f * (D3DX_PI) / float(numberOfSegments);

		float theta = 0.0f;
		MoveTo( center + radius * Vec2(cosf(theta), sinf(theta)) );
		for (unsigned i = 1; i <= numberOfSegments; ++i)
		{
			LineTo( center + radius * Vec2(cosf(increment*i), sinf(increment*i)) );
		}
	}

	void  Drawer::DrawBox(Vec2 center,float size)
	{
		float halfSize = size / 2.0f;
		MoveTo( center + Vec2(halfSize,halfSize) );
		LineTo( center + Vec2(halfSize,-halfSize) );
		LineTo( center + Vec2(-halfSize,-halfSize) );
		LineTo( center + Vec2(-halfSize,halfSize)  );
		LineTo( center + Vec2(halfSize,halfSize)  );
	}

	///////////////////////////////////////////////////
	DebugStream::DebugStream():
		m_file_sbuf(NULL),
		m_cout_sbuf(NULL),
		m_to_cout(1)
	{
		init(OUTPUT_FILE);
	}
	DebugStream::DebugStream(const char* file):
		m_file_sbuf(NULL),
		m_cout_sbuf(NULL),
		m_to_cout(1)
	{
		init(file);
	}
	DebugStream::~DebugStream()
	{
		if(m_cout_sbuf)
		{
			std::cout.rdbuf(m_cout_sbuf);
		}
		if(m_file.is_open())
		{
			m_file.close();
			m_file_sbuf = NULL;
		}
	}
	//
	std::streambuf* DebugStream::getFileStreamBuf()
	{
		return m_file_sbuf;
	}
	void DebugStream::toCout(int val)
	{
		m_to_cout = val;
	}
	std::ofstream* DebugStream::getFileStream()
	{
		return &m_file;
	}
	void DebugStream::init(const char* file)
	{
		m_file.open(file);
		m_file_sbuf = m_file.rdbuf();
		m_cout_sbuf = std::cout.rdbuf();
		std::cout.rdbuf(this);
	}
	DebugStream::int_type DebugStream::overflow(DebugStream::int_type c)
	{
		if (!traits_type::eq_int_type(c, traits_type::eof()))
		{
			c = static_cast<int_type>(m_file_sbuf->sputc(c));
			m_file.flush();
			if (m_to_cout && !traits_type::eq_int_type(c, traits_type::eof()))
			{
				c =  static_cast<int_type>(m_cout_sbuf->sputc(c));
			}
			return c;
		}
		else
		{
			return traits_type::not_eof(c);
		}
	}
	int DebugStream::sync()
	{
		int rc = m_file_sbuf->pubsync();
		if (m_to_cout && rc != -1)
		{
			rc = m_cout_sbuf->pubsync();
		}
		return rc;
	}
	/////////////////////
	std::ostream& operator<<(std::ostream &out, const Vec2 &v)
	{
		return out<<"["<<v.x<<", "<<v.y<<"]";
	}
	std::ostream& operator<<(std::ostream &out, const Vec3 &v)
	{
		return out<<"["<<v.x<<", "<<v.y<<", "<<v.z<<"]";
	}
	std::ostream& operator<<(std::ostream &out, const Vec4 &v)
	{
		return out<<"["<<v.x<<", "<<v.y<<", "<<v.z<<", "<<v.w<<"]";
	}
	std::ostream& operator<<(std::ostream &out, const Mat4 &v)
	{
		out<<"[("<<v._11<<", "<<v._12<<", "<<v._13<<", "<<v._14<<"), "
			<<"("<<v._21<<", "<<v._22<<", "<<v._23<<", "<<v._24<<"), "
			<<"("<<v._31<<", "<<v._32<<", "<<v._33<<", "<<v._34<<"), "
			<<"("<<v._41<<", "<<v._42<<", "<<v._43<<", "<<v._44<<")]";
		return out;
	}
}