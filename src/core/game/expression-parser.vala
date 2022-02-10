namespace Gsw {

errordomain ExpressionParserError {
  INVALID_TOKEN
}

class ExpressionParser {
  private Scanner scanner = new Scanner (null);

  public ExpressionParser (string input) {
    scanner.config.scan_identifier_1char = true;
    scanner.input_text (input, input.length);
  }

  public Expression parse () throws Error {
    return parse_expression ();
  }

  private Expression parse_expression () throws Error {
    switch (scanner.get_next_token ()) {
      case STRING:
        return new LiteralExpression (scanner.value.string);
      case INT:
        return new LiteralExpression (scanner.value.int.to_string ());
      case IDENTIFIER:
        return parse_identifier ();
      default:
        throw new ExpressionParserError.INVALID_TOKEN ("invalid token at '%u'", scanner.position);
    }
  }

  private Expression parse_identifier () throws Error {
    var name = scanner.value.identifier;

    switch (scanner.get_next_token ()) {
      case LEFT_BRACE:
        return parse_value (name);
      case LEFT_PAREN:
        return parse_function (name);
      default:
        throw new ExpressionParserError.INVALID_TOKEN ("invalid token at '%u'", scanner.position);
    }
  }

  private Expression parse_value (string name) throws Error {
    var key_expr = parse_expression ();

    if (scanner.get_next_token () != RIGHT_BRACE)
      throw new ExpressionParserError.INVALID_TOKEN ("invalid token at '%u'", scanner.position);

    return new ValueExpression (name, key_expr);
  }

  private Expression parse_function (string name) throws Error {
    var args = new Gee.ArrayList<Expression> ();

    while (!scanner.eof ()) {
      args.add (parse_expression ());

      var type = scanner.get_next_token ();

      if (type == RIGHT_PAREN) {
        break;
      }

      if (type == COMMA) {
        continue;
      }

      throw new ExpressionParserError.INVALID_TOKEN ("invalid token at '%u'", scanner.position);
    }

    switch (name) {
      case "regex":
        return new RegexExpression (args);
      case "mapKeyword":
        return new MapKeywordExpression (args);
      default:
        throw new ExpressionParserError.INVALID_TOKEN ("invalid token at '%u'", scanner.position);
    }
  }
}

}
