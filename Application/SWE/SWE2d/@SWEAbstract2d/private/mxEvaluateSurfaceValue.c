#include "mex.h"
#include "mxSWE2d.h"
#include <math.h>

#ifdef _OPENMP
#include <omp.h>
#endif

typedef enum {
  NdgEdgeInner = 0,
  NdgEdgeGaussEdge = 1,
  NdgEdgeSlipWall = 2,
  NdgEdgeNonSlipWall = 3,
  NdgEdgeZeroGrad = 4,
  NdgEdgeClamped = 5,
  NdgEdgeClampedDepth = 6,
  NdgEdgeClampedVel = 7,
  NdgEdgeFlather = 8,
  NdgEdgeNonLinearFlather = 9,
  NdgEdgeNonLinearFlatherFlow = 10,
  NdgEdgeNonReflectingFlux = 11
} NdgEdgeType;

typedef struct {
  double hM;   ///< water depth at local node
  double hP;   ///< water depth at next node
  double huM;  ///< flux at local node
  double huP;  ///< flux at next node
  double hvM;  ///< flux at local node
  double hvP;  ///< flux at next node
  double zM;   ///< bottom elevation
  double zP;   ///< bottom elevation
} SurfNodeField;

/** \brief Impose boundary condition to the next node */
void imposeBoundaryCondition(const double gra,        ///< gravity acceleration
                             NdgEdgeType type,        ///< boundary node type
                             const double nx,         ///< outward normal vector
                             const double ny,         ///< outward normal vector
                             const int idM,           ///< local node index
                             const int idP,           ///< next node index
                             const PhysField *fphys,  ///< physical field
                             const PhysField *fext,   ///< external field
                             SurfNodeField *surf) {
  // assign the local node values
  surf->hM = fphys->h[idM];
  surf->huM = fphys->hu[idM];
  surf->hvM = fphys->hv[idM];
  surf->zM = fphys->z[idM];

  // get next node values
  surf->zP = fphys->z[idP];
  if (type == NdgEdgeInner) {
    surf->hP = fphys->h[idP];
    surf->huP = fphys->hu[idP];
    surf->hvP = fphys->hv[idP];
  } else if (type == NdgEdgeZeroGrad) {
    surf->hP = fphys->h[idM];
    surf->huP = fphys->hu[idM];
    surf->hvP = fphys->hv[idM];
  } else if (type == NdgEdgeClamped) {
    surf->hP = fext->h[idM];
    surf->huP = fext->hu[idM];
    surf->hvP = fext->hv[idM];
  } else if (type == NdgEdgeClampedDepth) {
    surf->hP = fext->h[idM];
    surf->huP = fphys->hu[idM];
    surf->hvP = fphys->hv[idM];
  } else if (type == NdgEdgeClampedVel) {
    surf->hP = fphys->h[idM];
    surf->huP = fext->hu[idM];
    surf->hvP = fext->hv[idM];
  } else if (type == NdgEdgeSlipWall) {
    const double qxM = fphys->hu[idM];
    const double qyM = fphys->hv[idM];
    double qnM = qxM * nx + qyM * ny;   // outward normal flux
    double qvM = -qxM * ny + qyM * nx;  // outward tangential flux
    // adjacent value
    surf->hP = fphys->h[idM];
    surf->huP = (-qnM) * nx - qvM * ny;
    surf->hvP = (-qnM) * ny + qvM * nx;
  } else if (type == NdgEdgeNonSlipWall) {
    surf->hP = fphys->h[idM];
    surf->huP = -fphys->hu[idM];
    surf->hvP = -fphys->hv[idM];
  } else if (type == NdgEdgeFlather) {
    const double hE = fext->h[idM];
    const double uE = fext->hu[idM] / hE;
    const double vE = fext->hv[idM] / hE;
    const double unE = uE * nx + vE * ny;   // outward normal flux
    const double uvE = -uE * ny + vE * nx;  // tangential flux
    const double un = unE - sqrt(-gra / surf->zM) * (hE - surf->hM);
    const double uv = uvE;
    surf->hP = surf->hM;
    surf->huP = (un * nx - uv * ny) * surf->hM;
    surf->hvP = (un * ny + uv * nx) * surf->hM;
  } else if (type == NdgEdgeNonLinearFlather) {
    const double hE = fext->h[idM];
    const double uE = fext->hu[idM] / hE;
    const double vE = fext->hv[idM] / hE;
    const double unE = uE * nx + vE * ny;  // outward normal flux
    const double un = surf->huM / surf->hM * nx + surf->hvM / surf->hM * ny;
    const double temp = 0.5 * (un - unE) + sqrt(gra * hE);
    surf->hP = temp * temp / gra;
    surf->huP = fext->hu[idM];
    surf->hvP = fext->hv[idM];
  } else if (type == NdgEdgeNonLinearFlatherFlow) {
    const double hE = fext->h[idM];
    const double uE = fext->hu[idM] / hE;
    const double vE = fext->hv[idM] / hE;
    const double unE = uE * nx + vE * ny;   // outward normal flux
    const double uvE = -uE * ny + vE * nx;  // tangential flux
    const double un = unE - 2 * sqrt(gra * hE) + 2 * sqrt(gra * surf->hM);
    const double uv = uvE;
    surf->hP = surf->hM;
    surf->huP = (un * nx - uv * ny) * surf->hM;
    surf->hvP = (un * ny + uv * nx) * surf->hM;
  } else if (type == NdgEdgeNonReflectingFlux) {
    const double unM = surf->huM / surf->hM * nx + surf->hvM / surf->hM * ny;
    const double RLP = unM + 2 * sqrt(gra * surf->hM);
    const double hs = fext->h[idM];
    surf->hP = hs;
    const double RRM = RLP - 4 * sqrt(gra * hs);
    const double un = 0.5 * (RRM + RLP);
    const double uvM = -surf->huM / surf->hM * ny + surf->hvM / surf->hM * nx;
    surf->huP = (un * nx - uvM * ny) * surf->hM;
    surf->hvP = (un * ny + uvM * nx) * surf->hM;
  } else {
    mexPrintf("Matlab:%s:Unknown boundary type: %d\n", __FILE__, type);
  }
  return;
}

/**
 * \brief Implement the hydraostatic reconstruction
 * \details Details about the HR method can be found in Hou et. al. (2013)
 */
void evaluateHydrostaticReconstructValue(
    const double hmin,   ///< water depth threshold
    SurfNodeField *surf  ///< surface node values
) {
  double zstar = max(surf->zM, surf->zP);
  double um, vm, up, vp;
  evaluateFlowRateByDeptheThreshold(hmin, surf->hM, surf->huM, surf->hvM, &um,
                                    &vm);
  evaluateFlowRateByDeptheThreshold(hmin, surf->hP, surf->huP, surf->hvP, &up,
                                    &vp);
  const double etaM = surf->hM + surf->zM;
  const double etaP = surf->hP + surf->zP;
  zstar = min(etaM, zstar);  // z* = min( \eta^-, z* )
  surf->hM = etaM - zstar;
  surf->hP = max(0, etaP - zstar) - max(0, surf->zP - zstar);
  surf->huM = surf->hM * um;
  surf->huP = surf->hP * up;
  surf->hvM = surf->hM * vm;
  surf->hvP = surf->hP * vp;
  return;
}

#define NRHS 11
#define NLHS 2
#define NVAR 3

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {
  /* check input & output */
  if (nrhs != NRHS) {
    mexPrintf("Matlab:%s:InvalidNumberInput,\n", __FILE__);
    mexPrintf("%d inputs required.\n", NRHS);
  }

  if (nlhs != NLHS) {
    mexPrintf("Matlab:%s:InvalidNumberOutput,\n", __FILE__);
    mexPrintf("%d inputs required.\n", NLHS);
  }

  double hcrit = mxGetScalar(prhs[0]);
  double gra = mxGetScalar(prhs[1]);
  double *eidM = mxGetPr(prhs[2]);
  double *eidP = mxGetPr(prhs[3]);
  double *nx = mxGetPr(prhs[4]);
  double *ny = mxGetPr(prhs[5]);
  signed char *eidtype = (signed char *)mxGetData(prhs[6]);
  // double* fphys = mxGetPr(prhs[7]);
  // double* fext = mxGetPr(prhs[8]);
  double *etoe = mxGetPr(prhs[9]);
  signed char *regType = (signed char *)mxGetData(prhs[10]);

  PhysField fphys = convertMexToPhysField(prhs[7]);
  PhysField fext = convertMexToPhysField(prhs[8]);

  // const size_t Np = fphys.Np;
  const size_t K = fphys.K;
  // const size_t Nfield = fphys.Nfield;

  const size_t TNfp = mxGetM(prhs[2]);
  const size_t ndimOut = 3;
  const mwSize dimOut[3] = {TNfp, K, NVAR};
  plhs[0] = mxCreateNumericArray(ndimOut, dimOut, mxDOUBLE_CLASS, mxREAL);
  plhs[1] = mxCreateNumericArray(ndimOut, dimOut, mxDOUBLE_CLASS, mxREAL);
  const size_t Nface = mxGetM(prhs[9]);
  const size_t Nfp = TNfp / Nface;

  PhysField fM = convertMexToPhysField(plhs[0]);
  PhysField fP = convertMexToPhysField(plhs[1]);

#ifdef _OPENMP
#pragma omp parallel for num_threads(DG_THREADS)
#endif
  for (int k = 0; k < K; k++) {
    SurfNodeField surf;
    for (int f = 0; f < Nface; f++) {
      const int ind = k * Nface + f;
      const int eid = (int)etoe[ind] - 1;
      NdgEdgeType localCellType = (NdgRegionType)regType[k];
      NdgEdgeType adjcentCellType = (NdgRegionType)regType[eid];

      if ((localCellType == NdgRegionDry) & (adjcentCellType == NdgRegionDry))
        continue;

      for (int n = 0; n < Nfp; n++) {
        const size_t sk = k * TNfp + f * Nfp + n;
        NdgEdgeType type = (NdgEdgeType)eidtype[sk];
        if (type == NdgEdgeGaussEdge) {
          continue;
        }
        const int im = (int)eidM[sk] - 1;
        const int ip = (int)eidP[sk] - 1;

        const double nx_ = nx[sk];
        const double ny_ = ny[sk];
        imposeBoundaryCondition(gra, type, nx_, ny_, im, ip, &fphys, &fext,
                                &surf);
        evaluateHydrostaticReconstructValue(hcrit, &surf);

        fM.h[sk] = surf.hM;
        fM.hu[sk] = surf.huM;
        fM.hv[sk] = surf.hvM;
        fP.h[sk] = surf.hP;
        fP.hu[sk] = surf.huP;
        fP.hv[sk] = surf.hvP;
      }
    }
  }
  return;
}