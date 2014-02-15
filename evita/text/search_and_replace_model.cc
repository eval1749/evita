// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/text/search_and_replace_model.h"

SearchParameters::SearchParameters() : m_rgf(0) {
}

SearchParameters::~SearchParameters() {
}

namespace text {

SearchAndReplaceModel::SearchAndReplaceModel()
    : direction_(kDirectionDown), flags_(0), replace_in_(kReplaceInWhole) {
}

SearchAndReplaceModel::~SearchAndReplaceModel() {
}

}  // namespace text
