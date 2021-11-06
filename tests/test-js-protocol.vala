using Gsw;

public static int main (string[] args) {
  Test.init (ref args);

  Test.add_data_func ("/JsProtocol/correct-file", () => {
    try {
      new JsProtocol ("../src/scripts/dist/source.js");
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_data_func ("/JsProtocol/info", () => {
    try {
      var proto = new JsProtocol ("../src/scripts/dist/source.js");
      assert (proto.info.id == "source");
      assert (proto.info.name == "Source Engine");
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_data_func ("/JsProtocol/query", () => {
    try {
      var proto = new JsProtocol ("../src/scripts/dist/source.js");
      proto.query ();
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.add_data_func ("/LuaProtocol/process_response", () => {
    try {
      var proto = new JsProtocol ("../src/scripts/dist/source.js");
      bool signal_called = false;

      proto.sinfo_update.connect ((sinfo) => {
        signal_called = true;
        assert (sinfo.name == "game2xs.com Counter-Strike Source #1");
        assert (sinfo.map == "de_dust");
        assert (sinfo["version"] == "1.0.0.22");
      });

      uint8[] data = {
        0xFF, 0xFF, 0xFF, 0xFF, 0x49, 0x02, 0x67, 0x61, 0x6D, 0x65, 0x32, 0x78, 0x73, 0x2E, 0x63, 0x6F, // ÿÿÿÿI.game2xs.co
        0x6D, 0x20, 0x43, 0x6F, 0x75, 0x6E, 0x74, 0x65, 0x72, 0x2D, 0x53, 0x74, 0x72, 0x69, 0x6B, 0x65, // m Counter-Strike
        0x20, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x23, 0x31, 0x00, 0x64, 0x65, 0x5F, 0x64, 0x75, //  Source #1.de_du
        0x73, 0x74, 0x00, 0x63, 0x73, 0x74, 0x72, 0x69, 0x6B, 0x65, 0x00, 0x43, 0x6F, 0x75, 0x6E, 0x74, // st.cstrike.Count
        0x65, 0x72, 0x2D, 0x53, 0x74, 0x72, 0x69, 0x6B, 0x65, 0x3A, 0x20, 0x53, 0x6F, 0x75, 0x72, 0x63, // er-Strike: Sourc
        0x65, 0x00, 0xF0, 0x00, 0x05, 0x10, 0x04, 0x64, 0x6C, 0x00, 0x00, 0x31, 0x2E, 0x30, 0x2E, 0x30, // e......dl..1.0.0
        0x2E, 0x32, 0x32, 0x00                                                                          // .22.
      };

      proto.process_response (data);

      assert (signal_called == true);
    } catch (Error err) {
      assert_no_error (err);
    }
  });

  Test.run ();
	return 0;
}
