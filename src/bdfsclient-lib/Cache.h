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
#include <string>
#include <map>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include "LockFreeQueue.h"
#include "AsyncResult.h"

namespace dfs
{
  class Volume;

  class Cache
  {
  private:

    struct Item
    {
      uint64_t timestamp = 0;
      bool dirty = false;
    };

    enum class RequestType
    {
      Read,
      Write
    };

    struct Request
    {
      explicit Request(RequestType type) : type(type) {}
      virtual ~Request() = default;

      RequestType type;
    };

    struct ReadRequest : public Request
    {
      ReadRequest(uint64_t row, uint64_t column, void * buffer, size_t size, size_t offset)
        : Request(RequestType::Read)
        , row(row)
        , column(column)
        , buffer(buffer)
        , size(size)
        , offset(offset)
      {
      }

      uint64_t row;
      uint64_t column;
      void * buffer;
      size_t size;
      size_t offset;
      bdfs::AsyncResult<bool> result;
    };

    struct WriteRequest : public Request
    {
      WriteRequest(uint64_t row, uint64_t column, const void * buffer, size_t size, size_t offset)
        : Request(RequestType::Write)
        , row(row)
        , column(column)
        , buffer(buffer)
        , size(size)
        , offset(offset)
      {
      }

      uint64_t row;
      uint64_t column;
      const void * buffer;
      size_t size;
      size_t offset;
      bdfs::AsyncResult<bool> result;
    };


  public:

    explicit Cache(std::string rootPath, Volume * volume, size_t limit, uint32_t flushPolicy = 60);

    ~Cache();

    bool Read(uint64_t row, uint64_t column, void * buffer, size_t size, size_t offset);

    bool Write(uint64_t row, uint64_t column, const void * buffer, size_t size, size_t offset);

  private:

    void ThreadProc();

    bool ReadImpl(uint64_t row, uint64_t column, void * buffer, size_t size, size_t offset);

    bool WriteImpl(uint64_t row, uint64_t column, const void * buffer, size_t size, size_t offset);

    bool ReadFileBlock(const char * filename, uint64_t column, void * buffer);

    bool WriteFileBlock(const char * filename, uint64_t column, const void * buffer);

    void UpdateTimestamp(uint64_t row, bool setDirty);

    void Pop();

    bool Flush(bool force = false);

  private:

    std::string rootPath;

    size_t limit;

    uint32_t flushPolicy;

    Volume * volume;

    bdfs::LockFreeQueue<Request *> requests;

    std::multimap<uint64_t, uint64_t> timestamps;

    std::map<uint64_t, Item> items;

    std::mutex mutex;

    std::thread thread;

    std::condition_variable cond;

    std::atomic<bool> active;

    std::atomic<bool> hasNotification{false};
  };
}

