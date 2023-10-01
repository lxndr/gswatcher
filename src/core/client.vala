namespace Gsw {

private static Gtk.TextTagTable console_log_buffer_tag_table;

public class Client : Object {
  private QuerierManager querier_manager = QuerierManager.get_instance ();
  private Gee.List<Querier> tmp_queriers = new Gee.ArrayList<Querier> ();
  private Querier? querier;

  public Server server { get; construct set; }
  public string? ip_address { get; protected set; }
  public Error? error { get; protected set; }
  public int64 ping { get; protected set; default = -1; }
  public ServerDetailsList details { get; construct set; }
  public ServerInfo sinfo { get; construct set; }
  public PlayerList plist { get; construct set; }
  public ListStore? plist_fields { get; construct set; }
  public bool is_console_supported { get; private set; }
  public ConsoleClient? console_client { get; private set; }
  public Gtk.TextBuffer? console_log_buffer { get; private set; }

  class construct {
    console_log_buffer_tag_table = new Gtk.TextTagTable ();

    var inf_tag = new Gtk.TextTag ("inf");
    inf_tag.foreground = "dark gray";
    console_log_buffer_tag_table.add (inf_tag);

    var cmd_tag = new Gtk.TextTag ("cmd");
    cmd_tag.foreground = "dark green";
    console_log_buffer_tag_table.add (cmd_tag);

    var err_tag = new Gtk.TextTag ("err");
    err_tag.foreground = "dark red";
    console_log_buffer_tag_table.add (err_tag);
  }

  public Client (Server server) {
    Object (server : server);
  }

  ~Client () {
    tmp_queriers.clear ();
  }

  construct {
    details = new ServerDetailsList ();
    sinfo = new ServerInfo ();
    plist = new PlayerList ();
    plist_fields = new ListStore (typeof (PlayerField));

    if (server.protocol == null) {
      reset ();
    } else {
      try {
        querier = querier_manager.create_querier (server, server.protocol);
        on_game_detected (querier);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", server.protocol, err.message);
      }
    }
  }

  private void reset () {
    querier = null;
    tmp_queriers.clear ();

    foreach (var protocol_desc in ProtocolRegistry.get_instance ().list_by_feature (QUERY)) {
      try {
        var querier = querier_manager.create_querier (server, protocol_desc.id);
        querier.details_update.connect (on_details_update);
        querier.sinfo_update.connect (on_sinfo_update);
        querier.plist_fields_update.connect (on_plist_fields_updated);
        querier.plist_update.connect (on_plist_update);
        tmp_queriers.add (querier);
      } catch (Error err) {
        log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", protocol_desc.id, err.message);
      }
    }
  }

  private void on_details_update (Gee.Map<string, string> details) {
    this.details.apply (details);
  }

  private void on_sinfo_update (Querier querier, ServerInfo sinfo) {
    this.sinfo.game_id = sinfo.game_id;
    this.sinfo.game_name = sinfo.game_name;
    this.sinfo.game_mode = sinfo.game_mode;
    this.sinfo.server_name = sinfo.server_name;
    this.sinfo.server_type = sinfo.server_type;
    this.sinfo.server_os = sinfo.server_os;
    this.sinfo.game_version = sinfo.game_version;
    this.sinfo.map = sinfo.map;
    this.sinfo.num_players = sinfo.num_players;
    this.sinfo.max_players = sinfo.max_players;
    this.sinfo.private = sinfo.private;
    this.sinfo.secure = sinfo.secure;

    if (this.querier == null)
      on_game_detected (querier);
  }

  private void on_plist_fields_updated (Gee.List<PlayerField> plist_fields) {
    this.plist_fields.remove_all ();

    foreach (var field in plist_fields)
      this.plist_fields.append (field);
  }

  private void on_plist_update (Gee.ArrayList<Player> plist) {
    this.plist.apply (plist);
  }

  private void on_game_detected (Querier querier) {
    this.querier = querier;
    this.ip_address = querier.transport.saddr?.address.to_string ();
    querier.bind_property ("ping", this, "ping", DEFAULT | SYNC_CREATE);
    querier.bind_property ("error", this, "error", DEFAULT | SYNC_CREATE);
    tmp_queriers.clear ();

    var console_protocol = GameResolver.get_instance ().get_feature_protocol (sinfo.game_id, CONSOLE);
    is_console_supported = console_protocol != null;

    if (is_console_supported)
      console_log_buffer = new Gtk.TextBuffer (console_log_buffer_tag_table);
  }

  private void setup_console () {
    var console_protocol = GameResolver.get_instance ().get_feature_protocol (sinfo.game_id, CONSOLE);

    try {
      var proto = (ConsoleProtocol) ProtocolRegistry.get_instance ().create (console_protocol);
      console_client = new ConsoleClient (server, proto);
      console_client.connected.connect (on_console_connected);
      console_client.authenticated.connect (on_console_authenticated);
      console_client.disconnected.connect (on_console_disconnected);
      console_client.response_received.connect (on_console_response_received);
      console_client.error_occured.connect (on_console_error_occured);
    } catch (Error err) {
      log (Config.LOG_DOMAIN, LEVEL_ERROR, "failed to create protocol '%s': %s", console_protocol, err.message);
    }
  }

  private void on_console_connected () {
    log_to_console (_("Connected"), "inf");
  }

  private void on_console_disconnected () {
    log_to_console (_("Disonnected"), "inf");
    console_client = null;
  }

  private void on_console_authenticated () {
    log_to_console (_("Authenticated"), "inf");
  }

  private void on_console_response_received (string response) {
    log_to_console (response);
  }

  private void on_console_error_occured (Error err) {
    if (err is ConsoleError.AUTH_FAILED)
      log_to_console ("Authentication failed.", "err");
    else
      log_to_console (err.message, "err");

    console_client = null;
  }

  private void log_to_console (string msg, string? text_tag = null) {
    Gtk.TextIter iter;
    console_log_buffer.get_end_iter (out iter);
    console_log_buffer.insert_with_tags_by_name (ref iter, msg, -1, text_tag);
    console_log_buffer.insert (ref iter, "\n", -1);
  }

  public void send_console_command (string cmd) {
    if (console_client == null)
      setup_console ();

    log_to_console (cmd, "cmd");
    console_client.exec_command (cmd);
  }
}

}
