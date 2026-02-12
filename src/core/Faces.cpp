//------------------------------------------------------------------------
//  Copyright (C) Gabriel Taubin
//  Time-stamp: <2026-01-26 17:42:17 taubin>
//------------------------------------------------------------------------
//
// Faces.cpp
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

#include <math.h>
#include "Faces.hpp"

Faces::Faces(const int nV, const vector<int>& coordIndex) {
    _nV = nV;
    _coordIndex = coordIndex;

    if (nV > 0 && _coordIndex.size() > 0) {
        _faceStart.push_back(0);

        for (int i = 0; i < (int)_coordIndex.size(); ++i) {
            if (_coordIndex[i] == -1) {
                if (i + 1 < (int)_coordIndex.size()) {
                    _faceStart.push_back(i + 1); // store the first vertex index of face
                }
            } else {
                if (_coordIndex[i] < 0 || _coordIndex[i] >= _nV) {
                    _nV = _coordIndex[i] + 1;
                }
            }
        }
    }
}

int Faces::getNumberOfVertices() const {
    return _nV;
}

int Faces::getNumberOfFaces() const {
    return (int)_faceStart.size();
}

int Faces::getNumberOfCorners() const {
    return (int)_coordIndex.size();
}

int Faces::getFaceSize(const int iF) const {
    if (iF < 0 || iF >= (int)_faceStart.size()) return 0;

    int start = _faceStart[iF];
    int size = 0;
    while (start + size < (int)_coordIndex.size() && _coordIndex[start + size] != -1) {
        size++;
    }
    return size;
}

int Faces::getFaceFirstCorner(const int iF) const {
    if (iF < 0 || iF >= (int)_faceStart.size()) return -1;
    return _faceStart[iF];
}

int Faces::getFaceVertex(const int iF, const int j) const {
    int start = getFaceFirstCorner(iF);
    if (start == -1 || j < 0 || j >= getFaceSize(iF)) return -1;
    return _coordIndex[start + j];
}

int Faces::getCornerFace(const int iC) const {
    if (iC < 0 || iC >= (int)_coordIndex.size() || _coordIndex[iC] == -1) {
        return -1;
    }

    for (int i = 0; i < (int)_faceStart.size(); ++i) {
        int start = _faceStart[i];
        int nextStart = (i + 1 < (int)_faceStart.size()) ? _faceStart[i + 1] : (int)_coordIndex.size();

        if (iC >= start && iC < nextStart - 1) {
            return i;
        }
    }
    return -1;
}

int Faces::getNextCorner(const int iC) const {
    if (iC < 0 || iC >= (int)_coordIndex.size() || _coordIndex[iC] == -1) return -1;
    if (iC + 1 >= (int)_coordIndex.size() || _coordIndex[iC + 1] == -1) {
        return getFaceFirstCorner(getCornerFace(iC));
    }
    return iC + 1;
}
