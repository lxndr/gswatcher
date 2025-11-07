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

namespace Gsw {

class Game {
  public string id;
  public string protocol;
  public uint16 port;
  public int16 qport_diff;
  public int weight;
  public Gee.Map<string, GameDef.Expression> inf_matches = new Gee.HashMap<string, GameDef.Expression> ();
  public Gee.Map<string, GameDef.EvaluatableExpression> inf = new Gee.HashMap<string, GameDef.EvaluatableExpression> ();
  public Gee.List<PlayerField> pfields = new Gee.ArrayList<PlayerField> ();
  public Gee.Map<ProtocolFeature, string> features = new Gee.HashMap<ProtocolFeature, string> ();
  public GameDef.ExpressionContext maps = new GameDef.ExpressionContext ();

  public Game (string id, string protocol) {
    this.id = id;
    this.protocol = protocol;
  }

  public bool matches (string protocol_id, Gee.Map<string, string> details) {
    if (protocol != protocol_id)
      return false;

    var ctx = new GameDef.ExpressionContext ();
    ctx.set ("inf", details);

    foreach (var entry in inf_matches.entries) {
      try {
        var expr = entry.value;
        var value = details.get (entry.key);

        if (expr is GameDef.EvaluatableExpression && value == expr.eval (ctx))
          return true;

        if (expr is GameDef.LogicalExpression && expr.eval (ctx, value))
          return true;

        return false;
      } catch (GameDef.ExpressionError err) {
        warning ("failed to evaluate value of '%s' for '%s': %s", entry.key, id, err.message);
      }
    }

    return true;
  }

  public void enhance_sinfo (ServerInfo sinfo, Gee.Map<string, string> details) {
    var ctx = new GameDef.ExpressionContext ();
    ctx.set ("inf", details);
    ctx.set_all (maps);

    foreach (var item in inf) {
      try {
        var value = Value (typeof (string));
        value.set_string (item.value.eval (ctx));
        sinfo.set_property (item.key, value);
      } catch (GameDef.ExpressionError err) {
        warning ("failed to evaluate value of '%s' for '%s': %s", item.key, id, err.message);
      }
    }

    sinfo.set ("game-id", id);
  }
}

}
