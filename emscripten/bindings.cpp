#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS(constant_bindings) {

  function("setup", &setup);
  function("_readNFTMarker", &readNFTMarker);

  value_object<nftMarker>("nftMarker")
  .field("width", &nftMarker::widthNFT)
  .field("height", &nftMarker::heightNFT)
  .field("dpi", &nftMarker::dpiNFT)
  .field("numFsets", &nftMarker::numFsets)
  .field("numFpoints", &nftMarker::numFpoints)
  .field("imgBWsize", &nftMarker::imgBWsize)
  .field("pointer", &nftMarker::pointer);
}
