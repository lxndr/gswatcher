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

using Gee;

namespace Gsw.GameDef {

public errordomain ExpressionError {
  INVALID_MAP,
  INVALID_FUNCTION,
}

public class ExpressionContext : HashMap<string, Map<string, string>> {}

public abstract class Expression : Object {}

public interface EvaluatableExpression : Expression {
  public abstract string eval (ExpressionContext ctx) throws ExpressionError;
}

public interface LogicalExpression : Expression {
  public abstract bool eval (ExpressionContext ctx, string value) throws ExpressionError;
}

class LiteralExpression : Expression, EvaluatableExpression {
  private string value = "";

  public LiteralExpression (string value) {
    this.value = value;
  }

  public string eval (ExpressionContext ctx) throws ExpressionError {
    return value;
  }
}

class ValueExpression : Expression, EvaluatableExpression {
  private string map_name;
  private EvaluatableExpression key_expr;

  public ValueExpression (string map, EvaluatableExpression key_expr) {
    this.map_name = map;
    this.key_expr = key_expr;
  }

  public string eval (ExpressionContext ctx) throws ExpressionError {
    if (!ctx.has_key (map_name))
      throw new ExpressionError.INVALID_MAP ("map '%s' does not exist", map_name);

    var map = ctx[map_name];
    var key = key_expr.eval (ctx);

    if (!map.has_key (key))
      throw new ExpressionError.INVALID_MAP ("key '%s' does not exist in map '%s'", key, map_name);

    return map[key];
  }
}

abstract class FunctionExpression : Expression, EvaluatableExpression {
  private string name;
  private Gee.List<EvaluatableExpression> args;

  protected FunctionExpression (string name, Gee.List<EvaluatableExpression> args) {
    this.name = name;
    this.args = args;
  }

  public abstract string eval (ExpressionContext ctx) throws ExpressionError;

  protected EvaluatableExpression get_arg (int arg_index) throws ExpressionError {
    if (arg_index >= args.size)
      throw new ExpressionError.INVALID_FUNCTION ("missing argument %d", arg_index);

    return args[arg_index];
  }

  protected string eval_arg (ExpressionContext ctx, int arg_index) throws ExpressionError {
    try {
      var expression = get_arg (arg_index);
      return expression.eval (ctx);
    } catch (ExpressionError err) {
      throw new ExpressionError.INVALID_FUNCTION ("failed to evaluate argument %d of '%s()': %s", arg_index, name, err.message);
    }
  }
}

class RegexExpression : FunctionExpression {
  public RegexExpression (Gee.List<EvaluatableExpression> args) {
    base ("regex", args);
  }

  public override string eval (ExpressionContext ctx) throws ExpressionError {
    var pattern = eval_arg (ctx, 0);
    var input = eval_arg (ctx, 1);
    var regex = create_regex (pattern, input);
    return match_regex (regex, input);
  }

  private Regex create_regex (string pattern, string input) throws ExpressionError {
    try {
      return new Regex (pattern);
    } catch (RegexError err) {
      throw new ExpressionError.INVALID_FUNCTION ("failed to create regex for /%s/: %s", pattern, err.message);
    }
  }

  private string match_regex (Regex regex, string input) {
    MatchInfo match_info;

    if (!regex.match (input, 0, out match_info))
      return "";

    return match_info.fetch (1);
  }
}

class MapKeywordExpression : FunctionExpression {
  public MapKeywordExpression (Gee.List<EvaluatableExpression> args) {
    base ("mapKeyword", args);
  }

  public override string eval (ExpressionContext ctx) throws ExpressionError {
    var keywords_str = eval_arg (ctx, 0);
    var delimiter = eval_arg (ctx, 1);
    var map_name = eval_arg (ctx, 2);
    var def = eval_arg (ctx, 3);

    if (keywords_str.length == 0)
      return def;

    if (map_name.length == 0)
      return def;

    if (!ctx.has_key (map_name))
      return def;

    var keywords = keywords_str.split (delimiter);
    var map = ctx[map_name];

    foreach (var keyword in keywords)
      if (map.has_key (keyword))
        return map[keyword];

    return def;
  }
}

class ToMarkupExpression : FunctionExpression {
  public ToMarkupExpression (Gee.List<EvaluatableExpression> args) {
    base ("toMarkup", args);
  }

  public override string eval (ExpressionContext ctx) throws ExpressionError {
    var input = eval_arg (ctx, 0);
    var transformation_type = eval_arg (ctx, 1);

    switch (transformation_type) {
      case "quake-color-code":
        return parse_quake_color_codes (input);
      default:
        throw new ExpressionError.INVALID_FUNCTION ("unknown transformation type `%s`", transformation_type);
    }
  }
}

public class OrExpression : Expression, LogicalExpression {
  private Gee.List<EvaluatableExpression> expressions;

  public OrExpression (Gee.List<EvaluatableExpression> expressions) {
    this.expressions = expressions;
  }

  public bool eval (ExpressionContext ctx, string value) throws ExpressionError {
    foreach (var expr in expressions) {
      if (value == expr.eval (ctx)) {
        return true;
      }
    }

    return false;
  }
}

}
