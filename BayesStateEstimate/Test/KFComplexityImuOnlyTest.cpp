//
// Created by steve on 18-1-24.
//


#include <iostream>
#include <fstream>
#include <thread>

#define EIGEN_USE_MKL_ALL

#include <Eigen/Core>
#include <Eigen/Dense>
//#include <AWF.h>dd


#include <AWF.h>

#include "BayesFilter/KalmanFilter/IMUWBKFSimple.h"


#include "BayesFilter/KalmanFilter/KFComplex.h"


#include "../AuxiliaryTool/UwbTools.cpp"


#include <BayesFilter/KalmanFilter/KFComplexFull.h>

#include <BayesFilter/KalmanFilter/KFComplexFF.h>
#include <BayesFilter/KalmanFilter/UKFComplex.h>

#include <BayesFilter/KalmanFilter/UKFComplexCraft.h>

namespace plt = matplotlibcpp;


int main(int argc, char *argv[]) {

	omp_set_num_threads(12);

	double start_time = AWF::getDoubleSecondTime();

//	omp_set_num_threads(6);
//	Eigen::setNbThreads(6);

	std::cout.precision(10);
	// parameters
//    std::string dir_name = "/home/steve/Data/NewFusingLocationData/0018/";
//    std::string dir_name = "/home/steve/Data/FusingLocationData/0010/";
//    std::string dir_name = "/home/steve/Data/FusingLocationData/0010/";
//	std::string dir_name = "/home/steve/Data/FusingLocationData/0013/";
	std::string dir_name = "/home/steve/Data/NewFusingLocationData/0035/";


	auto logger_ptr = AWF::AlgorithmLogger::getInstance();


	// load data
	AWF::FileReader left_foot_file(dir_name + "LEFT_FOOT.data"),
			right_foot_file(dir_name + "RIGHT_FOOT.data"),
			head_imu_file(dir_name + "HEAD.data");

	Eigen::MatrixXd left_imu_data = left_foot_file.extractDoulbeMatrix(",");
	Eigen::MatrixXd right_imu_data = right_foot_file.extractDoulbeMatrix(",");
	Eigen::MatrixXd head_imu_data = head_imu_file.extractDoulbeMatrix(",");

	Eigen::Vector3d initial_pos = Eigen::Vector3d(0, 0, 0);

	double initial_ori = 0.0;
//	double initial_ori = 1.0;

	std::vector<std::vector<double>> optimize_trace_vec = {{},
	                                                       {},
	                                                       {}
	};




	//process
	BSE::ImuTools::processImuData(left_imu_data);
	BSE::ImuTools::processImuData(right_imu_data);
	BSE::ImuTools::processImuData(head_imu_data);

	Eigen::MatrixXd process_noise_matrix =
			Eigen::MatrixXd::Identity(6, 6);
	process_noise_matrix.block(0, 0, 3, 3) *= 0.1;
	process_noise_matrix.block(3, 3, 3, 3) *= (0.1 * M_PI / 180.0);


	Eigen::MatrixXd initial_prob_matrix = Eigen::MatrixXd::Identity(9, 9);
	initial_prob_matrix.block(0, 0, 3, 3) *= 0.05;
	initial_prob_matrix.block(3, 3, 3, 3) *= 0.05;
	initial_prob_matrix.block(6, 6, 3, 3) *= 0.05 * (M_PI / 180.0);


	Eigen::MatrixXd initial_prob_matrix_complex = Eigen::MatrixXd::Identity(15, 15);
	initial_prob_matrix_complex.block(0, 0, 3, 3) *= 0.001;
	initial_prob_matrix_complex.block(3, 3, 3, 3) *= 0.001;
	initial_prob_matrix_complex.block(6, 6, 3, 3) *= 0.001 * (M_PI / 180.0);
	initial_prob_matrix_complex.block(9, 9, 3, 3) *= 0.0001;
	initial_prob_matrix_complex.block(12, 12, 3, 3) *= 0.0001 * (M_PI / 180.0);

	Eigen::MatrixXd initial_prob_matrix_ff = Eigen::MatrixXd::Identity(21, 21);
	initial_prob_matrix_ff.block(0, 0, 15, 15) = initial_prob_matrix_complex * 1.0;

	initial_prob_matrix_ff.block(15, 15, 3, 3) *= 0.00000000001;
	initial_prob_matrix_ff.block(18, 18, 3, 3) *= 0.000001 * (M_PI / 180.0);


	auto f = [&process_noise_matrix,
			&initial_prob_matrix,
			&initial_prob_matrix_complex,
			&initial_prob_matrix_ff,
			&initial_pos,
			&initial_ori,
			&optimize_trace_vec,
			&logger_ptr](const Eigen::MatrixXd &imu_data,
	                     std::string data_name) {

		/// Define and Initialize Filter.
		auto filter = BSE::IMUWBKFSimple(
				initial_prob_matrix);

//		auto filter_complex = BSE::KFComplex(initial_prob_matrix);
//		auto filter_complex = BSE::UKFComplex(initial_prob_matrix);
//        auto filter_complex = BSE::KFComplexFull(initial_prob_matrix_complex);

//		auto complex_ukf_filter = BSE::KFComplexFull(initial_prob_matrix_complex);
//		auto complex_ukf_filter = BSE::UKFComplex(initial_prob_matrix_complex);
		auto complex_ukf_filter = BSE::UKFComplexCraft(initial_prob_matrix_complex);

//		auto ff_filter = BSE::KFComplexFF(initial_prob_matrix_ff);

		double tmp_time_interval = (imu_data(imu_data.rows() - 1, 0) - imu_data(0, 0))
		                           / double(imu_data.rows());

		std::cout << "time interval :" << tmp_time_interval << std::endl;
		if (std::abs(tmp_time_interval - 0.005) < 0.0001) {
			filter.setTime_interval_(0.005);
//			filter_complex.time_interval_ = 0.005;
		} else if (std::abs(tmp_time_interval - 0.01) < 0.001) {
			filter.setTime_interval_(0.01);
//			filter_complex.time_interval_ = 0.01;
		} else {
			filter.setTime_interval_(tmp_time_interval);
//			filter_complex.time_interval_ = tmp_time_interval;
		}
		complex_ukf_filter.time_interval_ =tmp_time_interval;
//		ff_filter.time_interval_ = complex_ukf_filter.time_interval_;

		double local_g = -9.81;

		filter.setLocal_g_(-9.8);
//		filter_complex.local_g_ = -9.8;
		complex_ukf_filter.local_g_ = local_g;
//		ff_filter.local_g_ = complex_ukf_filter.local_g_;
//    filter.IS_DEBUG = true;
		std::vector<double> zv_flag = {};


		auto time_begin = AWF::getDoubleSecondTime();
		filter.initial_state(imu_data.block(10, 1, 100, 6),
		                     initial_ori,
		                     initial_pos);


//		filter_complex.initial_state(imu_data.block(10, 1, 100, 9),
//		                             initial_ori,
//		                             initial_pos);
		std::cout << "initial state costed time :" << AWF::getDoubleSecondTime() - time_begin
		          << std::endl;
		complex_ukf_filter.initial_state(imu_data.block(10, 1, 100, 9),
		                                 initial_ori,
		                                 initial_pos);
//		ff_filter.initial_state(imu_data.block(10, 1, 100, 9),
//		                        initial_ori,
//		                        initial_pos);


//    filter.sett
		for (int i(5); i < imu_data.rows() - 5; ++i) {
			/// state transaction equation
//			filter.StateTransaction(imu_data.block(i, 1, 1, 6).transpose(),
//			                        process_noise_matrix,
//			                        BSE::StateTransactionMethodType::NormalRotation);


//			auto complex_state = filter_complex.StateTransIMU(imu_data.block(i, 1, 1, 6).transpose(),
//			                                                  process_noise_matrix);
//			auto complex_full_state =
//					complex_ukf_filter.StateTransIMU(imu_data.block(i, 1, 1, 6).transpose(),
//					                                  process_noise_matrix);
			auto complex_full_state =
					complex_ukf_filter.StateTransIMU_jac(imu_data.block(i, 1, 1, 6).transpose(),
					                                     process_noise_matrix);


//			auto ff_full_state =
//					ff_filter.StateTransIMU(imu_data.block(i, 1, 1, 6).transpose(),
//					                        process_noise_matrix);


//            filter_complex.MeasurementAngleCorrect(imu_data.block(i, 7, 1, 3).transpose(),
//                                                   Eigen::Matrix3d::Identity() * 0.000246);
//			complex_ukf_filter.MeasurementAngleCorrect(imu_data.block(i, 7, 1, 3).transpose(),
//			                                            Eigen::Matrix3d::Identity() * 0.00001);

			double uwb_index = 0;
			/// uwb measurement
			bool tmp_break_flag = false;


			if (BSE::ImuTools::GLRT_Detector(imu_data.block(i - 5, 1, 10, 6))) {
				/// zero velocity detector
//				filter.MeasurementState(Eigen::Vector3d(0, 0, 0),
//				                        Eigen::Matrix3d::Identity() * 0.0251001,
//				                        BSE::MeasurementMethodType::NormalZeroVeclotiMeasurement);

//				filter_complex.MeasurementStateZV(Eigen::Matrix3d::Identity() * 0.025);

				complex_ukf_filter.MeasurementStateZV(Eigen::Matrix3d::Identity() * 0.25);

//				ff_filter.MeasurementStateZV(Eigen::Matrix3d::Identity() * 0.0000025);


				/// angle constraint through acc.
				int zv_index = zv_flag.size() - 1;
				bool last_zv_flag = true;
				for (int d(0); d < 5; ++d) {
					if (zv_index - d > 0 && !zv_flag[zv_index - d]) {
						last_zv_flag = false;
					}
				}


				Eigen::Matrix<double, 6, 1> tmp_gm;
				tmp_gm.block(0, 0, 3, 1) = imu_data.block(i, 1, 1, 3).transpose();
				tmp_gm.block(3, 0, 3, 1) = imu_data.block(i, 7, 1, 3).transpose();
				Eigen::Matrix<double, 6, 6> cov_matrix = Eigen::Matrix<double, 6, 6>::Identity();
				cov_matrix.block(0, 0, 3, 3) *= 0.00001;
				cov_matrix.block(3, 3, 3, 3) *= 0.00003;


//                if( current_diff < 0.1)
//                filter_complex.MeasurementAngleCorrectMG(tmp_gm, cov_matrix);

				if (zv_flag.size() > 3 &&
				    zv_flag.at(zv_flag.size() - 2) < 0.5) {
//                    std::cout << i
//                              << " lacc:"
//                              << (filter_complex.rotation_q_.toRotationMatrix() *
//                                  imu_data.block(i, 1, 1, 3).transpose()).transpose()
//                              << std::endl;
				}

				zv_flag.push_back(1.0);
			} else {
				zv_flag.push_back(0.0);
			}

			Eigen::VectorXd state_simple = filter.getState_();
//			Eigen::VectorXd state = filter_complex.state_x_;
			Eigen::VectorXd full_state = complex_ukf_filter.state_x_;
			assert(full_state.rows() == 15 && full_state.cols() == 1);
//			Eigen::VectorXd ff_state = ff_filter.state_x_;

			logger_ptr->addPlotEvent(data_name + "velocity", "velocitysimple", state_simple.block(3, 0, 3, 1));
//			logger_ptr->addPlotEvent(data_name + "velocity", "velocitycomplex", state.block(3, 0, 3, 1));
			logger_ptr->addPlotEvent(data_name + "velocity", "velocityfull", full_state.block(3, 0, 3, 1));
//			logger_ptr->addPlotEvent(data_name + "velocity", "velocityff", ff_state.block(3, 0, 3, 1));

			logger_ptr->addPlotEvent(data_name + "angle", "anglesimple", state_simple.block(6, 0, 3, 1));
//			logger_ptr->addPlotEvent(data_name + "angle", "anglecomplex", state.block(6, 0, 3, 1));
			logger_ptr->addPlotEvent(data_name + "angle", "anglefull", full_state.block(6, 0, 3, 1));
//			logger_ptr->addPlotEvent(data_name + "angle", "angleff", ff_state.block(6, 0, 3, 1));


			logger_ptr->addTrace3dEvent(data_name, "simple", state_simple.block(0, 0, 3, 1));
//			logger_ptr->addTrace3dEvent(data_name, "complex", state.block(0, 0, 3, 1));
			logger_ptr->addTrace3dEvent(data_name, "full", full_state.block(0, 0, 3, 1));
//			logger_ptr->addTrace3dEvent(data_name, "ff", ff_state.block(0, 0, 3, 1));

			logger_ptr->addTraceEvent(data_name, "simple", state_simple.block(0, 0, 2, 1));
//			logger_ptr->addTraceEvent(data_name, "complex", state.block(0, 0, 2, 1));
			logger_ptr->addTraceEvent(data_name, "full", full_state.block(0, 0, 2, 1));
//			logger_ptr->addTraceEvent(data_name, "ff", ff_state.block(0, 0, 2, 1));

		}


	};

//
//	f(left_imu_data, "left_foot");
	f(right_imu_data, "right_foot");
//    f(head_imu_data, "head");

	std::cout << "time:" << AWF::getDoubleSecondTime() - start_time << std::endl;
	std::cout << "data time:" << right_imu_data(right_imu_data.rows() - 5, 0) - right_imu_data(0, 0) << std::endl;

	logger_ptr->outputAllEvent(true);

}
