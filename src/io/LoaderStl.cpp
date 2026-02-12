//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2026-01-26 17:42:17 taubin>
//------------------------------------------------------------------------
//
// LoaderStl.cpp
//
// Written by: <Your Name>
//
// Software developed for the course
// Digital Geometry Processing
// Copyright (c) 2026, Gabriel Taubin
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of the Brown University nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL GABRIEL TAUBIN BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <stdio.h>
#include <stdlib.h> // for atof
#include "TokenizerFile.hpp"
#include "LoaderStl.hpp"
#include "StrException.hpp"

#include "wrl/Shape.hpp"
#include "wrl/Appearance.hpp"
#include "wrl/Material.hpp"
#include "wrl/IndexedFaceSet.hpp"

const char* LoaderStl::_ext = "stl";

bool LoaderStl::load(const char* filename, SceneGraph& wrl) {
    bool success = false;

    // clear the scene graph
    wrl.clear();
    wrl.setUrl("");

    FILE* fp = (FILE*)0;
    try {

        // open the file
        if(filename==(char*)0) throw new StrException("filename==null");
        fp = fopen(filename,"r");
        if(fp==(FILE*)0) throw new StrException("fp==(FILE*)0");

        // use the io/Tokenizer class to parse the input ascii file
        TokenizerFile tkn(fp);

        // first token should be "solid"
        if(tkn.expecting("solid") && tkn.get()) {
            string stlName = tkn; // second token should be the solid name

            // create the scene graph structure :
            // 1) the SceneGraph should have a single Shape node a child
            Shape* shape = new Shape();
            wrl.addChild(shape);

            // 2) the Shape node should have an Appearance node in its appearance field
            Appearance* app = new Appearance();
            shape->setAppearance(app);

            // 3) the Appearance node should have a Material node in its material field
            app->setMaterial(new Material());

            // 4) the Shape node should have an IndexedFaceSet node in its geometry node
            IndexedFaceSet* ifs = new IndexedFaceSet();
            shape->setGeometry(ifs);

            // from the IndexedFaceSet
            // 5) get references to the coordIndex, coord, and normal arrays
            vector<float>& coord = ifs->getCoord();
            vector<int>& coordIndex = ifs->getCoordIndex();
            vector<float>& normal = ifs->getNormal();

            // 6) set the normalPerVertex variable to false (i.e., normals per face)
            ifs->getNormalPerVertex() = false;

            // - run an infinite loop to parse all the faces
            while(tkn.get()) {
                string token = tkn;
                if (token == "endsolid") {
                    success = true;
                    break;
                }

                if (token == "facet") {
                    // call the private method
                    if (!_loadFace(tkn, coord, normal, coordIndex)) {
                        throw new StrException("Error parsing facet data.");
                    }
                }
            }
        }

        fclose(fp);

    } catch(StrException* e) {
        if(fp!=(FILE*)0) fclose(fp);
        fprintf(stderr,"ERROR | %s\n",e->what());
        delete e;
    }

    return success;
}

// Fixed: The function signature now matches the HPP and handles tokens correctly
bool LoaderStl::_loadFace(TokenizerFile& tkn, vector<float>& coord, vector<float>& normal, vector<int>& coordIndex) {

    // 1. facet normal ni nj nk
    if (!tkn.expecting("normal")) return false;
    for (int i = 0; i < 3; ++i) {
        if (!tkn.get()) return false;
        // CAST tkn to string to access value, then call .c_str()
        normal.push_back((float)atof(((string)tkn).c_str()));
    }

    // 2. outer loop
    // Note: expecting() usually consumes the token if it matches
    if (!tkn.expecting("outer")) return false;
    if (!tkn.expecting("loop")) return false;

    // 3. vertex v1x v1y v1z ...
    int startIndex = (int)coord.size() / 3;
    for (int i = 0; i < 3; ++i) {
        if (!tkn.expecting("vertex")) return false;
        for (int j = 0; j < 3; ++j) {
            if (!tkn.get()) return false;
            coord.push_back((float)atof(((string)tkn).c_str()));
        }
        coordIndex.push_back(startIndex + i);
    }
    coordIndex.push_back(-1); // Facet delimiter

    // 4. endloop and endfacet
    if (!tkn.expecting("endloop")) return false;
    if (!tkn.expecting("endfacet")) return false;

    return true;
}
