using Gee;

namespace Gsw {

abstract class Expression {
  public abstract string eval (Map<string, Map<string, string>> ctx);
}

class LiteralExpression : Expression {
  private string value = "";

  public LiteralExpression (string value) {
    this.value = value;
  }

  public override string eval (Map<string, Map<string, string>> ctx) {
    return value;
  }
}

class ValueExpression : Expression {
  private string map;
  private Expression key_expr;

  public ValueExpression (string map, Expression key_expr) {
    this.map = map;
    this.key_expr = key_expr;
  }

  public override string eval (Map<string, Map<string, string>> ctx) {
    return ctx[map][key_expr.eval (ctx)];
  }
}

abstract class FunctionExpression : Expression {
  protected Gee.List<Expression> args;

  public override string eval (Map<string, Map<string, string>> ctx) {
    return "";
  }
}

class RegexExpression : FunctionExpression {
  public RegexExpression (Gee.List<Expression> args) {
    this.args = args;
  }

  public override string eval (Map<string, Map<string, string>> ctx) {
    try {
      var regex = new Regex (args[0].eval (ctx));
      var input = args[1].eval (ctx);

      MatchInfo match_info;
      if (!regex.match (input, 0, out match_info))
        return "";

      return match_info.fetch (1);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, err.message);
      return "";
    }
  }
}

}
