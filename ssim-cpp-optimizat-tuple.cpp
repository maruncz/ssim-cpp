#include <cv.h>
#include <highgui.h>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <armadillo>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <sys/resource.h>
#include <tuple>
using namespace std;
using namespace cv;
using namespace arma;

//tuple<int, int, int, int> divide(int dividend, int divisor, int divident1, int divisor1)
//{
//    return  make_tuple(dividend , divisor, divident1, divisor1);
//}

//Scalar getMSSIM( /*const*/ cv::Mat img1_temp, /*const*/ cv::Mat img2_temp);
//Scalar getMSSIM1( /*const*/ cv::Mat img1_temp, /*const*/ cv::Mat img2_temp);

//Scalar getMSSIM( cv::Mat img1_temp, cv::Mat img2_temp);
//Scalar getMSSIM1( cv::Mat img1_temp, cv::Mat img2_temp, cv::Mat &img1_extern, cv::Mat &img1_sq_extern, cv::Mat &mu1_extern, cv::Mat &mu1_sq_extern, cv::Mat &sigma1_sq_extern);
//Scalar getMSSIM2( cv::Mat img1, cv::Mat img2_temp, cv::Mat img1_sq_extern, cv::Mat mu1_extern, cv::Mat mu1_sq_extern, cv::Mat sigma1_sq_extern);





std::tuple < cv::Scalar, cv::Mat, cv::Mat > getMSSIM(cv::Scalar index_scalar, cv::Mat img1_temp, cv::Mat img2_temp)
	{

	double C1 = 6.5025, C2 = 58.5225, C3 = 0.00000681;

	cv::Mat kernel = (Mat_<float>(1,11) <<  0.00102818599752740, 0.00759732401586496, 0.03599397767545871, 0.10934004978399577, 0.21296533701490150, 0.26596152026762182, 0.21296533701490150, 0.10934004978399577, 0.03599397767545871, 0.00759732401586496, 0.00102818599752740);
	cv::Mat kernel1 = (Mat_<float>(11,1) <<  0.00102818599752740, 0.00759732401586496, 0.03599397767545871, 0.10934004978399577, 0.21296533701490150, 0.26596152026762182, 0.21296533701490150, 0.10934004978399577, 0.03599397767545871, 0.00759732401586496, 0.00102818599752740);

	Point anchor;
	anchor = Point( -1, -1 );
	double delta = 0;
	
	int x=img2_temp.rows, y=img2_temp.cols;
	std::cout<<x<<" "<<y<<endl;
	int nChan=img1_temp.channels()-2/*, d=CV_32FC1*/;
		
	cv::Mat img1_1 = cv::Mat( Size(x,y), CV_32FC3);
	cv::Mat img2_2 = cv::Mat( Size(x,y), CV_32FC3);
	cv::Mat img1_1_1 = cv::Mat( Size(x,y), CV_32FC3);
	cv::Mat img2_2_2 = cv::Mat( Size(x,y), CV_32FC3);
	
	img1_temp.convertTo(img1_1, CV_32FC1);
	img2_temp.convertTo(img2_2, CV_32FC1);
        
	cvtColor(img1_1, img1_1_1, CV_BGR2GRAY, 1);
        cvtColor(img2_2, img2_2_2, CV_BGR2GRAY, 1);

	add( img1_1_1, Scalar(C3), img1_1_1 );
	add( img2_2_2, Scalar(C3), img2_2_2 );
	
	cv::Mat img1_sq = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat img2_sq = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat img1_img2 = cv::Mat( Size(x,y), CV_32FC1);

	cv::Mat img1_1_1_trans = cv::Mat( Size(y,x), CV_32FC1);
	cv::Mat img2_2_2_trans = cv::Mat( Size(y,x), CV_32FC1);

	transpose(img1_1_1, img1_1_1_trans);
	transpose(img2_2_2, img2_2_2_trans);
	arma::fmat imagine1_arma(img1_1_1_trans.ptr<float>(), img1_temp.rows, img1_temp.cols, true, false);
	arma::fmat imagine2_arma(img2_2_2_trans.ptr<float>(), img2_temp.rows, img2_temp.cols, true, false);
	arma::fmat imagine1_arma_floor = floor(imagine1_arma);
	arma::fmat imagine2_arma_floor = floor(imagine2_arma);

	cv::Mat img1_from_arma(1920, 1080, CV_32FC1, imagine1_arma_floor.memptr());
	cv::Mat img1(img1_from_arma.t());
	cv::Mat img2_from_arma(1920, 1080, CV_32FC1, imagine2_arma_floor.memptr());
	cv::Mat img2(img2_from_arma.t());
	
	pow( img1, 2, img1_sq);
	pow( img2, 2, img2_sq);
	multiply( img1, img2, img1_img2, 1, CV_32FC1 );

	cv::Mat mu1 = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat mu2 = cv::Mat( Size(x,y), CV_32FC1);

	cv::Mat mu1_sq( Size(x,y), CV_32FC1);
	cv::Mat mu2_sq( Size(x,y), CV_32FC1);
	cv::Mat mu1_mu2( Size(x,y), CV_32FC1);
	
	cv::Mat sigma1_sq = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat sigma2_sq = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat sigma1_sq_inter = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat sigma2_sq_inter = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat sigma12 = cv::Mat( Size(x,y), CV_32FC1);

	cv::Mat temp1 = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat temp2 = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat temp3 = cv::Mat( Size(x,y), CV_32FC1);

	cv::Mat mu1_inter = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat mu2_inter = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat sigma12_inter = cv::Mat( Size(x,y), CV_32FC1);
	cv::Mat ssim_map = cv::Mat( Size(x,y), CV_32FC1);
	
	/*************************** END INITS **********************************/


	//////////////////////////////////////////////////////////////////////////
	// PRELIMINARY COMPUTING

	filter2D(img1, mu1_inter, CV_32FC1, kernel1, anchor, delta, BORDER_REFLECT );
	filter2D(mu1_inter, mu1, CV_32FC1 , kernel, anchor, delta, BORDER_REFLECT );
	
	filter2D(img2, mu2_inter, CV_32FC1 , kernel1, anchor, delta, BORDER_REFLECT );	
	filter2D(mu2_inter, mu2, CV_32FC1 , kernel, anchor, delta, BORDER_REFLECT );
	
	pow( mu1, 2, mu1_sq );
	pow( mu2, 2, mu2_sq );
	multiply( mu1, mu2, mu1_mu2, 1, CV_32FC1 );
	
	filter2D(img1_sq, sigma1_sq_inter, CV_32FC1 , kernel1, anchor, delta, BORDER_REFLECT );
	filter2D(sigma1_sq_inter, sigma1_sq, CV_32FC1 , kernel, anchor, delta, BORDER_REFLECT );
	addWeighted( sigma1_sq, 1, mu1_sq, -1, 0, sigma1_sq, CV_32FC1 );

	filter2D(img2_sq, sigma2_sq_inter, CV_32FC1 , kernel1, anchor, delta, BORDER_REFLECT );	
	filter2D(sigma2_sq_inter, sigma2_sq, CV_32FC1, kernel, anchor, delta, BORDER_REFLECT );
	addWeighted( sigma2_sq, 1, mu2_sq, -1, 0, sigma2_sq, CV_32FC1 );
	
	filter2D(img1_img2, sigma12_inter, CV_32FC1 , kernel1, anchor, delta, BORDER_REFLECT );
	filter2D(sigma12_inter, sigma12, CV_32FC1 , kernel, anchor, delta, BORDER_REFLECT );
	addWeighted( sigma12, 1, mu1_mu2, -1, 0, sigma12, CV_32FC1 );
	
	int a=mu1.rows, b=mu1.cols, c=mu1.channels(), d=mu1.type();
	int e=mu2.rows, f=mu2.cols, g=mu2.channels(), h=mu2.type();
	int i=mu1_mu2.rows, j=mu1_mu2.cols, k=mu1_mu2.channels(), l=mu1_mu2.type();
	int m=img1.rows, n=img1.cols, o=img1.channels(), p=img1.type();
	std::cout<< a <<" "<<b <<" " <<c<<" " << d<<" canale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	std::cout<< e <<" "<<f <<" " <<g<< " "<< h<<" canale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	std::cout<< i <<" "<<j <<" " <<k<< " " <<l<<" canale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
	std::cout<< m <<" "<<n <<" " <<o<< " "<< p<<" canale!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;

	//////////////////////////////////////////////////////////////////////////
	// FORMULA
	
	// (2*mu1_mu2 + C1)
	mu1_mu2.convertTo(temp1, CV_32FC1, 2);
	add( temp1, Scalar(C1), temp1, noArray(), CV_32FC1 );
	
	// (2*sigma12 + C2)
	sigma12.convertTo(temp2, CV_32FC1, 2);
	add( temp2, Scalar(C2), temp2, noArray(), CV_32FC1 );
	
	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
	multiply( temp1, temp2, temp3, 1, CV_32FC1 );

	// (mu1_sq + mu2_sq + C1)
	add( mu1_sq, mu2_sq, temp1, noArray(), CV_32FC1 );
	add( temp1, Scalar(C1), temp1, noArray(), CV_32FC1 );
	
	// (sigma1_sq + sigma2_sq + C2)
	add( sigma1_sq, sigma2_sq, temp2, noArray(), CV_32FC1 );
	add( temp2, Scalar(C2), temp2, noArray(), CV_32FC1 );
	
	// ((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
	multiply( temp1, temp2, temp1, 1, CV_32FC1 );
	
	// ((2*mu1_mu2 + C1).*(2*sigma12 + C2))./((mu1_sq + mu2_sq + C1).*(sigma1_sq + sigma2_sq + C2))
	divide( temp3, temp1, ssim_map, 1, CV_32FC1);
	
	/*Scalar */index_scalar = mean( ssim_map, noArray() );
	//return index_scalar;
	return  make_tuple(index_scalar, img1_temp, img2_temp);
	}


int main(int argc, char** argv)
{
	//if(argc!=3)
	//	return -1;

	// default settings
	Scalar index_scalar;
	
	/***************************** INITS **********************************/

	if(argc==3){
		cv::Mat img1_temp_bobo = imread(argv[1]/*, CV_LOAD_IMAGE_GRAYSCALE*/);
		cv::Mat img2_temp_bobo = imread(argv[2]/*, CV_LOAD_IMAGE_GRAYSCALE*/);

	//if(img1_temp==NULL || img2_temp==NULL)
	//	return -1;

	// through observation, there is approximately 
	// 0,00085% error comparing with the original program
	//index_scalar=getMSSIM( img1_temp, img2_temp);
	cv::Mat img1;
	cv::Mat img2;
	Scalar index;
	tie(index, img1, img2) = getMSSIM(index_scalar, img1_temp_bobo, img2_temp_bobo);

	cout.setf(std::ios::fixed, std:: ios::floatfield);	
	cout << setprecision(7) << index.val[0] <<endl ;
	//cout << setprecision(7) << img2 <<endl ;
	}
		
	return 0;
}

