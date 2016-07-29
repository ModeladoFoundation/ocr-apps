#ifndef OCXXR_HANDLE_HPP_
#define OCXXR_HANDLE_HPP_

namespace ocxxr {

/// Abstract base class for all OCR objects with GUIDs.
class ObjectHandle {
 public:
    /// Access the GUID for this object (for direct use with OCR API).
    ocrGuid_t guid() const { return guid_; }

    /// @brief Null handle predicate.
    ///
    /// A null handle indicates the absence of an object.
    bool is_null() const { return ocrGuidIsNull(guid_); }

    /// @brief Uninitialized handle predicate.
    ///
    /// Uninitialized handles correspond to holes in a Task's dependence list.
    /// @see UnknownDependence
    bool is_uninitialized() const { return ocrGuidIsUninitialized(guid_); }

    /// @brief Error handle predicate.
    ///
    /// An error handle may be returned in place of a valid handle
    /// to indicate that an error occurred.
    bool is_error() const { return ocrGuidIsError(guid_); }

    /// Compare two OCR objects for identity-based equality.
    bool operator==(const ObjectHandle &rhs) const {
        return ocrGuidIsEq(guid_, rhs.guid_);
    }

    /// Compare two OCR objects for identity-based inequality.
    bool operator!=(const ObjectHandle &rhs) const { return !(*this == rhs); }

    /// Order two OCR objects based on the internal GUID representation.
    bool operator<(const ObjectHandle &rhs) const {
        return ocrGuidIsLt(guid_, rhs.guid_);
    }

    /// Order two OCR objects based on the internal GUID representation.
    bool operator>(const ObjectHandle &rhs) const { return rhs < *this; }

    /// Order two OCR objects based on the internal GUID representation.
    bool operator<=(const ObjectHandle &rhs) const { return !(*this > rhs); }

    /// Order two OCR objects based on the internal GUID representation.
    bool operator>=(const ObjectHandle &rhs) const { return !(*this < rhs); }

 protected:
    explicit ObjectHandle(ocrGuid_t guid) : guid_(guid) {}

 private:
    ocrGuid_t guid_;
};

static_assert(internal::IsLegalHandle<ObjectHandle>::value,
              "ObjectHandle must be castable to/from ocrGuid_t.");

}  // namespace ocxxr

#endif  // OCXXR_HANDLE_HPP_
