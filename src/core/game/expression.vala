/*
  Game Server Watcher
  Copyright (C) 2025  Alexander Burobin

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

class MapKeywordExpression : FunctionExpression {
  public MapKeywordExpression (Gee.List<Expression> args) {
    this.args = args;
  }

  public override string eval (Map<string, Map<string, string>> ctx) {
    var keywords_str = args[0].eval (ctx);
    var delimiter = args[1].eval (ctx);
    var map_name = args[2].eval (ctx);
    var def = args[3].eval (ctx);

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

}
