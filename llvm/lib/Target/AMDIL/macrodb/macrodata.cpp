// Copyright (c) 2011, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software
// without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// If you use the software (in whole or in part), you shall adhere to all
// applicable U.S., European, and other export laws, including but not limited
// to the U.S. Export Administration Regulations (“EAR”), (15 C.F.R. Sections
// 730 through 774), and E.U. Council Regulation (EC) No 1334/2000 of 22 June
// 2000.  Further, pursuant to Section 740.6 of the EAR, you hereby certify
// that, except pursuant to a license granted by the United States Department
// of Commerce Bureau of Industry and Security or as otherwise permitted
// pursuant to a License Exception under the U.S. Export Administration
// Regulations ("EAR"), you will not (1) export, re-export or release to a
// national of a country in Country Groups D:1, E:1 or E:2 any restricted
// technology, software, or source code you receive hereunder, or (2) export to
// Country Groups D:1, E:1 or E:2 the direct product of such technology or
// software, if such foreign produced direct product is subject to national
// security controls as identified on the Commerce Control List (currently
// found in Supplement 1 to Part 774 of EAR).  For the most current Country
// Group listings, or for additional information about the EAR or your
// obligations under those regulations, please refer to the U.S. Bureau of
// Industry and Security’s website at http://www.bis.doc.gov/.
//
//==-----------------------------------------------------------------------===//
//
// Copyright (c) 2009 Advanced Micro Devices, Inc. All rights reserved.
//

/*! \file macrodata.hpp
 *  \brief  Macrodata compile-time/run-time implementation.
 *
 *  \author Alexander Lyashevsky (Alexander.Lyashevsky@amd.com)
 *  \date   March 2009
 */
#include <stdio.h>
#include <string.h>
#include "macrodata.h"
#include "macrodb.h"



namespace amd {

static const char *csMacroCallPattern = "mcall(";

static CMacroData sMacroDataDBObject;

CMacroData :: CMacroData()
{
    mInit = 0;
    mMacroDBCounter = 0;
	mRefNbr = 0;
    mRefIndex = 0;
    mMacroRef = 0;

	InitMacroDB();
	ResolveReferences();
}

CMacroData :: ~CMacroData()
{
	
	if ( mRefNbr )
	{
	   delete [] mRefNbr; 
	   mRefNbr = 0;
	}
	if ( mRefIndex )
	{
	   delete [] mRefIndex; 
	   mRefIndex = 0;
	}
	if ( mMacroRef )
	{
		delete [] mMacroRef;
		mMacroRef = 0;
	}
}

int CMacroData ::SearchForPattern(char *_SearchBuf, const char *_Pattern, int _PatLen)
{
int r = -1;
int BufLen = (int)strlen(_SearchBuf);
   for(int i = 0; i < BufLen - _PatLen; i++)
   {
	   if (!memcmp(&_SearchBuf[i],_Pattern,_PatLen))
	   {
		   r = i;
		   break;
	   }
   }
   return(r);
}

int CMacroData ::ExtractString(int *_Pos0, int *_Pos1,char * _Name, char *_pBuf, const char*_Delim0, const char*_Delim1)
{
int r = 0;
//int len = (int)strlen(_pBuf);
int len0 = (int)strlen(_Delim0);
int len1 = (int)strlen(_Delim1);
      *_Pos0 = SearchForPattern(_pBuf, _Delim0,len0 );
      *_Pos1 = SearchForPattern(&_pBuf[(*_Pos0)+len0], _Delim1, len1);
	  if ( *_Pos0 != -1 && *_Pos1 != -1 )
	  {
    int nameLen = *_Pos1;
		  *_Pos1 += (*_Pos0)+len0;
         memcpy(_Name, &_pBuf[(*_Pos0)+len0], nameLen);
         _Name[nameLen] = 0;
		 r = 1;
	  }
	  return(r);
}

int CMacroData :: InitMacroDB( void )
{
int r = 1;
// count macros
   for(mMacroDBCounter = 0; amd::sMacroDB[mMacroDBCounter].Name[0] != 0; mMacroDBCounter++);
   mInit = 1;
   return (r);
}

int CMacroData :: NumberOfReferences( int Ord )
{
int r = 0;
char *pMacro;
int patLen = (int)strlen(csMacroCallPattern);
int pos = 0;
    pMacro = (char*)sMacroDB[Ord].Body;
    while( 1 )
	{
		pos = SearchForPattern(&pMacro[pos], csMacroCallPattern, patLen);
		if ( pos != -1)
		{
			r++;
			pos += patLen;
		}
		else
		{
			break;
		}
	}

	return(r);
}

int CMacroData :: InsertReferences( int Ord, int StartPos )
{
int r = 0;
char *pMacro;
int patLen = (int)strlen(csMacroCallPattern);
int pos = 0;
    pMacro = (char*)sMacroDB[Ord].Body;
	 r = 0;
    while( 1 )
	{
		pos = SearchForPattern(&pMacro[pos], csMacroCallPattern, patLen);
		if ( pos != -1)
		{
    char Nmbr[64];
	int pos0,pos1;
		    if (ExtractString(&pos0,&pos1,Nmbr, &pMacro[pos], csMacroCallPattern, ")") != -1)
			{
     int newOrd;
		       sscanf(Nmbr,"%d",&newOrd);
			   mMacroRef[StartPos + r] = (char*)sMacroDB[newOrd].Body;
			   r++;
			}
			pos += patLen;
		}
		else
		{
			break;
		}
	}

// last is itself
    mMacroRef[StartPos + mRefNbr[Ord] - 1] = (char*)sMacroDB[Ord].Body;
	return(r);
}

int CMacroData :: ResolveReferences( void )
{
int r = 1;
int totalRef;
int startPos;
  if ( mRefNbr )
  {
	 delete [] mRefNbr; 
  }
   mRefNbr = new int [mMacroDBCounter];
   for(int i = 0; i < mMacroDBCounter; i++)
   {
// plus itself
      mRefNbr[i] = NumberOfReferences(i) + 1;
   }
// count total ref and set starting ref position per macro
   totalRef = 0;
   for(int i = 0; i < mMacroDBCounter; i++)
   {
      totalRef += mRefNbr[i];
   }


  if ( mRefIndex )
  {
	 delete [] mRefIndex; 
  }
   mRefIndex = new int [mMacroDBCounter];


   if ( mMacroRef )
   {
		delete [] mMacroRef;
   }

   mMacroRef = new char*[totalRef];

   startPos = 0;
   for( int i = 0; i < mMacroDBCounter; i++)
   {
      InsertReferences( i, startPos );
	  mRefIndex[i] = startPos;
      startPos += mRefNbr[i];

   }

   return (r);
}


int CMacroData :: MacroDBFindMacro( const char * _pcMacroNm )
{
int r = -1;
   if ( mInit )
   {
       for ( int i = 0; i < mMacroDBCounter; i++)
       {
	        if ( !strcmp(_pcMacroNm,sMacroDB[i].Name))
	        {
		        r = i;
		        break;
	        }
        }
   }
   return(r);
}

const char *CMacroData :: MacroDBGetMacro( int _iMacroId )
{
const char *r = 0;
	if ( mInit && _iMacroId >= 0 && _iMacroId < mMacroDBCounter)
	{
	   r = sMacroDB[_iMacroId].Body;
	}

	return r;
}

const char ** CMacroData :: MacroDBGetMacroList( int *_MacroListCounter, int _iMacroId )
{
const char **r = 0;
	if ( mInit && _MacroListCounter && _iMacroId >= 0 && _iMacroId < mMacroDBCounter)
	{
   int refPos = mRefIndex[_iMacroId];
       r = (const char **)&mMacroRef[refPos];
	   *_MacroListCounter = mRefNbr[_iMacroId];
	}
   return(r);
}

int CMacroData :: MacroDBFindNumInputs( int _iMacroId )
{
	int r = 0;
	if ( mInit && _iMacroId >=0 && _iMacroId < mMacroDBCounter) 
	{
		r = sMacroDB[_iMacroId].Inputs;
	}
	return r;
}

int CMacroData :: MacroDBFindNumOutputs( int _iMacroId )
{
	int r = 0;
	if ( mInit && _iMacroId >=0 && _iMacroId < mMacroDBCounter) 
	{
		r = sMacroDB[_iMacroId].Outputs;
	}
	return r;
}


// public:

int MacroDBFindMacro( const char * _pcMacroNm )
{
	return(sMacroDataDBObject.MacroDBFindMacro(_pcMacroNm));
}

const char ** MacroDBGetMacro( int *_MacroListCounter, int _iMacroId )
{
	return(sMacroDataDBObject.MacroDBGetMacroList(_MacroListCounter, _iMacroId));
}

int MacroDBNumInputs(int _iMacroId) 
{
	return(sMacroDataDBObject.MacroDBFindNumInputs(_iMacroId));
}

int MacroDBNumOutputs(int _iMacroId) 
{
	return (sMacroDataDBObject.MacroDBFindNumOutputs(_iMacroId));
}
} // namespace amd
