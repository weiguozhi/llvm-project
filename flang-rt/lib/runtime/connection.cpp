//===-- lib/runtime/connection.cpp ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "flang-rt/runtime/connection.h"
#include "flang-rt/runtime/environment.h"
#include "flang-rt/runtime/io-stmt.h"
#include <algorithm>

namespace Fortran::runtime::io {
RT_OFFLOAD_API_GROUP_BEGIN

RT_API_ATTRS std::size_t ConnectionState::RemainingSpaceInRecord() const {
  auto recl{recordLength.value_or(openRecl.value_or(
      executionEnvironment.listDirectedOutputLineLengthLimit))};
  return positionInRecord >= recl ? 0 : recl - positionInRecord;
}

RT_API_ATTRS bool ConnectionState::NeedAdvance(std::size_t width) const {
  return positionInRecord > 0 && width > RemainingSpaceInRecord();
}

RT_API_ATTRS bool ConnectionState::IsAtEOF() const {
  return endfileRecordNumber && currentRecordNumber >= *endfileRecordNumber;
}

RT_API_ATTRS bool ConnectionState::IsAfterEndfile() const {
  return endfileRecordNumber && currentRecordNumber > *endfileRecordNumber;
}

RT_API_ATTRS void ConnectionState::HandleAbsolutePosition(std::int64_t n) {
  positionInRecord = std::max(n, std::int64_t{0}) + leftTabLimit.value_or(0);
}

RT_API_ATTRS void ConnectionState::HandleRelativePosition(std::int64_t n) {
  positionInRecord = std::max(leftTabLimit.value_or(0), positionInRecord + n);
}

SavedPosition::SavedPosition(IoStatementState &io) : io_{io} {
  ConnectionState &conn{io_.GetConnectionState()};
  saved_ = conn;
  conn.pinnedFrame = true;
}

SavedPosition::~SavedPosition() {
  if (!cancelled_) {
    ConnectionState &conn{io_.GetConnectionState()};
    while (conn.currentRecordNumber > saved_.currentRecordNumber) {
      io_.BackspaceRecord();
    }
    conn.leftTabLimit = saved_.leftTabLimit;
    conn.furthestPositionInRecord = saved_.furthestPositionInRecord;
    conn.positionInRecord = saved_.positionInRecord;
    conn.pinnedFrame = saved_.pinnedFrame;
  }
}

RT_OFFLOAD_API_GROUP_END
} // namespace Fortran::runtime::io
