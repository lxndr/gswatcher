using Gsw;

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_func ("/Games/Left 4 Dead", () => {
    var server = new Server ("localhost:27015");
    var client = new Client (server);

    client.notify["querier"].connect(() => {
      if (client.querier != null) {
        client.querier.notify["error"].connect(() => {
          assert_no_error (client.querier.error);
        });
      }
    });
  });

  Test.run ();
  return 0;
}
