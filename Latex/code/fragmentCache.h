class FragmentCache {
 public:
  FragmentCache() {}
  ~FragmentCache() {}

  Fragment * createFragment(const voffset_t field, const uint32_t start_byte, const uint32_t end_byte) {
    ESP_LOGI(TAG, "Creating Fragment: %u, %u, %p at: %u", start_byte, end_byte, nullptr, field);
    _globalCache()->operator[](_getIndex(field)) 
      = std::make_shared<Fragment>(start_byte, end_byte);
    return _globalCache()->operator[](_getIndex(field)).get();
  }
  Fragment * createVector(const voffset_t field, const uint32_t start_byte, const uint32_t end_byte);
  base_t* getPointer(const voffset_t field);
  bool validFragment(const voffset_t field);

 private:
  static uint32_t* _globalIndex() {
    static uint32_t _globalIndex = 0;
    return &_globalIndex;
  }
  static std::map<const uint32_t, std::shared_ptr<Fragment>>* _globalCache() {
    static std::map<const uint32_t, std::shared_ptr<Fragment>> _globalCache = {};
    return &_globalCache;
  }
  std::map<const uint8_t, uint32_t> _indexCache;

  static void _freeMemory(const uint32_t size) {
    while (size > esp_get_free_heap_size()) {
      Fragment nullFragment(0, 0);
      Fragment * fragment = &nullFragment;
      uint32_t index;
      for (index = 0; index < *_globalIndex(); index++) {
        Fragment * tmp_fragment = _globalCache()->operator[](index).get();
        if (tmp_fragment->_pointer && 
            tmp_fragment->_end_byte - tmp_fragment->_start_byte > fragment->_end_byte - fragment->_start_byte) {
          fragment = tmp_fragment;
        }
      }
      fragment->free();
    }
  }
  uint32_t _getIndex(const voffset_t field) {
    if (_indexCache.find(field) == _indexCache.end()) {
      _indexCache[field] = *_globalIndex();
      *_globalIndex() += 1;
    }
    return _indexCache[field];
  }
};