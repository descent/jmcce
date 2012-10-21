#ifndef COVERTENCODE_H
#define COVERTENCODE_H

#include <iconv.h>
#include <errno.h>

#include <vector>
#include <string>

namespace DS
{
  // use for exception
  class ConvertEncodeEx
  {
    public:
      ConvertEncodeEx (const std::string & msg)
      {
        _msg = msg;
      }
      const std::string & msg ()
      {
        return _msg;
      }
    private:
      std::string _msg;
  };

  // now big5 -> unicode
  class ConvertEncode
  {
    public:
      // default is from big5 to unicode
      ConvertEncode (const char *tocode="ucs-2le", const char *fromcode="big5");
      ~ConvertEncode ();
      // from big5 to unicode
      int convert (const std::string & str, std::vector < int >&unicode);
      // from unicode to big5
      int convert (const std::vector < int >&unicode, std::string & str);

      int convert (const char *inbuf, size_t len, std::string &str);
    private:
    // this make the program not portable, I use intel little endian
      union
      {
        int unicode;
        char byte4[4];
      } _unicode_struct;
    //std::vector<int> _unicode;
      iconv_t _cd;
#ifndef EXCEPT
      int error_;
#endif
  };
}

#endif
