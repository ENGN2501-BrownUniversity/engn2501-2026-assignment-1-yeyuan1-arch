//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2026-01-26 17:42:17 taubin>
//------------------------------------------------------------------------
//
// SaverStl.cpp
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

#include "SaverStl.hpp"

#include "wrl/Shape.hpp"
#include "wrl/Appearance.hpp"
#include "wrl/Material.hpp"
#include "wrl/IndexedFaceSet.hpp"

#include "core/Faces.hpp"

const char* SaverStl::_ext = "stl";
// IndexedFaceSet
// public:

// IndexedFaceSet();

// void            clear();
// bool&           getCcw();
// bool&           getConvex();
// float&          getCreaseangle();
// bool&           getSolid();
// bool&           getNormalPerVertex();
// bool&           getColorPerVertex();
// vector<float>&  getCoord();
// vector<int>&    getCoordIndex();
// vector<float>&  getNormal();
// vector<int>&    getNormalIndex();
// vector<float>&  getColor();
// vector<int>&    getColorIndex();
// vector<float>&  getTexCoord();
// vector<int>&    getTexCoordIndex();

// Shape
//  Node* getGeometry();

//  SceneGraph
//   Node* find(const string& name);
//////////////////////////////////////////////////////////////////////
bool SaverStl::save(const char* filename, SceneGraph& wrl) const {
    bool success = false;
    if(filename!=(char*)0) {

        // Check these conditions

        // 1) the SceneGraph should have a single child
        if (wrl.getNumberOfChildren() != 1) {
            return false;
        }

        // 2) the child should be a Shape node
        Node* child = (Node*)(wrl.getChildren()[0]);
        Shape* shape = dynamic_cast<Shape*>(child);
        if (shape == nullptr) {
            return false;
        }

        // 3) the geometry of the Shape node should be an IndexedFaceSet node
        // - construct an instance of the Faces class from the IndexedFaceSet
        // - remember to delete it when you are done with it (if necessary)
        //   before returning
        Node* geom = shape->getGeometry();
        IndexedFaceSet* ifs = dynamic_cast<IndexedFaceSet*>(geom);
        if (ifs == nullptr) {
            return false;
        }

        int iV = ifs->getNumberOfCoord();
        const std::vector<int>& iVIdx = ifs->getCoordIndex();
        Faces* faces = new Faces(iV, iVIdx);

        // 4) the IndexedFaceSet should be a triangle mesh
        bool isTriangleMesh = true;
        int nF = faces->getNumberOfFaces();
        for (int iF = 0; iF < nF; ++iF) {
            if (faces->getFaceSize(iF) != 3) {
                isTriangleMesh = false;
                break;
            }
        }

        if (!isTriangleMesh) {
            delete faces;
            return false;
        }

        // 5) the IndexedFaceSet should have normals per face
        IndexedFaceSet::Binding normalBinding = ifs->getNormalBinding();
        bool hasNormalPerFace = (normalBinding == IndexedFaceSet::PB_PER_FACE ||
                                 normalBinding == IndexedFaceSet::PB_PER_FACE_INDEXED);

        // if (all the conditions are satisfied) {
        if (hasNormalPerFace && !ifs->getNormal().empty()) {
            FILE* fp = fopen(filename, "w");
            if (fp != (FILE*)0) {

                // if set, use ifs->getName()
                // otherwise use filename,
                // but first remove directory and extension
                std::string solidName = "DefaultName";
                if (!ifs->getName().empty()) {
                    solidName = ifs->getName();
                } else {
                    std::string path(filename);
                    size_t lastSlash = path.find_last_of("/\\");
                    std::string base = (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);
                    size_t lastDot = base.find_last_of('.');
                    solidName = (lastDot == std::string::npos) ? base : base.substr(0, lastDot);
                }

                fprintf(fp, "solid %s\n", solidName.c_str());

                const std::vector<float>& N = ifs->getNormal();
                const std::vector<float>& C = ifs->getCoord();

                // TODO ...
                // for each face {
                for (int iF = 0; iF < nF; iF++) {
                    int nIdx = iF;
                    if (normalBinding == IndexedFaceSet::PB_PER_FACE_INDEXED) {
                        const std::vector<int>& NI = ifs->getNormalIndex();
                        int firstCorner = faces->getFaceFirstCorner(iF);
                        if (firstCorner >= 0 && firstCorner < (int)NI.size()) {
                            nIdx = NI[firstCorner];
                        }
                    }

                    float nx = 0.0f, ny = 0.0f, nz = 0.0f;
                    if (nIdx >= 0 && (3 * nIdx + 2) < (int)N.size()) {
                        nx = N[3 * nIdx + 0];
                        ny = N[3 * nIdx + 1];
                        nz = N[3 * nIdx + 2];
                    }

                    fprintf(fp, "facet normal %g %g %g\n", nx, ny, nz);
                    fprintf(fp, "outer loop\n");
                    for (int iV_local = 0; iV_local < 3; ++iV_local) {
                        int iV_global = faces->getFaceVertex(iF, iV_local);
                        float vx = 0.0f, vy = 0.0f, vz = 0.0f;
                        if (iV_global >= 0 && (int)C.size() >= 3 * (iV_global + 1)) {
                            vx = C[3 * iV_global + 0];
                            vy = C[3 * iV_global + 1];
                            vz = C[3 * iV_global + 2];
                        }
                        fprintf(fp, "    vertex %e %e %e\n", vx, vy, vz);
                    }

                    fprintf(fp, "    endloop\n");
                    fprintf(fp, "  endfacet\n");
                }
                // }
                fprintf(fp, "endsolid %s\n", solidName.c_str());
                fclose(fp);
                success = true;
            }
        } else {
            delete faces;
            return false;
        }

        delete faces;
        // } endif (all the conditions are satisfied)
        }
    return success;
}
