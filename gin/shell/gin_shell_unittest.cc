// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/path_service.h"
#include "base/process/launch.h"
#include "base/strings/string_util.h"
#include "testing/gtest/include/gtest/gtest.h"

base::FilePath GinShellPath() {
  base::FilePath dir;
  PathService::Get(base::DIR_EXE, &dir);
  return dir.AppendASCII("gin_shell");
}

base::FilePath HelloWorldPath() {
  base::FilePath path;
  PathService::Get(base::DIR_SOURCE_ROOT, &path);
  return path
    .AppendASCII("gin")
    .AppendASCII("shell")
    .AppendASCII("hello_world.js");
}

TEST(GinShellTest, HelloWorld) {
  CommandLine cmd(GinShellPath());
  cmd.AppendArgPath(HelloWorldPath());
  std::string output;
  ASSERT_TRUE(base::GetAppOutput(cmd, &output));
  base::TrimWhitespaceASCII(output, base::TRIM_ALL, &output);
  ASSERT_EQ("Hello World", output);
}
