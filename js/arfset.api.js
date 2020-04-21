(function () {
  "use strict";
  var scope;
  if (typeof window !== "undefined") {
    scope = window;
  } else {
    scope = global;
  }

  var ARfset = function (width, height) {
    this.id = 0;
    this.nftMarkerCount = 0;
    this.imageSetWidth = 0;
    this.frameIbwpointer = null;
    this.pointer = null;
    this.imgBW = null;
    this.frameimgBWsize = null;

    if (typeof document !== "undefined") {
      this.canvas = document.createElement("canvas");
      this.canvas.width = width;
      this.canvas.height = height;
      this.ctx = this.canvas.getContext("2d");
    }
    this._init(width, height);
  };

  ARfset.prototype.display = function () {
    document.body.appendChild(this.canvas);
    var debugBuffer = new Uint8ClampedArray(
      Module.HEAPU8.buffer,
      this.pointer,
      this.frameimgBWsize
    );
    console.log(debugBuffer.length);
    console.log(debugBuffer.BYTES_PER_ELEMENT);
    var id = new ImageData(
      new Uint8ClampedArray(this.frameimgBWsize * 4),
      this.canvas.width,
      this.canvas.height
    );
    for (var i = 0, j = 0; i < debugBuffer.length; i++, j += 4) {
      var v = debugBuffer[i];
      id.data[j + 0] = v;
      id.data[j + 1] = v;
      id.data[j + 2] = v;
      id.data[j + 3] = 255;
    }

    this.ctx.putImageData(id, 0, 0);
    //Module._free(debugBuffer);
  };

  ARfset.prototype.loadNFTMarker = function (markerURL, onSuccess, onError) {
    var self = this;
    if (markerURL) {
      return arfset.readNFTMarker(
        this.id,
        markerURL,
        function (nftMarker) {
          console.log(nftMarker);
          self.pointer = nftMarker.pointer;
          self.frameimgBWsize = nftMarker.imgBWsize;
        },
        onError
      );
    } else {
      if (onError) {
        onError("Marker URL needs to be defined and not equal empty string!");
      } else {
        console.error(
          "Marker URL needs to be defined and not equal empty string!"
        );
      }
    }
  };

  ARfset.prototype.getImageSet = function () {
    return this.imageSet;
  };

  ARfset.prototype._init = function (width, height) {
    this.id = arfset.setup(width, height);
    var params = arfset.frameMalloc;
    this.frameIbwpointer = params.frameIbwpointer;
    this.frameimgBWsize = params.frameimgBWsize;
  };

  var markerCount = 0;

  function readNFTMarker(arId, url, callback, onError) {
    var mId = markerCount++;
    var prefix = "/markerNFT_" + mId;
    var filename1 = prefix + ".fset";
    var filename2 = prefix + ".iset";
    var filename3 = prefix + ".fset3";
    ajax(
      url + ".fset",
      filename1,
      function () {
        ajax(
          url + ".iset",
          filename2,
          function () {
            ajax(
              url + ".fset3",
              filename3,
              function () {
                var id = Module._readNFTMarker(arId, prefix);
                if (callback) callback(id);
              },
              function(errorNumber) {
                if (onError) onError(errorNumber);
              }
            );
          },
          function(errorNumber) {
            if (onError) onError(errorNumber);
          }
        );
      },
      function(errorNumber) {
        if (onError) onError(errorNumber);
      }
    );
  }

  function writeStringToFS(target, string, callback) {
    var byteArray = new Uint8Array(string.length);
    for (var i = 0; i < byteArray.length; i++) {
      byteArray[i] = string.charCodeAt(i) & 0xff;
    }
    writeByteArrayToFS(target, byteArray, callback);
  }

  function writeByteArrayToFS(target, byteArray, callback) {
    FS.writeFile(target, byteArray, { encoding: "binary" });
    // console.log('FS written', target);

    callback(byteArray);
  }

  function ajax(url, target, callback, errorCallback) {
    var oReq = new XMLHttpRequest();
    oReq.open("GET", url, true);
    oReq.responseType = "arraybuffer"; // blob arraybuffer

    oReq.onload = function () {
      if (this.status == 200) {
        // console.log('ajax done for ', url);
        var arrayBuffer = oReq.response;
        var byteArray = new Uint8Array(arrayBuffer);
        writeByteArrayToFS(target, byteArray, callback);
      } else {
        errorCallback(this.status);
      }
    };

    oReq.send();
  }

  var arfset = {
    readNFTMarker: readNFTMarker
  };

  var FUNCTIONS = ["setup", "display", "getImageSet"];

  function runWhenLoaded() {
    FUNCTIONS.forEach(function (n) {
      arfset[n] = Module[n];
    });

    for (var m in Module) {
      if (m.match(/^AR/)) arfset[m] = Module[m];
    }
  }

  scope.arfset = arfset;
  scope.ARfset = ARfset;

  if (scope.Module) {
    scope.Module.onRuntimeInitialized = function () {
      runWhenLoaded();
      var event = new Event("FeatureSETDisplay-loaded");
      scope.dispatchEvent(event);
    };
  } else {
    scope.Module = {
      onRuntimeInitialized: function () {
        runWhenLoaded();
      }
    };
  }
})();
