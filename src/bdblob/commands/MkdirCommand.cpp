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

#include "BlobApi.h"
#include "commands/MkdirCommand.h"

namespace bdblob
{
  extern BlobApi * g_api;


  MkdirCommand::MkdirCommand(std::string prefix)
    : Command("mkdir", std::move(prefix))
  {
    this->parser.Register("recursive", false, "p", "create directories recursively", false, false);
    this->parser.Register("target", Argument::Type::STRING, true, "", "target", true);
  }


  int MkdirCommand::Execute(int argc, const char ** argv)
  {
    assert(g_api);

    if (!this->parser.Parse(argc, argv))
    {
      fprintf(stderr, "Invalid arguments.\n");
      this->PrintUsage();
      return -1;
    }

    bool recursive = this->parser.GetValue<bool>("recursive");
    std::string target = this->parser.GetValue<std::string>("target");

    bool result = g_api->Mkdir(target);
    if (!result)
    {
      if (g_api->Error() != BlobError::NOT_FOUND || !recursive)
      {
        fprintf(stderr, "%s\n", BlobErrorToString(g_api->Error()));
        return static_cast<int>(g_api->Error());
      }

      auto parts = BlobApi::SplitPath(target);
      if (parts.empty())
      {
        // Trying to create root folder
        fprintf(stderr, "%s\n", BlobErrorToString(BlobError::ALREADY_EXIST));
        return static_cast<int>(BlobError::ALREADY_EXIST);
      }

      std::string path;
      for (size_t i = 0; i < parts.size(); ++i)
      {
        path += "/" + parts[i];
        if (!g_api->Mkdir(path) && g_api->Error() != BlobError::ALREADY_EXIST)
        {
          fprintf(stderr, "%s\n", BlobErrorToString(g_api->Error()));
          return static_cast<int>(g_api->Error());
        }
      }
    }

    return 0;
  }
}