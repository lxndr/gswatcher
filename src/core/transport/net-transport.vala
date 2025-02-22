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

namespace Gsw {

public abstract class NetTransport : Transport {
  public string host { get; construct; }
  public uint16 port { get; construct; }
  public InetSocketAddress? saddr { get; private set; }

  protected Cancellable cancellable = new Cancellable ();

  ~NetTransport () {
    if (cancellable != null && !cancellable.is_cancelled ())
      cancellable.cancel ();
  }

  public bool is_resolved {
    get {
      return saddr != null;
    }
  }

  protected async void resolve () throws Error {
    var resolver = Resolver.get_default ();
    var addresses = yield resolver.lookup_by_name_with_flags_async (host, IPV4_ONLY, cancellable);
  
    if (addresses.length () == 0)
      throw new IOError.NOT_FOUND ("no address found"); // TODO: handle

    var address = addresses.nth_data (0);
    saddr = new InetSocketAddress (address, port);
    resolved (saddr.address.to_string ());
  }
}

}
