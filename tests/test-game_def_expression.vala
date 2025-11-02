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

struct CorrectTableItem {
  unowned string name;
  unowned string pattern;
  unowned string? result;
  Error? error;
}

public static int main (string[] args) {
  Test.init (ref args);

  CorrectTableItem[] CORRECT_TABLE = {
    { name: "string",           pattern: "\"plain string\"",                                        result: "plain string", error: null                                                                                                                                                              },
    { name: "string_malformed", pattern: "\"invalid string",                                        result: null,           error: new GameDef.ExpressionParserError.INVALID_TOKEN ("invalid token at position 16, expected string, integer, float or identifier")                                   },
    { name: "integer",          pattern: "12345",                                                   result: "12345",        error: null                                                                                                                                                              },
    { name: "float",            pattern: "123.45",                                                  result: "123.45",       error: null                                                                                                                                                              },
    { name: "map",              pattern: "map[\"key\"]",                                            result: "map value",    error: null                                                                                                                                                              },
    { name: "map_malformed",    pattern: "map[\"key\"",                                             result: null,           error: new GameDef.ExpressionParserError.INVALID_TOKEN ("failed to parse value: invalid token at position 9, expected ']'")                                              },
    { name: "map_unknown_name", pattern: "unknownMap[\"key\"]",                                     result: null,           error: new GameDef.ExpressionError.INVALID_MAP ("map 'unknownMap' does not exist")                                                                                       },
    { name: "map_unknown_key",  pattern: "map[\"unknownKey\"]",                                     result: null,           error: new GameDef.ExpressionError.INVALID_MAP ("key 'unknownKey' does not exist in map 'map'")                                                                          },
    { name: "regexp_fn",        pattern: "regex(\"user (\\\\d)\", \"user 1\")",                     result: "1",            error: null                                                                                                                                                              },
    { name: "mapKeyword_fn",    pattern: "mapKeyword(\"key,two,three\", \",\", \"map\", \"Four\")", result: "map value",    error: null                                                                                                                                                              },
    { name: "fn_unknown",       pattern: "unknownFn()",                                             result: null,           error: new GameDef.ExpressionParserError.INVALID_TOKEN ("failed to parse function: unknown function 'unknownFn' at position 11, only supported 'regex' or 'mapKeyword'") },
    { name: "fn_malformed",     pattern: "regex(\"user (\\\\d)\",",                                 result: null,           error: new GameDef.ExpressionParserError.INVALID_TOKEN ("invalid token at position 19, expected string, integer, float or identifier")                                   },
    { name: "fn_missing_arg",   pattern: "regex(\"user\")",                                         result: null,           error: new GameDef.ExpressionError.INVALID_FUNCTION ("failed to evaluate argument 1 of 'regex()': missing argument 1")                                                   },
  };

  var ctx = new GameDef.ExpressionContext ();
  ctx["map"] = new Gee.HashMap<string, string> ();
  ctx["map"]["key"] = "map value";

  foreach (unowned var row in CORRECT_TABLE) {
    Test.add_data_func (@"/core/utils/game_def_expression/$(row.name)", () => {
      try {
        var parser = new GameDef.ExpressionParser (row.pattern);
        var expression = parser.parse ();
        var value = expression.eval (ctx);

        assert_cmpstr (value, EQ, row.result);
      } catch (Error error) {
        if (row.error == null) {
          assert_no_error (error);
        } else {
          assert_error (error, row.error.domain, row.error.code);
          assert_cmpstr (error.message, EQ, row.error.message);
        }
      }
    });
  }

  Test.run ();
  return 0;
}
