/* Copyright (c) 2016 PaddlePaddle Authors. All Rights Reserve.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include "paddle/operators/pad_op.h"

namespace paddle {
namespace operators {

using framework::Tensor;

class PadOp : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

 protected:
  void InferShape(const framework::InferShapeContext &ctx) const override {
    PADDLE_ENFORCE_NOT_NULL(ctx.InputVar("X"),
                            "Input(X) of PadOp should not be null.");
    PADDLE_ENFORCE_NOT_NULL(ctx.OutputVar("Out"),
                            "Output(Out) of PadOp should not be null.");

    auto x_dim = ctx.Input<Tensor>("X")->dims();
    auto paddings = Attr<std::vector<int>>("paddings");
    PADDLE_ENFORCE_EQ(x_dim.size() * 2, int64_t(paddings.size()),
                      "Size of paddings should be equal to 2 * dimension size "
                      "of input tensor.");
    std::vector<int64_t> out_dims(x_dim.size());
    for (int i = 0; i < x_dim.size(); ++i) {
      out_dims[i] = x_dim[i] + paddings[i * 2] + paddings[i * 2 + 1];
    }
    ctx.Output<framework::LoDTensor>("Out")->Resize(
        framework::make_ddim(out_dims));
  }
};

class PadOpMaker : public framework::OpProtoAndCheckerMaker {
 public:
  PadOpMaker(framework::OpProto *proto, framework::OpAttrChecker *op_checker)
      : OpProtoAndCheckerMaker(proto, op_checker) {
    AddInput("X",
             "The input of pad op. "
             "The input should be a k-D tensor(k > 0 and k < 7)");
    AddOutput("Out",
              "The output of pad op."
              "A tensor with the same shape as X.")
        .NotInGradient();
    AddComment(R"DOC(
Pad input into output, as specified by paddings and pad_value. The input should be a k-D tensor(k > 0 and k < 7). As an example:

Given:

X = [[1, 2],
   [3, 4]]

and 

paddings = [0, 1, 1, 2]

and
 
pad_value = 0 

then we get 

Out = [[0, 1, 2, 0, 0]
       [0, 3, 4, 0, 0]
       [0, 0, 0, 0, 0]]
)DOC");
    AddAttr<std::vector<int>>(
        "paddings",
        "A list<int> to describes padding rules for each dimension."
        " For 2-D image tensor, paddings=[0, 1, 2, 3] means"
        " padding 0 row to top, 1 row to bottom, 2 columns to left"
        " and 3 columns to right.Size of paddings should be equal to"
        " 2 * dimension size of input tensor.");
    AddAttr<float>("pad_value",
                   "(float) default to 0; "
                   "The value to fill padded areas.")
        .SetDefault(0.0f);
  }
};

class PadOpGrad : public framework::OperatorWithKernel {
 public:
  using framework::OperatorWithKernel::OperatorWithKernel;

 protected:
  void InferShape(const framework::InferShapeContext &ctx) const override {
    PADDLE_ENFORCE_NOT_NULL(ctx.InputVar("X"), "Input(X) should not be null");
    PADDLE_ENFORCE_NOT_NULL(ctx.InputVar(framework::GradVarName("Out")),
                            "Input(Out@GRAD) should not be null");
    auto x_dims = ctx.Input<Tensor>("X")->dims();
    auto *x_g = ctx.Output<framework::LoDTensor>(framework::GradVarName("X"));
    if (x_g != nullptr) {
      x_g->Resize(x_dims);
    }
  }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OP(pad, ops::PadOp, ops::PadOpMaker, pad_grad, ops::PadOpGrad);
REGISTER_OP_CPU_KERNEL(pad, ops::PadKernel<paddle::platform::CPUPlace, float>);
REGISTER_OP_CPU_KERNEL(pad_grad,
                       ops::PadGradKernel<paddle::platform::CPUPlace, float>);
