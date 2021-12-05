namespace Gsw.Ui {

string? get_querier_ip_address (Querier? querier) {
  if (querier == null)
    return null;

  var detector_querier = (DetectorQuerier) querier;
  var detected_querier = (WorkerQuerier) detector_querier.detected_querier;

  if (detected_querier == null)
    return null;

  return detected_querier.transport.saddr.address.to_string ();
}

}
