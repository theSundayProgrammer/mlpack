/**
* @file convolutional_network_test.cpp
* @author Marcus Edel
*
* Tests the convolutional neural network.
*/
#include <mlpack/core.hpp>

#include <mlpack/methods/ann/activation_functions/logistic_function.hpp>

#include <mlpack/methods/ann/layer/one_hot_layer.hpp>
#include <mlpack/methods/ann/layer/conv_layer.hpp>
#include <mlpack/methods/ann/layer/pooling_layer.hpp>
#include <mlpack/methods/ann/layer/softmax_layer.hpp>
#include <mlpack/methods/ann/layer/bias_layer.hpp>
#include <mlpack/methods/ann/layer/linear_layer.hpp>
#include <mlpack/methods/ann/layer/base_layer.hpp>

#include <mlpack/methods/ann/performance_functions/mse_function.hpp>
#include <mlpack/core/optimizers/rmsprop/rmsprop.hpp>

#include <mlpack/methods/ann/init_rules/random_init.hpp>
#include <mlpack/methods/ann/cnn.hpp>

#include <boost/test/unit_test.hpp>
#include "old_boost_test_definitions.hpp"

using namespace mlpack;
using namespace mlpack::ann;
using namespace mlpack::optimization;


BOOST_AUTO_TEST_SUITE(ConvolutionalNetworkTest);


void BuildVanillaNetwork()
{
  arma::mat X;
  X.load("mnist_first250_training_4s_and_9s.arm");

  // Normalize each point since these are images.
  arma::uword nPoints = X.n_cols;
  for (arma::uword i = 0; i < nPoints; i++)
  {
    X.col(i) /= norm(X.col(i), 2);
  }

  // Build the target matrix.
  arma::mat Y = arma::zeros<arma::mat>(10, nPoints);
  for (size_t i = 0; i < nPoints; i++)
  {
    if (i < nPoints / 2)
    {
      Y.col(i)(5) = 1;
    }
    else
    {
      Y.col(i)(8) = 1;
    }
  }

  arma::cube input = arma::cube(28, 28, nPoints);
  for (size_t i = 0; i < nPoints; i++)
    input.slice(i) = arma::mat(X.colptr(i), 28, 28);

  /*
  * Construct a convolutional neural network with a 28x28x1 input layer,
  * 24x24x8 convolution layer, 12x12x8 pooling layer, 8x8x12 convolution layer
  * and a 4x4x12 pooling layer which is fully connected with the output layer.
  * The network structure looks like:
  *
  * Input    Convolution  Pooling      Convolution  Pooling      Output
  * Layer    Layer        Layer        Layer        Layer        Layer
  *
  *          +---+        +---+        +---+        +---+
  *          | +---+      | +---+      | +---+      | +---+
  * +---+    | | +---+    | | +---+    | | +---+    | | +---+    +---+
  * |   |    | | |   |    | | |   |    | | |   |    | | |   |    |   |
  * |   +--> +-+ |   +--> +-+ |   +--> +-+ |   +--> +-+ |   +--> |   |
  * |   |      +-+   |      +-+   |      +-+   |      +-+   |    |   |
  * +---+        +---+        +---+        +---+        +---+    +---+
  */

  ConvLayer<> convLayer0(1, 8, 5, 5);
  BiasLayer2D<> biasLayer0(8);
  BaseLayer2D<> baseLayer0;
  PoolingLayer<> poolingLayer0(2);

  ConvLayer<> convLayer01(convLayer0);
  BiasLayer2D<> biasLayer01(biasLayer0);
  BaseLayer2D<> baseLayer01(baseLayer0);
  PoolingLayer<> poolingLayer01(poolingLayer0);


  ConvLayer<> convLayer1(8, 12, 5, 5);
  BiasLayer2D<> biasLayer1(12);
  BaseLayer2D<> baseLayer1;
  PoolingLayer<> poolingLayer1(2);

  LinearMappingLayer<> linearLayer0(192, 10);
  BiasLayer<> biasLayer2(10);
  SoftmaxLayer<> softmaxLayer0;

  LinearMappingLayer<> linearLayer01(linearLayer0);
  BiasLayer<> biasLayer21(biasLayer2);
  SoftmaxLayer<> softmaxLayer01(softmaxLayer0);

  OneHotLayer outputLayer;
  biasLayer0.Weights().zeros();
  biasLayer1.Weights().zeros();

  //auto modules = std::make_tuple(convLayer01, biasLayer01, baseLayer01, poolingLayer01,
  //  convLayer1, biasLayer1, baseLayer1, poolingLayer1,
  //  linearLayer01, biasLayer21, softmaxLayer01);

  //
  auto modules = std::tie(convLayer0, biasLayer0, baseLayer0, poolingLayer0,
    convLayer1, biasLayer1, baseLayer1, poolingLayer1,
    linearLayer0, biasLayer2, softmaxLayer0);

  CNN<decltype(modules), decltype(outputLayer),
    RandomInitialization, MeanSquaredErrorFunction> net(std::move(modules), std::move(outputLayer));

  RMSprop<decltype(net)> opt(net, 0.01, 0.88, 1e-8, 10 * input.n_slices, 0);

  net.Train(input, Y, opt);

  arma::mat prediction;
  net.Predict(input, prediction);

  size_t error = 0;
  for (size_t i = 0; i < nPoints; i++)
  {
    if (arma::sum(arma::sum(
      arma::abs(prediction.col(i) - Y.col(i)))) == 0)
    {
      error++;
    }
  }

  double classificationError = 1 - double(error) / nPoints;
  BOOST_REQUIRE_LE(classificationError, 0.6);
}
auto CopyVanillaNetwork()
{


  /*
  * Construct a convolutional neural network with a 28x28x1 input layer,
  * 24x24x8 convolution layer, 12x12x8 pooling layer, 8x8x12 convolution layer
  * and a 4x4x12 pooling layer which is fully connected with the output layer.
  * The network structure looks like:
  *
  * Input    Convolution  Pooling      Convolution  Pooling      Output
  * Layer    Layer        Layer        Layer        Layer        Layer
  *
  *          +---+        +---+        +---+        +---+
  *          | +---+      | +---+      | +---+      | +---+
  * +---+    | | +---+    | | +---+    | | +---+    | | +---+    +---+
  * |   |    | | |   |    | | |   |    | | |   |    | | |   |    |   |
  * |   +--> +-+ |   +--> +-+ |   +--> +-+ |   +--> +-+ |   +--> |   |
  * |   |      +-+   |      +-+   |      +-+   |      +-+   |    |   |
  * +---+        +---+        +---+        +---+        +---+    +---+
  */

  ConvLayer<> convLayer0(1, 8, 5, 5);
  BiasLayer2D<> biasLayer0(8);
  BaseLayer2D<> baseLayer0;
  PoolingLayer<> poolingLayer0(2);



  ConvLayer<> convLayer1(8, 12, 5, 5);
  BiasLayer2D<> biasLayer1(12);
  BaseLayer2D<> baseLayer1;
  PoolingLayer<> poolingLayer1(2);

  LinearMappingLayer<> linearLayer0(192, 10);
  BiasLayer<> biasLayer2(10);
  SoftmaxLayer<> softmaxLayer0;

  OneHotLayer outputLayer;
  biasLayer0.Weights().zeros();
  biasLayer1.Weights().zeros();

  auto modules = std::make_tuple(convLayer0, biasLayer0, baseLayer0, poolingLayer0,
    convLayer1, biasLayer1, baseLayer1, poolingLayer1,
    linearLayer0, biasLayer2, softmaxLayer0);

  CNN<decltype(modules), decltype(outputLayer),
    RandomInitialization, MeanSquaredErrorFunction> net(std::move(modules), std::move(outputLayer));
  return net;

}
/**
* Train the vanilla network on a larger dataset.
*/
BOOST_AUTO_TEST_CASE(CopyVanillaNetworkTest)
{
  arma::mat X;
  X.load("mnist_first250_training_4s_and_9s.arm");

  // Normalize each point since these are images.
  arma::uword nPoints = X.n_cols;
  for (arma::uword i = 0; i < nPoints; i++)
  {
    X.col(i) /= norm(X.col(i), 2);
  }

  // Build the target matrix.
  arma::mat Y = arma::zeros<arma::mat>(10, nPoints);
  for (size_t i = 0; i < nPoints; i++)
  {
    if (i < nPoints / 2)
    {
      Y.col(i)(5) = 1;
    }
    else
    {
      Y.col(i)(8) = 1;
    }
  }

  arma::cube input = arma::cube(28, 28, nPoints);
  for (size_t i = 0; i < nPoints; i++)
    input.slice(i) = arma::mat(X.colptr(i), 28, 28);
  auto net = CopyVanillaNetwork();
  RMSprop<decltype(net)> opt(net, 0.01, 0.88, 1e-8, 10 * input.n_slices, 0);

  net.Train(input, Y, opt);

  arma::mat prediction;
  net.Predict(input, prediction);

  size_t error = 0;
  for (size_t i = 0; i < nPoints; i++)
  {
    if (arma::sum(arma::sum(
      arma::abs(prediction.col(i) - Y.col(i)))) == 0)
    {
      error++;
    }
  }

  double classificationError = 1 - double(error) / nPoints;
  BOOST_REQUIRE_LE(classificationError, 0.6);
}

/**
* Train the vanilla network on a larger dataset.
*/
BOOST_AUTO_TEST_CASE(ConvVanillaNetworkTest)
{
  BuildVanillaNetwork();
}


BOOST_AUTO_TEST_SUITE_END();
