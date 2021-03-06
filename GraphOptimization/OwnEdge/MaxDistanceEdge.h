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
// Created by steve on 18-3-4.
//

#ifndef COMPLEXITYPOSITIONING_MAXDISTANCEEDGE_H
#define COMPLEXITYPOSITIONING_MAXDISTANCEEDGE_H

#include "SimpleDistanceEdge.h"

class MaxDistanceEdge: public SimpleDistanceEdge{
public:
    MaxDistanceEdge(){

    }

    void computeError(){
        g2o::VertexSE3 *from = static_cast<g2o::VertexSE3 *>(_vertices[0]);
        g2o::VertexSE3 *to = static_cast<g2o::VertexSE3 *>(_vertices[1]);

        double p1[10], p2[10];
        from->getEstimateData(p1);
        to->getEstimateData(p2);

        double dis_2 = (p1[0] - p2[0]) * (p1[0] - p2[0]) +
                       (p1[1] - p2[1]) * (p1[1] - p2[1]) +
                       (p1[2] - p2[2]) * (p1[2] - p2[2]);

        double dis = std::sqrt(dis_2);

        if(dis > max_distance_){
            _error(0,0)= std::abs(dis-max_distance_);
        }else{
            _error(0,0) = 0.0;
        }
    }

private:
    double max_distance_ = 1.5;
public:
    double getMax_distance_() const {
        return max_distance_;
    }

    void setMax_distance_(double max_distance_) {
        MaxDistanceEdge::max_distance_ = max_distance_;
    }
};

#endif //COMPLEXITYPOSITIONING_MAXDISTANCEEDGE_H
