/** 
                   _ooOoo_ 
                  o8888888o 
                  88" . "88 
                  (| -_- |) 
                  O\  =  /O 
               ____/`---'\____ 
             .'  \\|     |//  `. 
            /  \\|||  :  |||//  \ 
           /  _||||| -:- |||||-  \ 
           |   | \\\  -  /// |   | 
           | \_|  ''\---/''  |   | 
           \  .-\__  `-`  ___/-. / 
         ___`. .'  /--.--\  `. . __ 
      ."" '<  `.___\_<|>_/___.'  >'"". 
     | | :  `- \`.;`\ _ /`;.`/ - ` : | | 
     \  \ `-.   \_ __\ /__ _/   .-` /  / 
======`-.____`-.___\_____/___.-`____.-'====== 
                   `=---=' 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
         佛祖保佑       永无BUG 
*/
//
// Created by steve on 18-3-9.
//


#ifndef COMPLEXITYPOSITIONING_GRAVITYORIENTATIONFUNCTION_H
#define COMPLEXITYPOSITIONING_GRAVITYORIENTATIONFUNCTION_H

#include <Eigen/Dense>
#include <AWF.h>

//class GravityOrientationFunction : public AWF::FunctionAbstract {
//public:
//    GravityOrientationFunction(Eigen::Vector3d acc, double g, double initial_yaw) :
//            FunctionAbstract(1,1) {
//        acc_ = acc;
//        g_ = g;
//        yaw_ = initial_yaw;
//
//    }
//
//    Eigen::MatrixXd operator()(std::vector<Eigen::MatrixXd> in_vec) {
//        return compute(in_vec[0]);
//    }
//
//
//
//    Eigen::MatrixXd compute(Eigen::MatrixXd orientation) {
//        Eigen::MatrixXd error;
//        error.resize(1, 1);
//
//        return g_error(orientation(0,0),orientation(1,0),yaw_);
//    }
//
//
//    std::vector<Eigen::MatrixXd> derivative(Eigen::MatrixXd in1) {
//        return d(compress(in1));
//    }
//
//    std::vector<Eigen::MatrixXd> minimize_error(Eigen::MatrixXd in1){
//        return minimize(compress(in1),100,1e-1);
//    }
//
//    std::vector<Eigen::MatrixXd> compress(Eigen::MatrixXd ori) {
//        std::vector<Eigen::MatrixXd> t = {};
//        t.push_back(ori);
//        return t;
//    }
//
//    Eigen::Vector3d acc_;
//    double g_;
//    double yaw_;
//
//    Eigen::MatrixXd g_error(double roll, double pitch, double yaw) {
//
//        auto rotate_matrix = (Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX())
//                              * Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY())
//                              * Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
//        Eigen::MatrixXd error_matrix(1,1);
//        error_matrix(0,0) =  std::abs(g_ - (rotate_matrix * acc_)(2));
//        return error_matrix;
//    };
//
//
//};


class GravityOrientationFunction :
        public AWF::SingleFunctionAbstract {

public:

    /**
     *
     * @param acc
     * @param g
     * @param initial_yaw
     */
    GravityOrientationFunction(Eigen::Vector3d acc,
                               double g,
                               double initial_yaw) :
            SingleFunctionAbstract(1) {
        acc_ = acc;
        g_ = acc.norm();
        yaw_ = initial_yaw;

    }

//    Eigen::MatrixXd operator()(std::vector<Eigen::MatrixXd> in_vec) {
//        return operator()(in_vec[0]);
//    }


    Eigen::MatrixXd compute(Eigen::MatrixXd orientation) {
        Eigen::MatrixXd error;
        error.resize(1, 1);


        return g_error(orientation(0, 0), orientation(1, 0), yaw_);
    }


    Eigen::MatrixXd g_error(double roll, double pitch, double yaw) {

//        auto rotate_matrix = (Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX())
//                              * Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY())
//                              * Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
        Sophus::SO3d rbn_ = Sophus::SO3d::exp(Eigen::Vector3d(roll, pitch, yaw));
        Eigen::MatrixXd error_matrix(1, 1);
        error_matrix(0, 0) = std::abs(g_ - (rbn_.matrix() * acc_)(2));
//        std::cout << error_matrix(0,0) << std::endl;
        return error_matrix;
    }


    Eigen::Vector3d acc_;
    double g_;
    double yaw_;
};

#endif //COMPLEXITYPOSITIONING_GRAVITYORIENTATIONFUNCTION_H
