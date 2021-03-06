#ifndef dplyr_Result_Sum_H
#define dplyr_Result_Sum_H

#include <dplyr/Result/Processor.h>

namespace dplyr {

namespace internal {

// General case (for INTSXP and LGLSXP)
template <int RTYPE, bool NA_RM, typename Index>
struct Sum {
  typedef typename Rcpp::traits::storage_type<RTYPE>::type STORAGE;
  static STORAGE process(typename Rcpp::traits::storage_type<RTYPE>::type* ptr,  const Index& indices) {
    long double res = 0;
    int n = indices.size();
    for (int i = 0; i < n; i++) {
      STORAGE value = ptr[indices[i]];

      if (Rcpp::traits::is_na<RTYPE>(value)) {
        if (NA_RM) {
          continue;
        }

        return Rcpp::traits::get_na<RTYPE>();
      }

      res += value;
    }

    if (res > INT_MAX || res <= INT_MIN) {
      warning("integer overflow - use sum(as.numeric(.))");
      return Rcpp::traits::get_na<RTYPE>();
    }

    return (STORAGE)res;
  }
};

// special case for REALSXP because it treats NA differently
template <bool NA_RM, typename Index>
struct Sum<REALSXP, NA_RM, Index> {
  static double process(double* ptr,  const Index& indices) {
    long double res = 0;
    int n = indices.size();
    for (int i = 0; i < n; i++) {
      double value = ptr[indices[i]];

      if (NA_RM && Rcpp::traits::is_na<REALSXP>(value)) {
        continue;
      }

      if (!NA_RM && Rcpp::traits::is_na<REALSXP>(value)) {
        return NA_REAL;
      }

      res += value;
    }

    return (double)res;
  }
};

} // namespace internal

template <int RTYPE, bool NA_RM>
class Sum : public Processor < RTYPE == LGLSXP ? INTSXP : RTYPE, Sum<RTYPE, NA_RM> > {
public :
  typedef Processor < RTYPE == LGLSXP ? INTSXP : RTYPE, Sum<RTYPE, NA_RM> > Base;
  typedef typename Rcpp::traits::storage_type<RTYPE>::type STORAGE;

  Sum(SEXP x) :
    Base(x),
    data_ptr(Rcpp::internal::r_vector_start<RTYPE>(x))
  {}
  ~Sum() {}

  inline STORAGE process_chunk(const SlicingIndex& indices) {
    return internal::Sum<RTYPE, NA_RM, SlicingIndex>::process(data_ptr, indices);
  }

  STORAGE* data_ptr;
};

}

#endif
