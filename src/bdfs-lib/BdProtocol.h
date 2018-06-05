/*
  Copyright (c) 2018 Drive Foundation

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#pragma once
#include <stdint.h>
#pragma pack(push,1)
namespace bdfs
{
  // Block Drive Client Protocol
  namespace bdcp
  {
    enum T : uint8_t
    {
      Mount = 0,
      Unmount,
      Create,
      Delete,
      RequestNBD,
      Response
    };

    typedef struct
    {
      uint32_t length;
      T type;
    }BdHdr;

    typedef struct
    {
      BdHdr hdr;
      uint8_t success;
      char message[128];
    }BdResponse;

    typedef struct
    {
      BdHdr hdr;
      char volumeName[128];
      char path[128];
    }BdMount;

    typedef struct
    {
      BdHdr hdr;
      char volumeName[128];
    }BdUnmount;

    typedef struct
    {
      BdHdr hdr;
      char volumeName[128];
      char repoName[128];
      uint16_t dataBlocks;
      uint16_t codeBlocks;
    }BdCreate;
  
    typedef struct
    {
      BdHdr hdr;
      char volumeName[128];
    }BdDelete;
    
    typedef struct
    {
      BdHdr hdr;
      char volumeName[128];
    }BdRequestNBD;

  }
}
#pragma pack(pop)
