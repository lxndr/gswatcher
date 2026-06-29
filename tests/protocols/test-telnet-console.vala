/*
  Game Server Watcher
  Copyright (C) 2010-2026 Alexander Burobin

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

using Gsw;

public static int main (string[] args) {
  Environment.set_variable ("GSW_PROTOCOLS_DIR", "../data/protocols", true);

  Test.init (ref args);

  Test.add_data_func ("/protocols/telnet/auth_flow", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("listplayers")
      .param ("password", "secret")
      .expect_send ("\n".data)
      .response ("Please enter password:\n".data)
      .expect_response ("Please enter password:")
      .expect_send ("secret\n".data)
      .response ("Logon successful.\n".data)
      .expect_response ("Logon successful.")
      .expect_send ("listplayers\n".data)
      .response ("1. id=1, name=Player1\n".data)
      .expect_response ("1. id=1, name=Player1")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/auth_failure", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("listplayers")
      .param ("password", "wrong")
      .expect_send ("\n".data)
      .response ("Please enter password:\n".data)
      .expect_response ("Please enter password:")
      .expect_send ("wrong\n".data)
      .response ("Password incorrect, please enter password:\n".data)
      .expect_response ("Password incorrect, please enter password:")
      .expect_error (new ProtocolError.AUTH_FAILED ("authentication failed"))
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/empty_password", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("help")
      .param ("password", "")
      .expect_send ("\n".data)
      .response ("*** Connected to server ***\n".data)
      .expect_response ("*** Connected to server ***")
      .expect_send ("help\n".data)
      .response ("help output\n".data)
      .expect_response ("help output")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/modern_banner", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("help")
      .param ("password", "unused")
      .expect_send ("\n".data)
      .response ("*** Connected with 7DTD server.\r\n".data)
      .expect_response ("*** Connected with 7DTD server.")
      .expect_send ("help\n".data)
      .response ("*** Generic Console Help ***\r\n".data)
      .expect_response ("*** Generic Console Help ***")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/multiline_output", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("version")
      .param ("password", "")
      .expect_send ("\n".data)
      .response ("ready\n".data)
      .expect_response ("ready")
      .expect_send ("version\n".data)
      .response ("line one\nline two\n".data)
      .expect_response ("line one")
      .expect_response ("line two")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/crlf_line_endings", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("help")
      .param ("password", "")
      .expect_send ("\n".data)
      .response ("ready\r\n".data)
      .expect_response ("ready")
      .expect_send ("help\n".data)
      .response ("ok\r\n".data)
      .expect_response ("ok")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/iac_stripping", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("help")
      .param ("password", "")
      .expect_send ("\n".data)
      .response ({ 0xff, 0xfb, 0x01, 'r', 'e', 'a', 'd', 'y', '\n' })
      .expect_response ("ready")
      .expect_send ("help\n".data)
      .response ("ok\n".data)
      .expect_response ("ok")
      .run ();
  });

  Test.add_data_func ("/protocols/telnet/pending_command", () => {
    new ConsoleProtocolTestRunner ("../data/protocols/telnet-console.lua")
      .with_command ("gettime")
      .param ("password", "secret")
      .expect_send ("\n".data)
      .response ("Please enter password:\n".data)
      .expect_response ("Please enter password:")
      .expect_send ("secret\n".data)
      .response ("Logon successful.\n".data)
      .expect_response ("Logon successful.")
      .expect_send ("gettime\n".data)
      .response ("Day 7, 12:00\n".data)
      .expect_response ("Day 7, 12:00")
      .run ();
  });

  Test.run ();
  return 0;
}
