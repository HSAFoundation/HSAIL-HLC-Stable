#pragma warning(disable:4350) //behavior change: 'function template' is called instead of 'function'

#include <cstdlib>
#include <cstring>
#include <cassert>
#include <vector>
#include <string>
#include <map>

#include "elfutils.h"

namespace DebugInformationLinking {

  //SectionHeaderStringTable implementation
  //TODO_HSA(3)this is copied from libBRIGDWARF, we should move this to the separate module
  SectionHeaderStringTable::SectionHeaderStringTable() {}
	
  SectionHeaderStringTable::~SectionHeaderStringTable() {}

  size_t SectionHeaderStringTable::addHeaderName( const std::string & headerName )
  {
	  size_t newOffset = m_data.size();
	  m_data.insert( m_data.end(), headerName.begin(), headerName.end() );
	  m_data.push_back( '\0' );
	  return newOffset;
  }

  void SectionHeaderStringTable::clear()
  {
    m_data.clear();
  }

  const unsigned char * SectionHeaderStringTable::rawHeaderData()
  {
	  return &m_data[0];
  }

  size_t SectionHeaderStringTable::rawHeaderSize()
  {
	  return m_data.size();
  }

  const std::string brigCode = ".brigcode";
  const std::string brigDirectives = ".brigdirectives";

}; //namespace DebugInformationLinker
