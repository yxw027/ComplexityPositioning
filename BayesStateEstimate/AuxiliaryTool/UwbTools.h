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
// Created by steve on 18-2-3.
//

#ifndef COMPLEXITYPOSITIONING_UWBTOOLS_H
#define COMPLEXITYPOSITIONING_UWBTOOLS_H

#include <Eigen/Dense>

namespace BSE {
    class UwbTools {
    public:
        UwbTools(Eigen::MatrixXd uwb_data, Eigen::MatrixXd beacon_set_data) :
                uwb_data_(uwb_data), beacon_set_(beacon_set_data) {


        }


        Eigen::MatrixXd uwb_data_;
        Eigen::MatrixXd beacon_set_;
        int uwb_index = 0;

        std::function<double(Eigen::Vector3d)> uwb_err_function = [&]
                (Eigen::Vector3d pos) -> double {
            int vaild_counter = 0;
            double sum_err = 0.0;
            for (int i(1); i < uwb_data_.cols(); ++i) {
                if (uwb_data_(uwb_index, i) > 0.0) {
                    sum_err += std::abs(uwb_data_(uwb_index, i) -
                                        (pos - beacon_set_.block(i - 1, 0, 1, 3).transpose()).norm());
                    vaild_counter++;
                }

            }
            if (vaild_counter > 0) {

                return sum_err / double(vaild_counter);
            } else {
                return 1000000.0;
            }
        };

        std::function<Eigen::MatrixXd()> uwb_position_function = [
                &]() -> Eigen::MatrixXd {
            Eigen::MatrixXd trace = Eigen::MatrixXd(uwb_data_.rows(), 3);
            Eigen::Vector3d initial_pos(0, 0, 0);
            for (int i(0); i < trace.rows(); ++i) {
                uwb_index = i;
                double last_uwb_err = 10000000000.0;
                int ite_times = 0;

                double step_length = 0.00001;
                double update_rate = 0.15;

                while (
                        ite_times < 1000) {
                    last_uwb_err = uwb_err_function(initial_pos);
                    Eigen::Vector3d tmp_gradient(0, 0, 0);
                    for (int i(0); i < 3; ++i) {
                        auto t_v = initial_pos;
                        t_v(i) += step_length;
                        tmp_gradient(i) = (uwb_err_function(t_v) - last_uwb_err) / step_length;
                    }
                    initial_pos -= tmp_gradient * update_rate;


//                    std::cout << "last err:" << last_uwb_err << std::endl;
                    ite_times++;

                }

                trace.block(i,0,1,3) = initial_pos.transpose();

            }
            return trace;

        };


    };
}


#endif //COMPLEXITYPOSITIONING_UWBTOOLS_H
