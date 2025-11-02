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

namespace Gsw.GameDef {

errordomain ExpressionParserError {
  INVALID_TOKEN
}

class ExpressionParser {
  private Scanner scanner = new Scanner (null);

  public ExpressionParser (string input) {
    scanner.config.scan_identifier_1char = true;
    scanner.input_text (input, input.length);
  }

  public Expression parse () throws ExpressionParserError {
    return parse_expression ();
  }

  private Expression parse_expression () throws ExpressionParserError {
    switch (scanner.get_next_token ()) {
      case STRING:
        return new LiteralExpression (scanner.value.string);
      case INT:
        return new LiteralExpression (scanner.value.int.to_string ());
      case FLOAT:
        return new LiteralExpression (scanner.value.float.to_string ());
      case IDENTIFIER:
        return parse_identifier ();
      default:
        throw new ExpressionParserError.INVALID_TOKEN (
          "invalid token at position %u, expected string, integer, float or identifier",
          scanner.position
        );
    }
  }

  private Expression parse_identifier () throws ExpressionParserError {
    var name = scanner.value.identifier;

    switch (scanner.get_next_token ()) {
      case LEFT_BRACE:
        return parse_value (name);
      case LEFT_PAREN:
        return parse_function (name);
      default:
        throw new ExpressionParserError.INVALID_TOKEN (
          "failed to parse identifier: invalid token at position %u, expected '[' or '('",
          scanner.position
        );
    }
  }

  private Expression parse_value (string name) throws ExpressionParserError {
    var key_expr = parse_expression ();

    switch (scanner.get_next_token ()) {
      case RIGHT_BRACE:
        return new ValueExpression (name, key_expr);
      default:
        throw new ExpressionParserError.INVALID_TOKEN (
          "failed to parse value: invalid token at position %u, expected ']'",
          scanner.position
        );
    }
  }

  private Expression parse_function (string name) throws ExpressionParserError {
    var args = parse_function_args ();

    switch (name) {
      case "regex":
        return new RegexExpression (args);
      case "mapKeyword":
        return new MapKeywordExpression (args);
      default:
        throw new ExpressionParserError.INVALID_TOKEN (
          "failed to parse function: unknown function '%s' at position %u, only supported 'regex' or 'mapKeyword'",
          name, scanner.position
        );
    }
  }

  private Gee.List<Expression> parse_function_args () throws ExpressionParserError {
    var args = new Gee.ArrayList<Expression> ();

    if (scanner.peek_next_token () == RIGHT_PAREN) {
      scanner.get_next_token ();
      return args;
    }

    while (!scanner.eof ()) {
      args.add (parse_expression ());

      var type = scanner.get_next_token ();

      if (type == RIGHT_PAREN) {
        break;
      }

      if (type == COMMA) {
        continue;
      }

      throw new ExpressionParserError.INVALID_TOKEN (
        "failed to parse function arguments: invalid token at position %u, expected ')' or ','",
        scanner.position
      );
    }

    return args;
  }
}

}
