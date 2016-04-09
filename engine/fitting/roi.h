/*******************************************************************************
 *
 * This software was developed at the National Institute of Standards and
 * Technology (NIST) by employees of the Federal Government in the course
 * of their official duties. Pursuant to title 17 Section 105 of the
 * United States Code, this software is not subject to copyright protection
 * and is in the public domain. NIST assumes no responsibility whatsoever for
 * its use by other parties, and makes no guarantees, expressed or implied,
 * about its quality, reliability, or any other characteristic.
 *
 * This software can be redistributed and/or modified freely provided that
 * any derivative works bear some notice that they are derived from it, and
 * any modified versions bear some notice that they have been modified.
 *
 * Author(s):
 *      Martin Shetty (NIST)
 *
 * Description:
 *      Qpx::ROI
 *
 ******************************************************************************/

#ifndef QPX_ROI_H
#define QPX_ROI_H

#include "gamma_peak.h"
#include "polynomial.h"
#include "finder.h"
#include <boost/atomic.hpp>

namespace Qpx {

struct Fit {
  Fit (Finder f) :
    finder_(f)
  {}

  Finder finder_;
  std::map<double, Peak> peaks_;
  SUM4Edge  LB_, RB_;
  PolyBounded background_;
  PolyBounded sum4_background_;
};


struct ROI {
  ROI(FitSettings fs = FitSettings())
    : settings_(fs)
  {
    finder_.settings_ = settings_;
  }

  void set_data(const std::vector<double> &x, const std::vector<double> &y,
                double min, double max);
  void auto_fit(boost::atomic<bool>& interruptor);

  bool contains(double bin);
  bool overlaps(double bin);
  bool overlaps(double Lbin, double Rbin);

  //  void add_peaks(const std::set<Peak> &pks, const std::vector<double> &x, const std::vector<double> &y);
  void add_peak(const std::vector<double> &x, const std::vector<double> &y, double L, double R, boost::atomic<bool>& interruptor);
  void adjust_bounds(const std::vector<double> &x, const std::vector<double> &y, uint16_t L, uint16_t R, boost::atomic<bool>& interruptor);

  bool add_from_resid(int32_t centroid_hint = -1);

  bool remove_peaks(const std::set<double> &pks);

  void iterative_fit(boost::atomic<bool>& interruptor);

  bool rollback(int i);

  SUM4Edge LB() const {return LB_;}
  SUM4Edge RB() const {return RB_;}
  void set_LB(SUM4Edge);
  void set_RB(SUM4Edge);

  void rebuild();
  void render();
  void save_current_fit();

  //intrinsic
  FitSettings settings_;

  std::vector<Fit> fits_;
//  Fit current_fit_;

  //per_fit
  Finder finder_;
  std::map<double, Peak> peaks_;

  PolyBounded background_;
  PolyBounded sum4_background_;

  //rendition
  std::vector<double> hr_x, hr_x_nrg,
                      hr_background, hr_back_steps,
                      hr_fullfit, hr_sum4_background_;

private:
  SUM4Edge LB_, RB_;

  std::vector<double> remove_background();
  void init_background();
  bool remove_peak(double bin);
  void make_sum4_background();

};


}

#endif
