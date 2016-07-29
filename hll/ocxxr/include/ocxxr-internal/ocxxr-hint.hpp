#ifndef OCXXR_HINT_HPP_
#define OCXXR_HINT_HPP_

namespace ocxxr {

class Hint {
 public:
    Hint(ocrHintType_t hint_type) {
        internal::OK(ocrHintInit(&hint_, hint_type));
    }

    Hint &Set(ocrHintProp_t prop, u64 value) {
        internal::OK(ocrSetHintValue(&hint_, prop, value));
        return *this;
    }

    Hint &Unset(ocrHintProp_t prop) {
        internal::OK(ocrUnsetHintValue(&hint_, prop));
        return *this;
    }

    u64 Get(ocrHintProp_t prop) {
        u64 value;
        internal::OK(ocrGetHintValue(&hint_, prop, &value));
        return value;
    }

    Hint &AttachTo(ObjectHandle object) {
        internal::OK(ocrSetHint(object.guid(), &hint_));
        return *this;
    }

    Hint &ReadFrom(ObjectHandle object) {
        internal::OK(ocrGetHint(object.guid(), &hint_));
        return *this;
    }

    const ocrHint_t *internal() const { return &hint_; }

    ocrHint_t *internal() { return &hint_; }

 private:
    ocrHint_t hint_;
};

class TaskHint : public Hint {
 public:
    TaskHint() : Hint(OCR_HINT_EDT_T) {}
};

class DatablockHint : public Hint {
 public:
    DatablockHint() : Hint(OCR_HINT_DB_T) {}
};

}  // namespace ocxxr

#endif  // OCXXR_HINT_HPP_
