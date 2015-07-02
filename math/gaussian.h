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
 *      Gaussian
 *
 ******************************************************************************/

#ifndef GAUSSIAN_H
#define GAUSSIAN_H

#include <vector>
#include <iostream>
#include <numeric>


class Gaussian {
public:
  Gaussian() : height_(0), hwhm_(0), center_(0), rsq(-1) {}
  Gaussian(const std::vector<double> &x, const std::vector<double> &y);

  double evaluate(double x);
  std::vector<double> evaluate_array(std::vector<double> x);
  
  double height_, hwhm_, center_;
  double rsq;
};


class SplitGaussian {
public:
  SplitGaussian() : height_(0), center_(0), hwhm_l(0), hwhm_r(0), rsq(-1) {}
  SplitGaussian(const std::vector<double> &x, const std::vector<double> &y);

  double evaluate(double x);
  std::vector<double> evaluate_array(std::vector<double> x);
  
  double height_, hwhm_, center_, hwhm_l, hwhm_r;
  double rsq;
};

#endif
