// Copyright (C) 2014 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)

#include "evita/text/search_and_replace_model.h"

namespace text {

SearchParameters::SearchParameters(const base::string16& search_text)
    : flags(0), search_text(search_text) {
}

SearchParameters::SearchParameters() : SearchParameters(base::string16()) {
}

SearchParameters::~SearchParameters() {
}

SearchAndReplaceModel::SearchAndReplaceModel()
    : direction_(kDirectionDown), flags_(0), replace_in_(kReplaceInWhole) {
}

SearchAndReplaceModel::~SearchAndReplaceModel() {
}

}  // namespace text
