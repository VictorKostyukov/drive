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

#include "Options.h"

#include <string.h>
#include <fstream>
#include <streambuf>
#include <stdarg.h>
#include <json/json.h>

namespace dfs
{
  Action::T Options::Action = Action::Unknown;
  std::string Options::Name;
  uint16_t Options::DataBlocks = 4;
  uint16_t Options::CodeBlocks = 4;
  uint64_t Options::Size =  1*1024*1024*1024; // 1GB
  std::vector<std::string> Options::KademliaUrl;
  std::vector<std::string> Options::Paths;
  std::vector<std::string> Options::ExternalArgs;

  extern void Exit(const char * format, ...);

  void Usage(const char * format = NULL, ...)
  {
    if (format != NULL)
    {
      va_list args;
      va_start(args, format);
      vprintf(format, args);
      va_end(args);
      printf("\n");
    }

    printf("Usage: drive {action} [options] [files]\n");
    printf("\n");
    printf("Actions: create,delete,mount,unmount,format,list\n");
    printf("\n");
    printf("Options: create\n");
    printf("\n");
    printf("  -n {name}      Volume name\n");
    printf("  -s {size}      Volume size in Bytes\n");
    printf("  -k {url}       Kademlia server\n");
    printf("  -d {blocks}    Data blocks (1 .. 255)\n");
    printf("  -c {blocks}    Code blocks (1 .. 255)\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    printf("Options: delete\n");
    printf("\n");
    printf("  -n {name}      Volume name\n");
    printf("  -k {url}       Kademlia server\n");
    printf("  -f {path}      Volume config file\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    printf("Options: mount\n");
    printf("eg: ./drive mount -n volume /folder/to/mount\n");
    printf("\n");
    printf("  -n {name}      Volume name\n");
    printf("  --args {args}  Args for mount command\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    printf("Options: unmount\n");
    printf("\n");
    printf("  -n {name}      Volume name\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    printf("Options: format\n");
    printf("eg: ./drive format -n volume xfs\n");
    printf("\n");
    printf("  -n {name}      Volume name\n");
    printf("  {fstype}       Format type (xfs,ext2,ext3,ext4,ntfs,fat,vfat)\n");
    printf("  --args {args}  Args for mkfs command\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    printf("Options: list\n");
    printf("\n");
    printf("  -?|h           Show this help screen\n");
    printf("\n");
    exit(format == NULL ? 0 : 1);
  }

  uint64_t Options::ParseSizeStr(const std::string & ssize)
  {
    uint64_t s = 0;
    std::string units = ssize.substr(ssize.size()-2, 2);
    s = atoi(ssize.c_str());
    if(units == "KB")
    {
      s *= 1024;
    }
    else if(units == "MB")
    {
      s *= 1024 * 1024;
    }
    else if(units == "GB")
    {
      s *= 1024 * 1024 * 1024;
    }
    return s;
  }

  void Options::ReadConfig()
  {
    std::ifstream cfg("/etc/drive/bdfs.conf");
    std::string data((std::istreambuf_iterator<char>(cfg)),
                 std::istreambuf_iterator<char>());

    if(!data.size())
    {
      return;
    }

    Json::Reader reader;
    Json::Value json;
    if (!reader.parse(data.c_str(), data.size(), json, false) ||
        !json.isObject())
    {
      return;
    }

    if(json["kademlia"].isArray())
    {
      for(int i = 0; i < json["kademlia"].size(); i++)
      {
        auto kd = json["kademlia"][i].asString();
        Options::KademliaUrl.emplace_back(kd);
      }
    }

    if(json["codeBlocks"].isIntegral())
    {
      Options::CodeBlocks = json["codeBlocks"].asUInt();
    }

    if(json["dataBlocks"].isIntegral())
    {
      Options::DataBlocks = json["dataBlocks"].asUInt();
    }

    if(json["size"].isIntegral())
    {
      Options::Size = json["size"].asUInt();
    }
    else if(json["size"].isString())
    {
      Options::Size = ParseSizeStr(json["size"].asString());
    }
  }

  void Options::Init(int argc, char ** argv)
  {
    ReadConfig();

    for (int i = 1; i < argc; i++)
    {
      char * arg = argv[i];

      if (strcmp(arg, "-?") == 0 || strcmp(arg, "-h") == 0)
      {
        Usage();
      }
      else if (strcmp(arg, "--args") == 0)
      {
        while(++i < argc)
        {
          printf("External arg: %s\n",argv[i]);
          Options::ExternalArgs.push_back(argv[i]);
        }
      }
      else if (strcmp(arg, "-n") == 0)
      {
        Options::Name = argv[++i];
      }
      else if (strcmp(arg, "-d") == 0)
      {
        int val = atoi(argv[++i]);
        if (val < 1 || val > 256)
        {
          Usage("\nError: Invalid data block count: %d (valid: 1 .. 255)\n", val);
        }
        Options::DataBlocks = (uint16_t)val;
      }
      else if (strcmp(arg, "-c") == 0)
      {
        int val = atoi(argv[++i]);
        if (val < 1 || val > 256)
        {
          Usage("\nError: Invalid code block count: %d (valid: 1 .. 255)\n", val);
        }
        Options::CodeBlocks = (uint16_t)val;
      }
      else if (strcmp(arg, "-s") == 0)
      {
        errno = 0;
        // Expect size bytes
        Options::Size = strtoull(argv[++i], nullptr, 10);

        if (errno)
        {
          Usage("Error: Invalid size.\n");
        }
      }
      else if (strcmp(arg, "-k") == 0)
      {
        Options::KademliaUrl.insert(Options::KademliaUrl.begin(), argv[++i]);
      }
      else if (strcmp(arg, "-f") == 0)
      {
        Options::Paths.push_back(argv[++i]);
      }
      else if (arg[0] == '-')
      {
        Usage("\nError: Unrecognized option: %s\n", arg);
      }
      else
      {
        if (Options::Action == Action::Unknown)
        {
          Options::Action = Action::FromString(arg);
          if (Options::Action == Action::Unknown)
          {
            Usage("\nError: Unknown action: %s\n", argv[i]);
          }
        }
        else
        {
          Options::Paths.push_back(arg);
        }
      }
    }

    if (Options::Action == Action::Unknown)
    {
      Usage("\nError: No action specified\n");
    }

    if (Options::Action == Action::Create)
    {
      uint16_t totalBlocks = Options::DataBlocks + Options::CodeBlocks;
      if (totalBlocks > 256)
      {
        Usage("\nError: Too many blocks specified: %d (valid: datablocks+codeblocks <= 256)\n", totalBlocks);
      }
    }

    if (Options::Action == Action::Mount && Paths.size() == 0)
    {
      Usage("\nError: No files specified\n");
    }
  }
}
