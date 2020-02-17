#include <stdio.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <emscripten.h>
#include <AR/ar.h>
#include <AR2/config.h>
#include <AR2/imageFormat.h>
#include <AR2/imageSet.h>
#include <AR2/featureSet.h>
#include <AR2/tracking.h>
#include <AR2/util.h>
#include <KPM/kpm.h>

#define PAGES_MAX               10          // Maximum number of pages expected. You can change this down (to save memory) or up (to accomodate more pages.)

struct arFset {
  int id;
  int width = 0;
  int height = 0;
  ARUint8 *videoFrame = NULL;
  ARUint8 *imgBW = NULL;
	int videoFrameSize;
  int imgBWsize;
  AR2ImageSetT *imageSet = NULL;
  AR2SurfaceSetT      *surfaceSet[PAGES_MAX];
  int width_NFT;
	int height_NFT;
	int dpi_NFT;
  int num_F_set_NFT; // number of Feature sets
  int num_F_points_NFT; // number of Feature points
  int surfaceSetCount = 0; // Running NFT marker id
};

std::unordered_map<int, arFset> arFsets;

static int ARFSET_NOT_FOUND = -1;
static int gARFsetID = 0;

extern "C" {

  int loadNFTMarker(arFset *arc, int surfaceSetCount, const char* datasetPathname) {
		int i, pageNo, numIset, width, height, dpi;

		// Load AR2 data.
		ARLOGi("Reading %s.fset\n", datasetPathname);

		if ((arc->surfaceSet[surfaceSetCount] = ar2ReadSurfaceSet(datasetPathname, "fset", NULL)) == NULL ) {
		    ARLOGe("Error reading data from %s.fset\n", datasetPathname);
		}

		numIset = arc->surfaceSet[surfaceSetCount]->surface[0].imageSet->num;
		arc->width_NFT = arc->surfaceSet[surfaceSetCount]->surface[0].imageSet->scale[0]->xsize;
		arc->height_NFT = arc->surfaceSet[surfaceSetCount]->surface[0].imageSet->scale[0]->ysize;
		arc->dpi_NFT = arc->surfaceSet[surfaceSetCount]->surface[0].imageSet->scale[0]->dpi;
    arc->num_F_set_NFT =  arc->surfaceSet[surfaceSetCount]->surface[0].featureSet[0].num;
    arc->num_F_points_NFT =  arc->surfaceSet[surfaceSetCount]->surface[0].featureSet[0].list[0].num;
    arc->imgBW = arc->surfaceSet[surfaceSetCount]->surface[0].imageSet->scale[0]->imgBW;

		ARLOGi("NFT num. of ImageSet: %i\n", numIset);
		ARLOGi("NFT marker width: %i\n", arc->width_NFT);
		ARLOGi("NFT marker height: %i\n", arc->height_NFT);
		ARLOGi("NFT marker dpi: %i\n", arc->dpi_NFT);
    ARLOGi("NFT Number of Feature sets: %i\n", arc->num_F_set_NFT);
    ARLOGi("NFT Num. of feature points: %d\n", arc->num_F_points_NFT);
    ARLOGi("imgBW filled\n");

		ARLOGi("  Done.\n");

	  if (surfaceSetCount == PAGES_MAX) exit(-1);

		ARLOGi("Loading of NFT data complete.\n");
		return (TRUE);
	}

  int readNFTMarker(int id, std::string datasetPathname) {
		if (arFsets.find(id) == arFsets.end()) { return -1; }
		arFset *arc = &(arFsets[id]);
    //ARUint8 * Imgbw;

		// Load marker(s).
		int patt_id = arc->surfaceSetCount;
		if (!loadNFTMarker(arc, patt_id, datasetPathname.c_str())) {
			ARLOGe("ARimageFsetDisplay(): Unable to read NFT marker.\n");
			return -1;
		} else {
      ARLOGi("Passing the imgBW pointer: %d\n", (int)arc->imgBW);
    }

		arc->surfaceSetCount++;
    arc->imgBWsize =  arc->width_NFT * arc->height_NFT * sizeof(ARUint8);
    ARLOGi("imgsizePointer: %d\n", arc->imgBWsize);

    /*EM_ASM_({
			if (!arfset["frameMalloc"]) {
				arfset["frameMalloc"] = ({});
			}
			var frameMalloc = arfset["frameMalloc"];
      //frameMalloc["frameIbwpointer"] = $1;
      frameMalloc["frameimgBWsize"] = $1;
		},
			0,
      //arc->imgBW,
      arc->imgBWsize
		);*/

		return (int)arc->imgBW;
	}

  int setup(int width, int height){
    int id = gARFsetID++;
		arFset *arc = &(arFsets[id]);
		arc->id = id;
    arc->width = width;
		arc->height = height;
    width = 893;
    height = 1117;
    arc->imgBWsize = width * height * sizeof(ARUint8);
    //arc->imgBW = (ARUint8*) malloc(arc->imgBWsize);

    ARLOGi("marker width %d\n", arc->width_NFT);
    ARLOGi("Allocated imgBWsize %d\n", arc->imgBWsize);

    EM_ASM_({
			if (!arfset["frameMalloc"]) {
				arfset["frameMalloc"] = ({});
			}
			var frameMalloc = arfset["frameMalloc"];
      frameMalloc["frameIbwpointer"] = $1;
      frameMalloc["frameimgBWsize"] = $2;
		},
			arc->id,
      arc->imgBW,
      arc->imgBWsize
		);

		return arc->id;
  }

}

#include "bindings.cpp"
