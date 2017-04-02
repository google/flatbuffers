// automatically generated by the FlatBuffers compiler, do not modify


#ifndef FLATBUFFERS_GENERATED_NONROOT1_WARCHANT_H_
#define FLATBUFFERS_GENERATED_NONROOT1_WARCHANT_H_

#include "flatbuffers/flatbuffers.h"

#include "root1_generated.h"
#include "root2_generated.h"

namespace warchant {

struct Transaction;

struct Transaction FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_ACCOUNT = 4,
    VT_ASSET = 6
  };
  const warchant::Account *account() const {
    return GetPointer<const warchant::Account *>(VT_ACCOUNT);
  }
  const warchant::Asset *asset() const {
    return GetPointer<const warchant::Asset *>(VT_ASSET);
  }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_ACCOUNT) &&
           verifier.VerifyTable(account()) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_ASSET) &&
           verifier.VerifyTable(asset()) &&
           verifier.EndTable();
  }
};

struct TransactionBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_account(flatbuffers::Offset<warchant::Account> account) {
    fbb_.AddOffset(Transaction::VT_ACCOUNT, account);
  }
  void add_asset(flatbuffers::Offset<warchant::Asset> asset) {
    fbb_.AddOffset(Transaction::VT_ASSET, asset);
  }
  TransactionBuilder(flatbuffers::FlatBufferBuilder &_fbb)
        : fbb_(_fbb) {
    start_ = fbb_.StartTable();
  }
  TransactionBuilder &operator=(const TransactionBuilder &);
  flatbuffers::Offset<Transaction> Finish() {
    const auto end = fbb_.EndTable(start_, 2);
    auto o = flatbuffers::Offset<Transaction>(end);
    return o;
  }
};

inline flatbuffers::Offset<Transaction> CreateTransaction(
    flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<warchant::Account> account = 0,
    flatbuffers::Offset<warchant::Asset> asset = 0) {
  TransactionBuilder builder_(_fbb);
  builder_.add_asset(asset);
  builder_.add_account(account);
  return builder_.Finish();
}

}  // namespace warchant

#endif  // FLATBUFFERS_GENERATED_NONROOT1_WARCHANT_H_
