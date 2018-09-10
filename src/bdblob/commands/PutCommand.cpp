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

#include <sys/stat.h>
#include "BlobApi.h"
#include "commands/PutCommand.h"


namespace bdblob
{
  extern BlobApi * g_api;


  PutCommand::PutCommand(std::string prefix)
    : Command("put", std::move(prefix))
  {
    this->parser.Register("force", false, "f", "overwrite if file already exists", false, false);
    this->parser.Register("source", Argument::Type::STRING, true, "", "local source", true);
    this->parser.Register("target", Argument::Type::STRING, true, "", "remote target", true);
  }


  int PutCommand::Execute(int argc, const char ** argv)
  {
    assert(g_api);

    if (!this->parser.Parse(argc, argv))
    {
      fprintf(stderr, "Invalid arguments.\n");
      this->PrintUsage();
      return -1;
    }

    bool force = this->parser.GetValue<bool>("force");
    std::string source = this->parser.GetValue<std::string>("source");
    std::string target = this->parser.GetValue<std::string>("target");

    // TODO: support upload folders and wildcard filenames
    struct stat props;
    if (stat(source.c_str(), &props) != 0)
    {
      fprintf(stderr, "%s\n", BlobErrorToString(BlobError::NOT_FOUND));
      return static_cast<int>(BlobError::NOT_FOUND);
    }

    if (S_ISDIR(props.st_mode))
    {
      fprintf(stderr, "%s\n", BlobErrorToString(BlobError::NOT_SUPPORTED));
      return static_cast<int>(BlobError::NOT_SUPPORTED);
    }

    if (target.empty())
    {
      target = "/";
    }

    auto info = g_api->Info(target);
    if (info && info->type == Folder::EntryType::FOLDER)
    {
      target += (target[target.size() - 1] == '/' ? "" : "/") + BlobApi::BaseName(source);
    }

    if (!g_api->Put(target, source, force))
    {
      fprintf(stderr, "%s\n", BlobErrorToString(g_api->Error()));
      return static_cast<int>(g_api->Error());
    }

    return 0;
  }
}