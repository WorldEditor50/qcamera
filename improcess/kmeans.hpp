#ifndef KMEANS_H
#define KMEANS_H
#include <vector>
#include <map>
#include <cmath>
#include <string>
#include <random>
#include <functional>
#include "tensor.hpp"
#include "linalg.h"

class Kmeans
{
public:
    using FnDistance = std::function<float(const Tensor &x1, const Tensor &x2)>;
public:
    FnDistance distance;
    std::size_t topicDim;
    std::size_t featureDim;
    std::vector<Tensor> centers;
public:
    Kmeans(){}
    explicit Kmeans(std::size_t k, std::size_t featureDim_)
        :distance(LinAlg::normL2),topicDim(k),featureDim(featureDim_){}
    explicit Kmeans(std::size_t k, std::size_t featureDim_, const FnDistance &func)
        :distance(func),topicDim(k),featureDim(featureDim_){}
    void cluster(const std::vector<Tensor> &x, std::size_t maxEpoch, float eps=1e-6)
    {
        /* init center */
        std::uniform_int_distribution<int> uniform(0, x.size() - 1);
        centers = std::vector<Tensor>(topicDim);
        for (std::size_t i = 0; i < centers.size(); i++) {
            int k = uniform(LinAlg::Random::engine);
            centers[i] = x[k];
        }
        /* cluster */
        std::vector<std::vector<std::size_t> > groups(topicDim);
        std::vector<Tensor> centers_(topicDim, Tensor(featureDim, 1));
        for (std::size_t epoch = 0; epoch < maxEpoch; epoch++) {
            /* pick the nearest topic */
            for (std::size_t i = 0; i < x.size(); i++) {
                float maxD = -1;
                std::size_t topic = 0;
                for (std::size_t j = 0; j < centers.size(); j++) {
                    float d = distance(x[i], centers[j]);
                    if (d > maxD) {
                        maxD = d;
                        topic = j;
                    }
                }
                groups[topic].push_back(i);
            }
            /* calculate center */
            for (std::size_t i = 0; i < groups.size(); i++) {
                centers_[i].zero();
                for (std::size_t j = 0; j < groups[i].size(); j++) {
                    std::size_t h = groups[i][j];
                    centers_[i] += x[h];
                }
                centers_[i] /= float(groups[i].size());
            }
            /* error */
            float delta = 0;
            for (std::size_t i = 0; i < topicDim; i++) {
                delta += LinAlg::normL2(centers[i], centers_[i]);
            }
            delta /= float(topicDim);
            //std::cout<<"epoch:"<<epoch<<", error:"<<delta<<std::endl;
            if (delta < eps) {
                break;
            }
            /* update center */
            centers.swap(centers_);
            /* clear */
            for (std::size_t i = 0; i < centers.size(); i++) {
                groups[i].clear();
            }
        }
        return;
    }

    std::size_t operator()(const Tensor &x)
    {
        float maxD = -1;
        std::size_t topic = 0;
        for (std::size_t i = 0; i < centers.size(); i++) {
            float d = distance(x, centers[i]);
            if (d > maxD) {
                maxD = d;
                topic = i;
            }
        }
        return topic;
    }

};

#endif // KMEANS_H
