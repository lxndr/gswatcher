using Gsw;

struct CorrectTableItem {
  unowned string name;
  unowned string addr;
  unowned string host;
  uint16 gport;
  uint16 qport;
}

struct IncorrectTableItem {
  unowned string name;
  unowned string addr;
}

public static int main (string[] args) {
  Test.init (ref args);

  CorrectTableItem[] CORRECT_TABLE = {
    { name: "correct", addr: "awsm.tk:27011", host: "awsm.tk", gport: 27011, qport: 27011 },
    { name: "correct_with_qport", addr: "awsm.tk:27011:30000", host: "awsm.tk", gport: 27011, qport: 30000 },
  };

  foreach (unowned var row in CORRECT_TABLE) {
    Test.add_data_func (@"/core/utils/parse_address/$(row.name)", () => {
      string host;
      uint16 gport;
      uint16 qport;

      assert_true (parse_address (row.addr, out host, out gport, out qport));
      assert_cmpstr (host, EQ, row.host);
      assert_cmpuint (gport, EQ, row.gport);
      assert_cmpuint (qport, EQ, row.qport);
    });
  }

  IncorrectTableItem[] INCORRECT_TABLE = {
    { name: "incorrect", addr: "awsm.tk" },
  };

  foreach (unowned var row in INCORRECT_TABLE) {
    Test.add_data_func (@"/core/utils/parse_address/$(row.name)", () => {
      string host;
      uint16 gport;
      uint16 qport;

      assert_false (parse_address (row.addr, out host, out gport, out qport));
    });
  }

  Test.run ();
  return 0;
}
