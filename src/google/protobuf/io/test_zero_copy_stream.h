// Protocol Buffers - Google's data interchange format
// Copyright 2023 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_IO_TEST_ZERO_COPY_STREAM_H__
#define GOOGLE_PROTOBUF_IO_TEST_ZERO_COPY_STREAM_H__

#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/absl_check.h"
#include "absl/types/span.h"
#include "google/protobuf/io/zero_copy_stream.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace io {
namespace internal {

// Input stream used for testing the proper handling of input fragmentation.
// It also will assert the preconditions of the methods.
class TestZeroCopyInputStream final : public ZeroCopyInputStream {
 public:
  // The input stream will provide the buffers exactly as passed here.
  TestZeroCopyInputStream(std::initializer_list<std::string> buffers)
      : buffers_(buffers.begin(), buffers.end()) {}
  explicit TestZeroCopyInputStream(const std::vector<std::string>& buffers)
      : buffers_(buffers.begin(), buffers.end()) {}

  // Allow copy to be able to inspect the stream without consuming the original.
  TestZeroCopyInputStream(const TestZeroCopyInputStream& other)
      : ZeroCopyInputStream(),
        buffers_(other.buffers_),
        last_returned_buffer_(
            other.last_returned_buffer_
                ? std::make_unique<std::string>(*other.last_returned_buffer_)
                : nullptr),
        byte_count_(other.byte_count_) {}

  bool Next(const void** data, int* size) override {
    ABSL_CHECK(data) << "data must not be null";
    ABSL_CHECK(size) << "size must not be null";
    last_returned_buffer_ = nullptr;

    // We are done
    if (buffers_.empty()) return false;

    last_returned_buffer_ =
        std::make_unique<std::string>(std::move(buffers_.front()));
    buffers_.pop_front();
    *data = last_returned_buffer_->data();
    *size = static_cast<int>(last_returned_buffer_->size());
    byte_count_ += *size;
    return true;
  }

  void BackUp(int count) override {
    ABSL_CHECK_GE(count, 0) << "count must not be negative";
    ABSL_CHECK(last_returned_buffer_ != nullptr)
        << "The last call was not a successful Next()";
    ABSL_CHECK_LE(static_cast<size_t>(count), last_returned_buffer_->size())
        << "count must be within bounds of last buffer";
    // Return the backed up buffer to the front s.t. Next() can return it.
    buffers_.push_front(
        last_returned_buffer_->substr(last_returned_buffer_->size() - count));
    // Note that last_returned_buffer_ is moved to last_backed_up_buffer_ to
    // avoid use-after-free of data that was exposed to users but not backed up.
    // For example, ZCIS::ReadCord() reads data, then backs up unused data
    // before copying the data to Cord.
    last_backed_up_buffer_ = std::move(last_returned_buffer_);
    byte_count_ -= count;
  }

  bool Skip(int count) override {
    ABSL_CHECK_GE(count, 0) << "count must not be negative";
    last_returned_buffer_ = nullptr;
    while (true) {
      if (count == 0) return true;
      if (buffers_.empty()) return false;
      auto& front = buffers_[0];
      int front_size = static_cast<int>(front.size());
      if (front_size <= count) {
        count -= front_size;
        byte_count_ += front_size;
        buffers_.pop_front();
      } else {
        // The front is enough, just chomp from it.
        front = front.substr(count);
        byte_count_ += count;
        return true;
      }
    }
  }

  int64_t ByteCount() const override { return byte_count_; }

 private:
  // For simplicity of implementation, we pop the elements from the from and
  // move them to `last_returned_buffer_`. It makes it simpler to keep track of
  // the state of the object. The extra cost is not relevant for testing.
  std::deque<std::string> buffers_;
  // std::optional could work here, but std::unique_ptr makes it more likely
  // for sanitizers to detect if the string is used after it is destroyed.
  std::unique_ptr<std::string> last_returned_buffer_;
  // It is possible that some part of the buffer needs to be valid after the
  // unused part is backed up. Keeping last_returned_buffer_ alive broke some
  // tests. Instead, we temporarily store it to last_backed_up_buffer_.
  std::unique_ptr<std::string> last_backed_up_buffer_;
  int64_t byte_count_ = 0;
};

// Output stream used for testing the proper handling of output fragmentation.
// It also will assert the preconditions of the methods.
class TestZeroCopyOutputStream final : public ZeroCopyOutputStream {
 public:
  explicit TestZeroCopyOutputStream(std::vector<std::string>& buffers) {
    for (auto& buffer : buffers) {
      buffers_.emplace_back(absl::MakeSpan(buffer));
    }
  }

  bool Next(void** data, int* size) override {
    ABSL_CHECK(data) << "data must not be null";
    ABSL_CHECK(size) << "size must not be null";
    last_returned_buffer_ = nullptr;

    // We are done
    if (buffers_.empty()) return false;

    last_returned_buffer_ =
        std::make_unique<absl::Span<char>>(std::move(buffers_.front()));
    buffers_.pop_front();
    *data = last_returned_buffer_->data();
    *size = static_cast<int>(last_returned_buffer_->size());
    byte_count_ += *size;
    return true;
  }

  void BackUp(int count) override {
    ABSL_CHECK_GE(count, 0) << "count must not be negative";
    // If the stream is trimmed (e.g. from WriteCord), we may not have a
    // buffer, which is different from TestZeroCopyInputStream.
    if (last_returned_buffer_) {
      ABSL_CHECK_LE(static_cast<size_t>(count), last_returned_buffer_->size())
          << "count must be within bounds of last buffer";
      buffers_.push_front(last_returned_buffer_->subspan(
          last_returned_buffer_->size() - count));
      last_returned_buffer_ = nullptr;
    }
    byte_count_ -= count;
  }

  int64_t ByteCount() const override { return byte_count_; }

 private:
  // For simplicity of implementation, we pop the elements from the from and
  // move them to `last_returned_buffer_`. It makes it simpler to keep track of
  // the state of the object. The extra cost is not relevant for testing.
  std::deque<absl::Span<char>> buffers_;
  // std::optional could work here, but std::unique_ptr makes it more likely
  // for sanitizers to detect if the string is used after it is destroyed.
  std::unique_ptr<absl::Span<char>> last_returned_buffer_;
  int64_t byte_count_ = 0;
};

}  // namespace internal
}  // namespace io
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_IO_TEST_ZERO_COPY_STREAM_H__
