/**
 * @file softmax_layer.hpp
 * @author Marcus Edel
 *
 * Definition of the SoftmaxLayer class.
 */
#ifndef __MLPACK_METHODS_ANN_LAYER_SOFTMAX_LAYER_HPP
#define __MLPACK_METHODS_ANN_LAYER_SOFTMAX_LAYER_HPP

#include <mlpack/core.hpp>

namespace mlpack {
namespace ann /** Artificial Neural Network. */ {

/**
 * Implementation of the softmax layer. The softmax loss layer computes the
 * multinomial logistic loss of the softmax of its inputs.
 *
 * @tparam InputDataType Type of the input data (arma::colvec, arma::mat,
 *         arma::sp_mat or arma::cube).
 * @tparam OutputDataType Type of the output data (arma::colvec, arma::mat,
 *         arma::sp_mat or arma::cube).
 */
template <
    typename InputDataType = arma::mat,
    typename OutputDataType = arma::mat
>
class SoftmaxLayer
{
 public:
  /**
   * Create the SoftmaxLayer object.
   */
  SoftmaxLayer()
  {
    // Nothing to do here.
  }  

  /**
   * Ordinary feed forward pass of a neural network, evaluating the function
   * f(x) by propagating the activity forward through f.
   *
   * @param input Input data used for evaluating the specified function.
   * @param output Resulting output activation.
   */
  template<typename eT>
  void Forward(const arma::Mat<eT>& input, arma::Mat<eT>& output)
  {
    output = arma::trunc_exp(input -
        arma::repmat(arma::max(input), input.n_rows, 1));
    output /= arma::accu(output);
  }

  /**
   * Ordinary feed backward pass of a neural network, calculating the function
   * f(x) by propagating x backwards trough f. Using the results from the feed
   * forward pass.
   *
   * @param input The propagated input activation.
   * @param gy The backpropagated error.
   * @param g The calculated gradient.
   */
  template<typename eT>
  void Backward(const arma::Mat<eT>& /* unused */,
                const arma::Mat<eT>& gy,
                arma::Mat<eT>& g)
  {
    g = gy;
  }

  //! Get the input parameter.
  const InputDataType& InputParameter() const { return inputParameter; }
  //! Modify the input parameter.
  InputDataType& InputParameter() { return inputParameter; }

  //! Get the output parameter.
  const OutputDataType& OutputParameter() const { return outputParameter; }
  //! Modify the output parameter.
  OutputDataType& OutputParameter() { return outputParameter; }

  //! Get the delta.
  const InputDataType& Delta() const { return delta; }
  //! Modify the delta.
  InputDataType& Delta() { return delta; }

 private:
  //! Locally-stored delta object.
  OutputDataType delta;

  //! Locally-stored input parameter object.
  InputDataType inputParameter;

  //! Locally-stored output parameter object.
  OutputDataType outputParameter;
}; // class SoftmaxLayer

} // namespace ann
} // namespace mlpack

#endif
