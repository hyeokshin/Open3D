// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "open3d/core/TensorKey.h"

#include <memory>
#include <sstream>

#include "open3d/core/Tensor.h"
#include "open3d/utility/Console.h"

namespace open3d {
namespace core {

class TensorKey::Impl {
public:
    virtual ~Impl() {}
};

class TensorKey::IndexImpl : public TensorKey::Impl {
public:
    IndexImpl(int64_t index) : index_(index) {}
    int64_t GetIndex() const { return index_; }

private:
    int64_t index_;
};

class TensorKey::SliceImpl : public TensorKey::Impl {
public:
    SliceImpl(utility::optional<int64_t> start,
              utility::optional<int64_t> stop,
              utility::optional<int64_t> step)
        : start_(start), stop_(stop), step_(step) {}
    std::shared_ptr<SliceImpl> UpdateWithDimSize(int64_t dim_size) const {
        return std::make_shared<SliceImpl>(
                start_.has_value() ? start_.value() : 0,
                stop_.has_value() ? stop_.value() : dim_size,
                step_.has_value() ? step_.value() : 1);
    }
    int64_t GetStart() const {
        if (start_.has_value()) {
            return start_.value();
        } else {
            utility::LogError("TensorKeyMode::Slice: start is None.");
        }
    }
    int64_t GetStop() const {
        if (stop_.has_value()) {
            return stop_.value();
        } else {
            utility::LogError("TensorKeyMode::Slice: stop is None.");
        }
    }
    int64_t GetStep() const {
        if (step_.has_value()) {
            return step_.value();
        } else {
            utility::LogError("TensorKeyMode::Slice: step is None.");
        }
    }

private:
    utility::optional<int64_t> start_ = utility::nullopt;
    utility::optional<int64_t> stop_ = utility::nullopt;
    utility::optional<int64_t> step_ = utility::nullopt;
};

class TensorKey::IndexTensorImpl : public TensorKey::Impl {
public:
    IndexTensorImpl(const Tensor& index_tensor)
        : index_tensor_(std::make_shared<Tensor>(index_tensor)) {}
    std::shared_ptr<Tensor> GetIndexTensor() const { return index_tensor_; }

private:
    std::shared_ptr<Tensor> index_tensor_;
};

TensorKey::TensorKey(const std::shared_ptr<Impl>& impl) : impl_(impl) {}

TensorKey::TensorKeyMode TensorKey::GetMode() const {
    if (std::dynamic_pointer_cast<IndexImpl>(impl_)) {
        return TensorKeyMode::Index;
    } else if (std::dynamic_pointer_cast<SliceImpl>(impl_)) {
        return TensorKeyMode::Slice;
    } else if (std::dynamic_pointer_cast<IndexTensorImpl>(impl_)) {
        return TensorKeyMode::IndexTensor;
    } else {
        utility::LogError("GetMode() failed: unsupported mode.");
    }
}

std::string TensorKey::ToString() const { return "TODO"; }

TensorKey TensorKey::Index(int64_t index) {
    return TensorKey(std::make_shared<IndexImpl>(index));
}

TensorKey TensorKey::Slice(utility::optional<int64_t> start,
                           utility::optional<int64_t> stop,
                           utility::optional<int64_t> step) {
    return TensorKey(std::make_shared<SliceImpl>(start, stop, step));
}

TensorKey TensorKey::IndexTensor(const Tensor& index_tensor) {
    return TensorKey(std::make_shared<IndexTensorImpl>(index_tensor));
}

/// For TensorKeyMode::Index.
int64_t TensorKey::GetIndex() const {
    if (auto index_impl = std::dynamic_pointer_cast<IndexImpl>(impl_)) {
        return index_impl->GetIndex();
    } else {
        utility::LogError("GetIndex() failed: the impl is not IndexImpl.");
    }
}

/// For TensorKeyMode::Slice.
int64_t TensorKey::GetStart() const {
    if (auto slice_impl = std::dynamic_pointer_cast<SliceImpl>(impl_)) {
        return slice_impl->GetStart();
    } else {
        utility::LogError("GetStart() failed: the impl is not SliceImpl.");
    }
}
int64_t TensorKey::GetStop() const {
    if (auto slice_impl = std::dynamic_pointer_cast<SliceImpl>(impl_)) {
        return slice_impl->GetStop();
    } else {
        utility::LogError("GetStop() failed: the impl is not SliceImpl.");
    }
}
int64_t TensorKey::GetStep() const {
    if (auto slice_impl = std::dynamic_pointer_cast<SliceImpl>(impl_)) {
        return slice_impl->GetStep();
    } else {
        utility::LogError("GetStep() failed: the impl is not SliceImpl.");
    }
}
TensorKey TensorKey::UpdateWithDimSize(int64_t dim_size) const {
    if (auto slice_impl = std::dynamic_pointer_cast<SliceImpl>(impl_)) {
        return TensorKey(slice_impl->UpdateWithDimSize(dim_size));
    } else {
        utility::LogError(
                "UpdateWithDimSize() failed: the impl is not SliceImpl.");
    }
}

/// For TensorKeyMode::IndexTensor.
std::shared_ptr<Tensor> TensorKey::GetIndexTensor() const {
    if (auto index_tensor_impl =
                std::dynamic_pointer_cast<IndexTensorImpl>(impl_)) {
        return index_tensor_impl->GetIndexTensor();
    } else {
        utility::LogError(
                "GetIndexTensor() failed: the impl is not IndexTensorImpl.");
    }
}

// std::string TensorKey::ToString() const { return "bad"; }

// TensorKey TensorKey::Index(int64_t index) { return TensorKeyIndex(index); }

// TensorKey TensorKey::Slice(utility::optional<int64_t> start,
//                            utility::optional<int64_t> stop,
//                            utility::optional<int64_t> step) {
//     return TensorKeySlice(start, stop, step);
// }

// TensorKey TensorKey::IndexTensor(const Tensor& index_tensor) {
//     return TensorKeyIndexTensor(index_tensor);
// }

// TensorKey::TensorKeyMode TensorKey::GetMode() const { return mode_; }

// int64_t TensorKey::GetIndex() const {
//     if (const TensorKeyIndex* tk = dynamic_cast<const TensorKeyIndex*>(this))
//     {
//         return tk->GetIndex();
//     } else {
//         utility::LogError(
//                 "TensorKey::GetIndex() is invalid since the TensorKey is not
//                 a " "TensorKeyIndex instance.");
//     }
// }

// int64_t TensorKey::GetStart() const {
//     if (const TensorKeySlice* tk = dynamic_cast<const TensorKeySlice*>(this))
//     {
//         return tk->GetStart();
//     } else {
//         utility::LogError(
//                 "TensorKey::GetStart() is invalid since the TensorKey is not
//                 a " "TensorKeySlice instance.");
//     }
// }

// int64_t TensorKey::GetStop() const {
//     if (const TensorKeySlice* tk = dynamic_cast<const TensorKeySlice*>(this))
//     {
//         return tk->GetStop();
//     } else {
//         utility::LogError(
//                 "TensorKey::GetStop() is invalid since the TensorKey is not a
//                 " "TensorKeySlice instance.");
//     }
// }

// int64_t TensorKey::GetStep() const {
//     if (const TensorKeySlice* tk = dynamic_cast<const TensorKeySlice*>(this))
//     {
//         return tk->GetStep();
//     } else {
//         utility::LogError(
//                 "TensorKey::GetStep() is invalid since the TensorKey is not a
//                 " "TensorKeySlice instance.");
//     }
// }

// TensorKey TensorKey::UpdateWithDimSize(int64_t dim_size) const {
//     if (const TensorKeySlice* tk = dynamic_cast<const TensorKeySlice*>(this))
//     {
//         return tk->UpdateWithDimSize(dim_size);
//     } else {
//         utility::LogError(
//                 "TensorKey::UpdateWithDimSize() is invalid since the
//                 TensorKey " "is not a TensorKeySlice instance.");
//     }
// }

// std::shared_ptr<Tensor> TensorKey::GetIndexTensor() const {
//     if (const TensorKeyIndexTensor* tk =
//                 dynamic_cast<const TensorKeyIndexTensor*>(this)) {
//         return tk->GetIndexTensor();
//     } else {
//         utility::LogError(
//                 "TensorKey::GetIndexTensor() is invalid since the TensorKey
//                 is " "not a TensorKeyIndexTensor instance.");
//     }
// }

// int64_t TensorKeyIndex::GetIndex() const { return index_; }

// std::string TensorKeyIndex::ToString() const {
//     std::stringstream ss;
//     ss << "TensorKey::Index(" << index_ << ")";
//     return ss.str();
// }

// int64_t TensorKeySlice::GetStart() const {
//     if (start_.has_value()) {
//         return start_.value();
//     } else {
//         utility::LogError("TensorKeyMode::Slice: start is None.");
//     }
// }

// int64_t TensorKeySlice::GetStop() const {
//     if (stop_.has_value()) {
//         return stop_.value();
//     } else {
//         utility::LogError("TensorKeyMode::Slice: stop is None.");
//     }
// }

// int64_t TensorKeySlice::GetStep() const {
//     if (step_.has_value()) {
//         return step_.value();
//     } else {
//         utility::LogError("TensorKeyMode::Slice: step is None.");
//     }
// }

// TensorKey TensorKeySlice::UpdateWithDimSize(int64_t dim_size) const {
//     return TensorKeySlice(start_.has_value() ? start_.value() : 0,
//                           stop_.has_value() ? stop_.value() : dim_size,
//                           step_.has_value() ? step_.value() : 1);
// }

// std::string TensorKeySlice::ToString() const {
//     std::stringstream ss;
//     ss << "TensorKey::Slice(";
//     if (start_.has_value()) {
//         ss << start_.value();
//     } else {
//         ss << "None";
//     }
//     ss << ", ";
//     if (stop_.has_value()) {
//         ss << stop_.value();
//     } else {
//         ss << "None";
//     }
//     ss << ", ";
//     if (step_.has_value()) {
//         ss << step_.value();
//     } else {
//         ss << "None";
//     }
//     ss << ")";
//     return ss.str();
// }

// std::shared_ptr<Tensor> TensorKeyIndexTensor::GetIndexTensor() const {
//     return index_tensor_;
// }

// std::string TensorKeyIndexTensor::ToString() const {
//     std::stringstream ss;
//     ss << "TensorKey::IndexTensor(" << index_tensor_->ToString() << ")";
//     return ss.str();
// }

}  // namespace core
}  // namespace open3d
