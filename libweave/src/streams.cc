// Copyright 2015 The Chromium OS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/streams.h"

#include <base/bind.h>
#include <base/callback.h>
#include <weave/provider/task_runner.h>
#include <weave/stream.h>

namespace weave {

namespace {}  // namespace

MemoryStream::MemoryStream(const std::vector<uint8_t>& data,
                           provider::TaskRunner* task_runner)
    : data_{data}, task_runner_{task_runner} {}

void MemoryStream::Read(void* buffer,
                        size_t size_to_read,
                        const ReadSuccessCallback& success_callback,
                        const ErrorCallback& error_callback) {
  CHECK_LE(read_position_, data_.size());
  size_t size_read = std::min(size_to_read, data_.size() - read_position_);
  if (size_read > 0)
    memcpy(buffer, data_.data() + read_position_, size_read);
  read_position_ += size_read;
  task_runner_->PostDelayedTask(FROM_HERE,
                                base::Bind(success_callback, size_read), {});
}

void MemoryStream::Write(const void* buffer,
                         size_t size_to_write,
                         const SuccessCallback& success_callback,
                         const ErrorCallback& error_callback) {
  data_.insert(data_.end(), static_cast<const char*>(buffer),
               static_cast<const char*>(buffer) + size_to_write);
  task_runner_->PostDelayedTask(FROM_HERE, success_callback, {});
}

StreamCopier::StreamCopier(InputStream* source, OutputStream* destination)
    : source_{source}, destination_{destination}, buffer_(4096) {}

void StreamCopier::Copy(
    const InputStream::ReadSuccessCallback& success_callback,
    const ErrorCallback& error_callback) {
  source_->Read(
      buffer_.data(), buffer_.size(),
      base::Bind(&StreamCopier::OnSuccessRead, weak_ptr_factory_.GetWeakPtr(),
                 success_callback, error_callback),
      error_callback);
}

void StreamCopier::OnSuccessRead(
    const InputStream::ReadSuccessCallback& success_callback,
    const ErrorCallback& error_callback,
    size_t size) {
  size_done_ += size;
  if (size) {
    return destination_->Write(
        buffer_.data(), size,
        base::Bind(&StreamCopier::Copy, weak_ptr_factory_.GetWeakPtr(),
                   success_callback, error_callback),
        error_callback);
  }
  success_callback.Run(size_done_);
}

}  // namespace weave