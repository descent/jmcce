#include "convertencode.h"

#include <sstream>
#include <iostream>

using namespace std;

//#include <qapplication.h>




DS::ConvertEncode::ConvertEncode (const char *tocode, const char *fromcode):
error_ (0)
{
  //const char to_code[]="ucs-2le";
  //const char to_code[]="ucs-2le";
  //const char to_code[]="unicode";
  //const char from_code[]="big5";
  _cd = iconv_open (tocode, fromcode);
  if (_cd == reinterpret_cast < iconv_t > (-1))	// I use static_cast, but cannot typecast
  {
    if (errno == EINVAL)
    {
#ifdef EXCEPT
      throw
	ConvertEncodeEx ("conver from " + std::string (from_code) +
			 " to " + std::string (to_code) + " not supported");
#else
      error_ = EINVAL;
#endif
      //cout << "conver from " << from_code << " to " << to_code << " not supported" << endl;
      //return -1;
    }
    else
    {
#ifdef EXCEPT
      throw
      ConvertEncodeEx ("iconv_open init fail");
#else
      error_ = -1;
#endif
      //cout << "iconv_open fail" << endl;
      //return -1;
    }
  }
  _unicode_struct.byte4[2] = _unicode_struct.byte4[3] = 0;
  //unicode_struct.byte4[0]=unicode_struct.byte4[1]=0;
}

DS::ConvertEncode::~ConvertEncode ()
{
  if (iconv_close (_cd) == -1)
  {
    if (errno == EBADF)
    {
      //cout << "conversion descriptor is incalid" << endl;
      //return -1;
    }
    else
    {
      //cout << "iconv_close fail" << endl;
      //return -1;
    }
  }
}

int DS::ConvertEncode::convert (const char *inbuf, size_t len, std::string &str)
{
  if (error_ != 0)
    return -1;
  const int  OUTBUF_SIZE=32768;

  char *tmp_char=new char [len];
  for (size_t i=0 ; i < len ; ++i)
    tmp_char[i]=inbuf[i];


  const char *start = tmp_char;
  char outbuf[OUTBUF_SIZE];
  char *outptr;
  size_t outlen;
  size_t n;


  while (len)
  {
    outptr = outbuf;
    outlen = OUTBUF_SIZE;
    n = iconv (_cd, &tmp_char, &len, &outptr, &outlen);


    if (outptr != outbuf)
    {
     //if (fwrite (outbuf, 1, outptr - outbuf, stdout) < (size_t) (outptr - outbuf))
     for (int i=0 ; i < outptr - outbuf ; ++i)
       str.push_back(outbuf[i]);

    }
    if (n != static_cast < size_t > (-1))
    {
      outptr = outbuf;
      outlen = OUTBUF_SIZE;
      // I don't know why call iconv again ???
      (void) iconv (_cd, NULL, NULL, &outptr, &outlen);
      if (outptr != outbuf)
      {
	for (int i = 0; i < outptr - outbuf; ++i)
	{
	  // this should never goto here
	  //printf("%x ",outbuf[i]);
	}
      }
      break;
    }

    std::stringstream format;
    //if (errno != E2BIG)
    //{
    //throw ConvertEncodeEx("out buffer to small");
    //}

    if (errno != E2BIG)
    {
      /* iconv() ran into a problem.  */
      switch (errno)
      {
      case EILSEQ:
	{
	  format << "illegal input sequence at position " << (long) (tmp_char - start);
#ifdef EXCEPT
	  throw
	  ConvertEncodeEx (format.str ());
#else
	  error_ = EILSEQ;
#endif
	  break;
	}
      case EINVAL:
	{
#ifdef EXCEPT
	  throw
	    ConvertEncodeEx
	    ("incomplete character or shift sequence at end of buffer");
#else
	  error_ = EINVAL;
#endif
	  break;
	}
      case EBADF:
	{
#ifdef EXCEPT
	  throw
	  ConvertEncodeEx ("internal error (illegal descriptor)");
#else
	  error_ = EBADF;
#endif
	  break;
	}
      default:
	{
#ifdef EXCEPT
	  throw
	  ConvertEncodeEx ("unknown iconv() error");
#else
	  error_ = -2;
#endif
	  break;
	}
      }
      error_= errno;
      return -1;
    }
    else //(errno == E2BIG)
    {
      error_ = E2BIG;
      break;
    }
  }				// end while(len)

  delete [] start;
  return 0;
}

int DS::ConvertEncode::convert (const std::vector < int >&unicode, std::string & str)
{
  if (error_ != 0) return -1;
  char *char_ptr = new char [unicode.size()*2];
  // one unicode => 2 char
  for (std::vector < int >::size_type i=0 ; i < unicode.size() ; ++i)
  {
    _unicode_struct.unicode=unicode[i];

    char_ptr[i*2]=_unicode_struct.byte4[0];
    char_ptr[i*2+1]=_unicode_struct.byte4[1];
  }

  if (convert(char_ptr, unicode.size()*2, str)!=0)
  {
    delete [] char_ptr;
    return -1;
  }
  delete [] char_ptr;
  return 0;
}

int DS::ConvertEncode::convert (const std::string & str, std::vector < int >&unicode)
{

  if (error_ != 0) return -1;
  std::string return_str;
  if (convert(str.c_str(), str.length(), return_str)!=0)
    return -1;
  for (size_t i=0 ; i < return_str.length(); ++i)
  {
    _unicode_struct.byte4[i%2] = return_str[i];
    if (i % 2 == 1)
      unicode.push_back (_unicode_struct.unicode);
  }


  return 0;
}


#ifdef TEST
#include <algorithm>
#include <iostream>

class Print
{
public:
  bool
  operator     () (int unicode)
  {
    std::cout << unicode << " ";
  }
private:
};

int main ()
{
  using namespace std;

  DS::ConvertEncode convert_encode;
  std::vector < int > unicode;
  //convert_encode.convert ("§Ú¬O§º¥K¤¯", unicode);
  convert_encode.convert ("home", unicode);

  for_each (unicode.begin (), unicode.end (), Print ());

}

#endif

#ifdef TEST_U_BIG5
#include <algorithm>
#include <iostream>

class Print
{
public:
  bool
  operator     () (int unicode)
  {
    std::cout << unicode << " ";
  }
private:
};




int main ()
{
  using namespace std;

  std::vector < int > unicode;
  //int u[]={25105,26159,23435,20181,20161};
  int u[]={104,111,109,101};
  for (int i=0 ; i < sizeof(u)/sizeof(int) ; ++i)
    unicode.push_back(u[i]);

  DS::ConvertEncode convert_encode("big5","ucs-2le");

  std::string s;
  convert_encode.convert (unicode,s);
  cout << s << endl;
  return 0;
}

#endif

#ifdef TEST_ICONV
#include <algorithm>
#include <iostream>

class Print
{
public:
  bool
  operator     () (int unicode)
  {
    std::cout << unicode << " ";
  }
private:
};


#include <cstdio>

int main ()
{
  using namespace std;

  char buf[255];
  fread(buf,1,255,stdin);

  DS::ConvertEncode convert_encode;
  std::string str;
  convert_encode.convert (buf,255,str);
  cout << str << endl;
  cout << str.length() << endl;
  return 0;
}

#endif
