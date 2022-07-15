class Fragment {
  friend class FragmentCache;
  friend class AddonClass;

 public:
  Fragment(const uint32_t start_byte, const uint32_t end_byte)
      : _start_byte(start_byte),
        _end_byte(end_byte),
        _pointer(nullptr) {}

  virtual base_t * getPointer() {
    if (_pointer == nullptr) {
      ESP_LOGI(TAG, "Fragment not read");
      return nullptr;
    }
    return _pointer.get();
  }
  virtual void read() {
    if (_pointer == nullptr) {
      _pointer = GetData(_start_byte, _end_byte - _start_byte);
      AddonClass::Get(_pointer.get())->SetFragment(this);
    }
  }
  virtual void free() { _pointer = nullptr; }
  bool CheckByte(const uint32_t byte) {
    return byte >= _start_byte && _end_byte >= byte;
  }

 protected:
  const uint32_t _start_byte;
  const uint32_t _end_byte;
  std::shared_ptr<base_t[]> _pointer;
};