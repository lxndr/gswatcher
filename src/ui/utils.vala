namespace Gsw {

string? format_num_players (int num, int max) {
  if (num < 0 && max < 0)
    return null;

  var num_s = num < 0 ? "-" : num.to_string ();
  var max_s = max < 0 ? "-" : max.to_string ();
  return @"$(num_s) / $(max_s)";
}

}
