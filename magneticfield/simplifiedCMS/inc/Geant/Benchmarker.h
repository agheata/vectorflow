#include "iostream"
#include "MagField.h"
// #include "Geant/MagFieldCellVersion.h"
// #include "Geant/MagFieldVcGather.h"
// #include "Geant/MagFieldReorder.h"
// #include "Geant/MagFieldAutoVec.h"
#include "base/Vector3D.h"
#include "base/SOA3D.h"
#include "base/Global.h"
#include <string>
#include <vector>
#include <ctime>
#include <cmath> //for sqrt
#include <stdlib.h>
#include <Vc/Vc>
#include "backend/vc/Backend.h"
#include "backend/vcfloat/Backend.h"
#include "base/Vector.h"
#include "TRandom1.h"

using namespace std;
typedef vecgeom::Vector3D<float> ThreeVector; // normal Vector3D
typedef vecgeom::Vector3D<vecgeom::kVcFloat::precision_v> ThreeVecSimd_t;
typedef vecgeom::Vector<float> VcVectorFloat;

const float kRMax = 9000;
const float kZMax = 16000;

float RandR();
float RandZ();
void GenVecCartSubR(float &x, float &y);
void GenVecCart(ThreeVector &pos);
void GenVecCart(vecgeom::Vector<ThreeVector> &posVec, const int &n);
void CalculateMeanStDev(const vector<float> timeVec, float &mean, float &stDev);

template <class Backend>
void StoreInput(const vecgeom::Vector<ThreeVector> &posVec,
                vecgeom::Vector3D<typename Backend::precision_v> inputForVec[n], int n)
{
  cout << "\nScalar: " << endl;
  for (int i = 0; i < n; ++i) {
    inputForVec.push_back(posVec[i]);
  }
}

/*template <>
void StoreInput<vecgeom::kVcFloat>(const vecgeom::Vector<ThreeVector> &posVec,
                                            vecgeom::Vector3D<typename vecgeom::kVcFloat::precision_v> &inputForVec,
                                            int &n)
{
    cout<<"\nVector fields start: "<<endl;
    vecgeom::kVcFloat::precision_v vX;
    vecgeom::kVcFloat::precision_v vY;
    vecgeom::kVcFloat::precision_v vZ;

    int noOfFloats = 8;
    int inputVcLen = ceil(((float)n)/noOfFloats);

    int init = 0;

    for (int i = 0; i < n; i=i+noOfFloats){
       for (int j = 0; j < noOfFloats; ++j){
            vX[j]= posVec[i+j].x();
            vY[j]= posVec[i+j].y();
            vZ[j]= posVec[i+j].z();
        }
        ThreeVecSimd_t Pos;
        Pos[0] = vX;
        Pos[1] = vY;
        Pos[2] = vZ;

        inputForVec[init] = Pos;
        init++;
    }
}*/

// template <class Backend, class InputForVecType>
template <class Backend>
float Time(MagField &m1, const vecgeom::Vector<ThreeVector> &posVec, const int &n, const int &nRepetitions, bool flag)
{

  int noOfFloats = 1;
  float totTime  = 0.;
  int noRunsAvg  = 16;
  vector<float> timePerRepitition;
  int inputVcLen = ceil(((float)n) / noOfFloats);

  vecgeom::Vector3D<typename Backend::precision_v> xyzField, sumXYZField;

  typedef vecgeom::Vector3D<typename Backend::precision_v> InputForVecType;

  InputForVecType *inputForVec = new InputForVecType[n];
  // InputForVecType inputForVec;

  StoreInput<Backend>(posVec, inputForVec, n);

  /*
   {
      vector<ThreeVector> inputForVec;
      for (int i = 0; i < n; ++i)
      {
          inputForVec.push_back(posVec[i]);
      }
   }
  #else
      ThreeVecSimd_t *inputForVec = new ThreeVecSimd_t[inputVcLen];

      InputForVecType *inputForVec = new InputForVecType[];
  */

  for (int k = 0; k < noRunsAvg; ++k) {
    clock_t clock1 = clock();
    for (int j = 0; j < nRepetitions; ++j) {
      for (int i = 0; i < inputVcLen; ++i) {
        m1.GetFieldValue<Backend>(inputForVec[i], xyzField);
        sumXYZField += xyzField;
      }
    }
    clock1              = clock() - clock1;
    float clock1InFloat = ((float)clock1) / CLOCKS_PER_SEC;
    timePerRepitition.push_back(clock1InFloat / n / nRepetitions);
    totTime += clock1InFloat;
  }

  float timeMean, timeStDev;
  CalculateMeanStDev(timePerRepitition, timeMean, timeStDev);

  cout << sumXYZField << endl;

  // cout<<"Time per field value is : "<<clock1InFloat/(n*nRepetitions)*1e+9 << " ns "<<endl;
  // cout<<"totTime is: "<<totTime<<endl;
  cout << "Mean time is: " << timeMean * 1e+9 << "ns" << endl;
  // cout<<"Standard devi. is: "<<timeStDev*1e+9<<"ns"<<endl;
  return totTime / noRunsAvg;
}

float RandR()
{
  float r = (float)rand() / (RAND_MAX);
  r       = r * kRMax; // because r is in range (0,9000) mm
  return r;
}

float RandZ()
{
  float z  = (float)rand() / (RAND_MAX);
  z        = z * kZMax;  // range of z is between -16k and 16k
  int sign = rand() % 2; // to define the sign, since it can be both positive and negative
  if (sign == 0) {
    z = -z;
  }
  return z;
}

void GenVecCartSubR(float &x, float &y)
{
  x = RandR();
  y = RandR();
  if ((x * x + y * y) > kRMax * kRMax) {
    GenVecCartSubR(x, y);
  }
}

void GenVecCart(ThreeVector &pos)
{
  float x = 0, y = 0;
  float z = RandZ();
  GenVecCartSubR(x, y);
  pos.x() = x;
  pos.y() = y;
  pos.z() = z;
}

void GenVecCart(vecgeom::Vector<ThreeVector> &posVec, const int &n)
{
  for (int i = 0; i < n; ++i) {
    ThreeVector pos;
    GenVecCart(pos);
    posVec.push_back(pos);
  }
}

void CalculateMeanStDev(const vector<float> timeVec, float &mean, float &stDev)
{
  float sum   = std::accumulate(timeVec.begin(), timeVec.end(), 0.0);
  mean        = sum / timeVec.size();
  float sqSum = std::inner_product(timeVec.begin(), timeVec.end(), timeVec.begin(), 0.0);
  stDev       = std::sqrt(sqSum / timeVec.size() - mean * mean);
  // cout<<"Mean Time: "<<mean<<"ns"<<endl;
  // cout<<"Standard dev: "<<stDev<<"ns"<<endl;
}
