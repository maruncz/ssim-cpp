/*
 * The equivalent of Jeff Terrace's SSIM python code using OpenCV.
 * from https://github.com/jterrace/pyssim
 * The measure is described in :
 * "Image quality assessment: From error measurement to structural similarity"
 * C++ code by Jeff Terrace. https://github.com/jterrace/pyssim
 *
 * This implementation is under the public domain.
 * @see http://creativecommons.org/licenses/publicdomain/
 * The original work may be under copyrights.
 */

#include <algorithm>
#include <armadillo>
#include <opencv2/core.hpp>
#include <fstream>
#include <getopt.h>
#include <opencv2/highgui.hpp>
#include <iomanip>
#include <iostream>
#include <iterator>
//#include <omp.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/resource.h>
#include <tuple>
#include <vector>
using namespace std;
using namespace cv;
using namespace arma;

Scalar getMSSIM(cv::Mat img1_temp, cv::Mat img2_temp);

Scalar getMSSIM(cv::Mat img1_temp, cv::Mat img2_temp)
{

    double C1 = 6.5025, C2 = 58.5225, C3 = 0.00000681;

    cv::Mat kernel =
        (Mat_<float>(1, 11) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);
    cv::Mat kernel1 =
        (Mat_<float>(11, 1) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);

    Point anchor;
    anchor       = Point(-1, -1);
    double delta = 0;

    int x = img1_temp.rows, y = img1_temp.cols;
    // int nChan=img1_temp.channels()-2, d=CV_32FC1;

    cv::Mat img1_1   = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img2_2   = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img1_1_1 = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img2_2_2 = cv::Mat(Size(x, y), CV_32FC3);

    img1_temp.convertTo(img1_1, CV_32FC1);
    img2_temp.convertTo(img2_2, CV_32FC1);

    cvtColor(img1_1, img1_1_1, CV_BGR2GRAY, 1);
    cvtColor(img2_2, img2_2_2, CV_BGR2GRAY, 1);

    add(img1_1_1, Scalar(C3), img1_1_1);
    add(img2_2_2, Scalar(C3), img2_2_2);

    cv::Mat img1_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img2_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img1_img2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat img1_1_1_trans = cv::Mat(Size(y, x), CV_32FC1);
    cv::Mat img2_2_2_trans = cv::Mat(Size(y, x), CV_32FC1);

    transpose(img1_1_1, img1_1_1_trans);
    transpose(img2_2_2, img2_2_2_trans);
    arma::fmat imagine1_arma(img1_1_1_trans.ptr<float>(), img1_temp.rows,
                             img1_temp.cols, true, false);
    arma::fmat imagine2_arma(img2_2_2_trans.ptr<float>(), img2_temp.rows,
                             img2_temp.cols, true, false);
    arma::fmat imagine1_arma_floor = floor(imagine1_arma);
    arma::fmat imagine2_arma_floor = floor(imagine2_arma);
    std::cout << imagine2_arma << std::endl;
    std::cout << imagine2_arma_floor << std::endl;

    cv::Mat img1_from_arma(y, x, CV_32FC1, imagine1_arma_floor.memptr());
    cv::Mat img1(img1_from_arma.t());
    cv::Mat img2_from_arma(y, x, CV_32FC1, imagine2_arma_floor.memptr());
    cv::Mat img2(img2_from_arma.t());

    pow(img1, 2, img1_sq);
    pow(img2, 2, img2_sq);
    multiply(img1, img2, img1_img2, 1, CV_32FC1);

    cv::Mat mu1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat mu2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu1_sq(Size(x, y), CV_32FC1);
    cv::Mat mu2_sq(Size(x, y), CV_32FC1);
    cv::Mat mu1_mu2(Size(x, y), CV_32FC1);

    cv::Mat sigma1_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma1_sq_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12         = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat temp1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp2 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp3 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu1_inter     = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat mu2_inter     = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat ssim_map      = cv::Mat(Size(x, y), CV_32FC1);

    /*************************** END INITS **********************************/

    //////////////////////////////////////////////////////////////////////////
    // PRELIMINARY COMPUTING

    filter2D(img1, mu1_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT);
    filter2D(mu1_inter, mu1, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT);

    filter2D(img2, mu2_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT);
    filter2D(mu2_inter, mu2, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT);

    pow(mu1, 2, mu1_sq);
    pow(mu2, 2, mu2_sq);
    multiply(mu1, mu2, mu1_mu2, 1, CV_32FC1);

    filter2D(img1_sq, sigma1_sq_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma1_sq_inter, sigma1_sq, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq, CV_32FC1);

    filter2D(img2_sq, sigma2_sq_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma2_sq_inter, sigma2_sq, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq, CV_32FC1);

    filter2D(img1_img2, sigma12_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma12_inter, sigma12, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma12, 1, mu1_mu2, -1, 0, sigma12, CV_32FC1);

    //////////////////////////////////////////////////////////////////////////
    // FORMULA

    // (2*mu1_mu2 + C1)
    mu1_mu2.convertTo(temp1, CV_32FC1, 2);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (2*sigma12 + C2)
    sigma12.convertTo(temp2, CV_32FC1, 2);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    multiply(temp1, temp2, temp3, 1, CV_32FC1);

    // (mu1_sq + mu2_sq + C1)
    add(mu1_sq, mu2_sq, temp1, noArray(), CV_32FC1);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (sigma1_sq + sigma2_sq + C2)
    add(sigma1_sq, sigma2_sq, temp2, noArray(), CV_32FC1);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    multiply(temp1, temp2, temp1, 1, CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq
    // + sigma2_sq + C2))
    divide(temp3, temp1, ssim_map, 1, CV_32FC1);

    Scalar index_scalar = mean(ssim_map, noArray());
    return index_scalar;
}

std::tuple<cv::Scalar, cv::Mat, cv::Mat, cv::Mat, cv::Mat, cv::Mat, cv::Mat,
           cv::Mat>
getMSSIM1(cv::Scalar index_scalar, cv::Mat img1_temp, cv::Mat img2_temp,
          cv::Mat img1_extern, cv::Mat img1_sq_extern, cv::Mat mu1_extern,
          cv::Mat mu1_sq_extern, cv::Mat sigma1_sq_extern)
{

    double C1 = 6.5025, C2 = 58.5225, C3 = 0.00000681;

    cv::Mat kernel =
        (Mat_<float>(1, 11) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);
    cv::Mat kernel1 =
        (Mat_<float>(11, 1) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);

    Point anchor;
    anchor       = Point(-1, -1);
    double delta = 0;

    int x = img1_temp.rows, y = img1_temp.cols;
    // int nChan=img1_temp.channels()-2, d=CV_32FC1;

    cv::Mat img1_1   = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img2_2   = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img1_1_1 = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img2_2_2 = cv::Mat(Size(x, y), CV_32FC3);

    img1_temp.convertTo(img1_1, CV_32FC1);
    img2_temp.convertTo(img2_2, CV_32FC1);

    cvtColor(img1_1, img1_1_1, CV_BGR2GRAY, 1);
    cvtColor(img2_2, img2_2_2, CV_BGR2GRAY, 1);

    add(img1_1_1, Scalar(C3), img1_1_1);
    add(img2_2_2, Scalar(C3), img2_2_2);

    cv::Mat img1_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img2_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img1_img2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat img1_1_1_trans = cv::Mat(Size(y, x), CV_32FC1);
    cv::Mat img2_2_2_trans = cv::Mat(Size(y, x), CV_32FC1);

    transpose(img1_1_1, img1_1_1_trans);
    transpose(img2_2_2, img2_2_2_trans);
    arma::fmat imagine1_arma(img1_1_1_trans.ptr<float>(), img1_temp.rows,
                             img1_temp.cols, true, false);
    arma::fmat imagine2_arma(img2_2_2_trans.ptr<float>(), img2_temp.rows,
                             img2_temp.cols, true, false);
    arma::fmat imagine1_arma_floor = floor(imagine1_arma);
    arma::fmat imagine2_arma_floor = floor(imagine2_arma);

    cv::Mat img1_from_arma(y, x, CV_32FC1, imagine1_arma_floor.memptr());
    cv::Mat img1(img1_from_arma.t());
    cv::Mat img2_from_arma(y, x, CV_32FC1, imagine2_arma_floor.memptr());
    cv::Mat img2(img2_from_arma.t());
    img1_extern = img1;

    pow(img1, 2, img1_sq);
    img1_sq_extern = img1_sq;
    pow(img2, 2, img2_sq);
    multiply(img1, img2, img1_img2, 1, CV_32FC1);

    cv::Mat mu1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat mu2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu1_sq(Size(x, y), CV_32FC1);
    cv::Mat mu2_sq(Size(x, y), CV_32FC1);
    cv::Mat mu1_mu2(Size(x, y), CV_32FC1);

    cv::Mat sigma1_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma1_sq_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12         = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat temp1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp2 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp3 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu1_inter     = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat mu2_inter     = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat ssim_map      = cv::Mat(Size(x, y), CV_32FC1);

    /*************************** END INITS **********************************/

    //////////////////////////////////////////////////////////////////////////
    // PRELIMINARY COMPUTING

    filter2D(img1, mu1_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT);
    filter2D(mu1_inter, mu1, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT);
    mu1_extern = mu1;

    filter2D(img2, mu2_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT);
    filter2D(mu2_inter, mu2, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT);

    pow(mu1, 2, mu1_sq);
    mu1_sq_extern = mu1_sq;
    pow(mu2, 2, mu2_sq);
    multiply(mu1, mu2, mu1_mu2, 1, CV_32FC1);

    filter2D(img1_sq, sigma1_sq_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma1_sq_inter, sigma1_sq, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq, CV_32FC1);

    sigma1_sq_extern = sigma1_sq;

    filter2D(img2_sq, sigma2_sq_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma2_sq_inter, sigma2_sq, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq, CV_32FC1);

    filter2D(img1_img2, sigma12_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma12_inter, sigma12, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma12, 1, mu1_mu2, -1, 0, sigma12, CV_32FC1);

    //////////////////////////////////////////////////////////////////////////
    // FORMULA

    // (2*mu1_mu2 + C1)
    mu1_mu2.convertTo(temp1, CV_32FC1, 2);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (2*sigma12 + C2)
    sigma12.convertTo(temp2, CV_32FC1, 2);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    multiply(temp1, temp2, temp3, 1, CV_32FC1);

    // (mu1_sq + mu2_sq + C1)
    add(mu1_sq, mu2_sq, temp1, noArray(), CV_32FC1);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (sigma1_sq + sigma2_sq + C2)
    add(sigma1_sq, sigma2_sq, temp2, noArray(), CV_32FC1);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    multiply(temp1, temp2, temp1, 1, CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq
    // + sigma2_sq + C2))
    divide(temp3, temp1, ssim_map, 1, CV_32FC1);

    index_scalar = mean(ssim_map, noArray());

    // return index_scalar;
    return make_tuple(index_scalar, img1_temp, img2_temp, img1_extern,
                      img1_sq_extern, mu1_extern, mu1_sq_extern,
                      sigma1_sq_extern);
}

std::tuple<cv::Scalar, cv::Mat, cv::Mat, cv::Mat, cv::Mat, cv::Mat, cv::Mat>
getMSSIM2(cv::Scalar index_scalar, cv::Mat img1_extern, cv::Mat img2_temp,
          cv::Mat img1_sq_extern, cv::Mat mu1_extern, cv::Mat mu1_sq_extern,
          cv::Mat sigma1_sq_extern)
{

    double C1 = 6.5025, C2 = 58.5225, C3 = 0.00000681;

    cv::Mat kernel =
        (Mat_<float>(1, 11) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);
    cv::Mat kernel1 =
        (Mat_<float>(11, 1) << 0.00102818599752740, 0.00759732401586496,
         0.03599397767545871, 0.10934004978399577, 0.21296533701490150,
         0.26596152026762182, 0.21296533701490150, 0.10934004978399577,
         0.03599397767545871, 0.00759732401586496, 0.00102818599752740);

    Point anchor;
    anchor       = Point(-1, -1);
    double delta = 0;
    int x = img1_extern.rows, y = img1_extern.cols;

    cv::Mat img2_2   = cv::Mat(Size(x, y), CV_32FC3);
    cv::Mat img2_2_2 = cv::Mat(Size(x, y), CV_32FC3);

    img2_temp.convertTo(img2_2, CV_32FC1);
    cvtColor(img2_2, img2_2_2, CV_BGR2GRAY, 1);

    add(img2_2_2, Scalar(C3), img2_2_2);

    cv::Mat img1      = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img1_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img2_sq   = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat img1_img2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat img2_2_2_trans = cv::Mat(Size(y, x), CV_32FC1);

    transpose(img2_2_2, img2_2_2_trans);
    arma::fmat imagine2_arma(img2_2_2_trans.ptr<float>(), img2_temp.rows,
                             img2_temp.cols, true, false);
    arma::fmat imagine2_arma_floor = floor(imagine2_arma);

    cv::Mat img2_from_arma(y, x, CV_32FC1, imagine2_arma_floor.memptr());
    cv::Mat img2(img2_from_arma.t());

    img1 = img1_extern;

    img1_sq = img1_sq_extern;
    pow(img2, 2, img2_sq);

    multiply(img1, img2, img1_img2, 1, CV_32FC1);

    cv::Mat mu1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat mu2 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu1_sq(Size(x, y), CV_32FC1);
    cv::Mat mu2_sq(Size(x, y), CV_32FC1);
    cv::Mat mu1_mu2(Size(x, y), CV_32FC1);

    cv::Mat sigma1_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq       = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma2_sq_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12         = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat temp1 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp2 = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat temp3 = cv::Mat(Size(x, y), CV_32FC1);

    cv::Mat mu2_inter     = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat sigma12_inter = cv::Mat(Size(x, y), CV_32FC1);
    cv::Mat ssim_map      = cv::Mat(Size(x, y), CV_32FC1);

    /*************************** END INITS **********************************/

    //////////////////////////////////////////////////////////////////////////
    // PRELIMINARY COMPUTING

    mu1 = mu1_extern;

    filter2D(img2, mu2_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT);
    filter2D(mu2_inter, mu2, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT);

    mu1_sq = mu1_sq_extern;
    pow(mu2, 2, mu2_sq);

    multiply(mu1, mu2, mu1_mu2, 1, CV_32FC1);

    sigma1_sq = sigma1_sq_extern;

    filter2D(img2_sq, sigma2_sq_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma2_sq_inter, sigma2_sq, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq, CV_32FC1);

    filter2D(img1_img2, sigma12_inter, CV_32FC1, kernel1, anchor, delta,
             BORDER_REFLECT);
    filter2D(sigma12_inter, sigma12, CV_32FC1, kernel, anchor, delta,
             BORDER_REFLECT);
    addWeighted(sigma12, 1, mu1_mu2, -1, 0, sigma12, CV_32FC1);

    //////////////////////////////////////////////////////////////////////////
    // FORMULA

    // (2*mu1_mu2 + C1)
    mu1_mu2.convertTo(temp1, CV_32FC1, 2);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (2*sigma12 + C2)
    sigma12.convertTo(temp2, CV_32FC1, 2);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
    multiply(temp1, temp2, temp3, 1, CV_32FC1);

    // (mu1_sq + mu2_sq + C1)
    add(mu1_sq, mu2_sq, temp1, noArray(), CV_32FC1);
    add(temp1, Scalar(C1), temp1, noArray(), CV_32FC1);

    // (sigma1_sq + sigma2_sq + C2)
    add(sigma1_sq, sigma2_sq, temp2, noArray(), CV_32FC1);
    add(temp2, Scalar(C2), temp2, noArray(), CV_32FC1);

    // ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
    multiply(temp1, temp2, temp1, 1, CV_32FC1);

    // ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq
    // + sigma2_sq + C2))
    divide(temp3, temp1, ssim_map, 1, CV_32FC1);

    index_scalar = mean(ssim_map, noArray());
    // return index_scalar;
    return make_tuple(index_scalar, img1_extern, img2_temp, img1_sq_extern,
                      mu1_extern, mu1_sq_extern, sigma1_sq_extern);
}

template<typename T>
std::string to_string_with_precision(const T a_value, const int n = 7)
{
    std::ostringstream out;
    out << std::fixed << std::setprecision(n) << a_value;
    return out.str();
}

int main(int argc, char **argv)
{
    Scalar index_scalar;
    int c;
    const char *short_opt    = "hlf:";
    struct option long_opt[] = {{"help", no_argument, NULL, 'h'},
                                {"file", required_argument, NULL, 'f'},
                                {"list", required_argument, NULL, 'l'},
                                {NULL, 0, NULL, 0}};

    while ((c = getopt_long(argc, argv, short_opt, long_opt, NULL)) != -1)
    {
        switch (c)
        {
            case -1: /* no more arguments */
            case 0: /* long options toggles */ break;

            case 'f':
                // default settings
                {
                    unsigned int i;
                    Scalar index;
                    int x = 1080, y = 1920;
                    // cv::Mat img1 = cv::Mat( Size(x,y), CV_32FC3);
                    cv::Mat img1_extern = cv::Mat(Size(x, y), CV_32FC3);
                    cv::Mat img1_sq;
                    cv::Mat mu1;
                    cv::Mat mu1_sq;
                    cv::Mat sigma1_sq;
                    cv::Mat img1_temp;
                    cv::Mat img2_temp;
                    std::string name_previous_file;
                    vector<string> input;
                    vector<string> output;
                    ifstream readFile(optarg);
                    if (readFile.is_open())
                    {
                        copy(istream_iterator<string>(readFile), {},
                             back_inserter(input));
                        // cout << "Vector Size is now " << input.size() <<endl;
                        readFile.close();
                    }
                    else
                        cout << "Unable to open file" << endl;

//#pragma omp parallel for
#pragma omp parallel for schedule(dynamic) private(                         \
    name_previous_file, i, img1_temp, img2_temp, img1_extern, img1_sq, mu1, \
    mu1_sq, sigma1_sq, index_scalar, index)
                    for (i = 0; i < input.size(); i = i + 2)
                    {
                        if (i == 0)
                        {
                            cv::Mat img1_temp =
                                imread(input[i] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                            cv::Mat img2_temp = imread(
                                input[i + 1] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                            name_previous_file = input[i];

                            tie(index, img1_temp, img2_temp, img1_extern,
                                img1_sq, mu1, mu1_sq, sigma1_sq) =
                                getMSSIM1(index_scalar, img1_temp, img2_temp,
                                          img1_extern, img1_sq, mu1, mu1_sq,
                                          sigma1_sq);
                        }

                        if ((input[i].compare(name_previous_file) != 0) &&
                            (i > 0))
                        {
                            cv::Mat img1_temp =
                                imread(input[i] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                            cv::Mat img2_temp = imread(
                                input[i + 1] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                            name_previous_file = input[i];

                            tie(index, img1_temp, img2_temp, img1_extern,
                                img1_sq, mu1, mu1_sq, sigma1_sq) =
                                getMSSIM1(index_scalar, img1_temp, img2_temp,
                                          img1_extern, img1_sq, mu1, mu1_sq,
                                          sigma1_sq);
                        }
                        if ((input[i].compare(name_previous_file) == 0) &&
                            (i > 0))
                        {
                            cv::Mat img2_temp = imread(
                                input[i + 1] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                            //#pragma omp parallel private (img2_temp,
                            //name_previous_file, index_scalar, img1_extern,
                            //img1_sq, mu1, mu1_sq, sigma1_sq)
                            name_previous_file = input[i];
                            tie(index, img1_extern, img2_temp, img1_sq, mu1,
                                mu1_sq, sigma1_sq) =
                                getMSSIM2(index_scalar, img1_extern, img2_temp,
                                          img1_sq, mu1, mu1_sq, sigma1_sq);
                        }
                        cout.setf(std::ios::fixed, std::ios::floatfield);
                        // cout << input[i] << " " << input[i+1] << " " <<
                        // setprecision(7) << index.val[0]<<endl ;
                        std::stringstream randul_intermediar;
#pragma omp critical
                        {
                            randul_intermediar
                                << input[i] << " " << input[i + 1] << " "
                                << to_string_with_precision(index.val[0], 7);
                            std::string randul = randul_intermediar.str();
                            output.push_back(randul);
                            std::cout << std::fixed << setprecision(0)
                                      << "Procesarea fisierului " << optarg
                                      << " s-a efectuat in proportie de: "
                                      << i / double(input.size()) * 100 << "%"
                                      << endl;
                        }
                    }
                    // copy(output.begin(), output.end(),
                    // ostream_iterator<string>(cout << setprecision(7), "\n"));
                    ofstream writeFile(strcat(optarg, "-calculat"));
                    if (writeFile.is_open())
                    {
                        copy(output.begin(), output.end(),
                             ostream_iterator<string>(writeFile, "\n"));
                        writeFile.close();
                    }
                    else
                        cout << "Unable to open file";
                    break;
                };

            case 'l':
            {
                if (optind < argc && *argv[optind] != '-')
                {
                    cv::Mat img1_temp =
                        imread(argv[optind] /*, CV_LOAD_IMAGE_GRAYSCALE*/);
                    cv::Mat img2_temp =
                        imread(argv[optind + 1] /*, CV_LOAD_IMAGE_GRAYSCALE*/);

                    if ((!img1_temp.data) &&
                        (!img2_temp.data)) // Check for invalid input
                    {
                        cout << "Could not open or find the image" << std::endl;
                        return -1;
                    }

                    // through observation, there is approximately
                    // 0,00085% error comparing with the original program
                    index_scalar = getMSSIM(img1_temp, img2_temp);

                    cout.setf(std::ios::fixed, std::ios::floatfield);
                    cout << setprecision(7) << index_scalar.val[0] << endl;
                }
                else
                {
                    fprintf(stderr,
                            "\n-l option require TWO arguments <image1> "
                            "<image2>\n\n");
                };
                break;
            };

            case 'h':
            {
                printf("Usage: %s [OPTIONS]\n", argv[0]);
                printf("  -f file            file with name of two images, per "
                       "row, for processing\n");
                printf("  -l list            list of two files\n");
                printf("  -h, --help         print this help and exit\n");
                printf("\n");
                return (0);
            };

            case ':':
            case '?':
                fprintf(stderr, "Try `%s --help' for more information.\n",
                        argv[0]);
                return (-2);

            default:
            {
                fprintf(stderr, "%s: invalid option -- %c\n", argv[0], c);
                fprintf(stderr, "Try `%s --help' for more information.\n",
                        argv[0]);
                return (-2);
            }
        };
    };

    return 0;
}
