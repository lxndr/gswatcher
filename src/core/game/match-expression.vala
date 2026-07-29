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

/**
 * An expression that decides whether a server detail value matches.
 */
public interface MatchExpression : Expression {
  public abstract bool matches (ExpressionContext ctx, string value) throws ExpressionError;
}

/**
 * A match expression that succeeds when the value equals the result of
 * evaluating the wrapped expression.
 */
public class EqualsExpression : Expression, MatchExpression {
  private EvaluatableExpression expr;

  public EqualsExpression (EvaluatableExpression expr) {
    this.expr = expr;
  }

  public bool is_literal {
    get { return expr is LiteralExpression; }
  }

  public bool matches (ExpressionContext ctx, string value) throws ExpressionError {
    return value == expr.eval (ctx);
  }
}

/**
 * An expression that evaluates to true if the input value matches any of the sub-expressions.
 *
 * Example: <expr1> or <expr2> or <expr3>
 */
public class OrExpression : Expression, MatchExpression {
  private Gee.List<EvaluatableExpression> expressions;

  public OrExpression (Gee.List<EvaluatableExpression> expressions) {
    this.expressions = expressions;
  }

  public bool all_literal {
    get {
      foreach (var expr in expressions) {
        if (!(expr is LiteralExpression))
          return false;
      }

      return true;
    }
  }

  public bool matches (ExpressionContext ctx, string value) throws ExpressionError {
    foreach (var expr in expressions) {
      if (value == expr.eval (ctx))
        return true;
    }

    return false;
  }
}

}
