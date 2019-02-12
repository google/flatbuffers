// custom type for native api including converters

#include <complex>
#include "native_test_generated.h"

class MyMat
{
public:
  MyMat() = default;
  MyMat(unsigned rows, std::vector<std::complex<double>> dat)
    :m_rows(rows), m_dat(dat){}

  unsigned rows() const {return m_rows;}
  const std::vector<std::complex<double>> &dat() const {return m_dat;}

public:
  unsigned m_rows = 0;
  std::vector<std::complex<double>> m_dat;
};

inline bool operator==(const MyMat &lhs, const MyMat &rhs) {
  return (lhs.rows() == rhs.rows()) && (lhs.dat() == rhs.dat());
}

namespace Native
{
  inline const std::complex<double> UnPack(const ::Complex &v) {
    return std::complex<double>(v.i(), v.q());
  }

  inline const ::Complex Pack(const std::complex<double> &v) {
    return ::Complex(v.real(), v.imag());
  }

  inline MyMat UnPack(const ::Mat &_f, const flatbuffers::resolver_function_t *) {
    std::vector<std::complex<double>> dat;
    auto _e = _f.data();
    dat.resize(_e->size());
    for (flatbuffers::uoffset_t _i = 0; _i < _e->size(); _i++) {
      dat[_i] = Native::UnPack(*_e->Get(_i));
    }
    return MyMat(_f.rows(), dat);
  }

  inline flatbuffers::Offset<::Mat> Pack( flatbuffers::FlatBufferBuilder &_fbb, const MyMat &_o,
      const flatbuffers::rehasher_function_t *) {

    auto data__ = _o.dat().size() ? _fbb.CreateVectorOfStructs<::Complex>(
                                       _o.dat().size(),
                                       [&](size_t i, ::Complex *r) {
                                         *r = Native::Pack(_o.dat()[i]);
                                       })
                                 : 0;

    MatBuilder builder_(_fbb);
    builder_.add_data(data__);
    builder_.add_rows(_o.rows());
    return builder_.Finish();
  }


}
