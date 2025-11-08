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
    var expr = parse_expression ();

    switch (scanner.peek_next_token ()) {
      case EOF:
        return expr;
      case IDENTIFIER:
        if (scanner.next_value.identifier != "or") {
          scanner.get_next_token ();
          throw_invalid_token (null, "expression");
        }

        return parse_or_expression (expr);
      default:
        throw_invalid_token (null, "EOF or 'or'");
    }
  }

  private EvaluatableExpression parse_expression () throws ExpressionParserError {
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

  private EvaluatableExpression parse_identifier () throws ExpressionParserError {
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

  private EvaluatableExpression parse_value (string name) throws ExpressionParserError {
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

  private EvaluatableExpression parse_function (string name) throws ExpressionParserError {
    var args = parse_function_args ();

    switch (name) {
      case "regex":
        return new RegexExpression (args);
      case "mapKeyword":
        return new MapKeywordExpression (args);
      case "toMarkdown":
        return new ToMarkdownExpression (args);
      default:
        throw new ExpressionParserError.INVALID_TOKEN (
          "failed to parse function: unknown function '%s' at position %u, only supported 'regex' or 'mapKeyword'",
          name, scanner.position
        );
    }
  }

  private Gee.List<EvaluatableExpression> parse_function_args () throws ExpressionParserError {
    var args = new Gee.ArrayList<EvaluatableExpression> ();

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

  private Expression parse_or_expression (EvaluatableExpression first_expr) throws ExpressionParserError {
    var expressions = new Gee.ArrayList<EvaluatableExpression> ();
    expressions.add (first_expr);

    while (!scanner.eof ()) {
      var token = scanner.get_next_token ();

      if (token == EOF) {
        break;
      }

      if (!(token == IDENTIFIER && scanner.value.identifier == "or")) {
        throw_invalid_token (null, "'or'");
      }

      var expr = parse_expression ();
      expressions.add (expr);
    }

    return new OrExpression (expressions);
  }

  [NoReturn]
  private void throw_invalid_token (string? msg, string expected) throws ExpressionParserError {
    throw new ExpressionParserError.INVALID_TOKEN (
      (msg == null ? "" : @"$(msg): ") + "invalid token %s at position %u, expected %s",
      current_token (),
      scanner.position,
      expected
    );
  }

  private string current_token () {
    switch (scanner.cur_token ()) {
      case EOF:
        return "EOF";
      case LEFT_PAREN:
        return "(";
      case RIGHT_PAREN:
        return ")";
      case LEFT_CURLY:
        return "{";
      case RIGHT_CURLY:
        return "}";
      case LEFT_BRACE:
        return "[";
      case RIGHT_BRACE:
        return "]";
      case EQUAL_SIGN:
        return "=";
      case COMMA:
        return ",";
      case NONE:
        return "NONE";
      case ERROR:
        return "ERROR";
      case CHAR:
        return @"'$(scanner.cur_value ().char)'";
      case BINARY:
        return @"'$(scanner.cur_value ().binary)'";
      case OCTAL:
        return @"'$(scanner.cur_value ().octal)'";
      case INT:
        return @"'$(scanner.cur_value ().int)'";
      case HEX:
        return @"'$(scanner.cur_value ().hex)'";
      case FLOAT:
        return @"'$(scanner.cur_value ().float)'";
      case STRING:
        return @"\"$(scanner.cur_value ().string)\"";
      case SYMBOL:
        return @"SYMBOL $((ulong) scanner.cur_value ().symbol)";
      case IDENTIFIER:
        return @"$(scanner.cur_value ().identifier)";
      case IDENTIFIER_NULL:
        return "NULL";
      case COMMENT_SINGLE:
        return "COMMENT";
      case COMMENT_MULTI:
        return "MULTILINE COMMENT";
      case LAST:
        return "LAST";
    }

    return "";
  }
}

}
